#include <sentrium/core.hpp>
#include <sentrium/device.hpp>
#include <sentrium/intelbras.hpp>
#include <sentrium/media.hpp>

#include <Windows.h>
#include <dhnetsdk.h>
#include <dhplay.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>

namespace {

using sentrium::media::Packet;
using sentrium::media::PacketQueue;

struct EnvironmentDeleter {
    void operator()(char* value) const noexcept {
        std::free(value);
    }
};

std::string environment(const char* name, bool required = true) {
    char* raw = nullptr;
    std::size_t length = 0;
    const errno_t status = _dupenv_s(&raw, &length, name);
    std::unique_ptr<char, EnvironmentDeleter> value(raw);

    if (status != 0) {
        throw std::runtime_error(
            std::string("could not read environment variable: ") + name);
    }

    if (!value || length <= 1 || value.get()[0] == '\0') {
        if (required) {
            throw std::runtime_error(
                std::string("missing environment variable: ") + name);
        }
        return {};
    }

    return std::string(value.get());
}

int integer_environment(const char* name, int default_value, int minimum, int maximum) {
    const auto text = environment(name, false);
    if (text.empty()) {
        return default_value;
    }
    std::size_t consumed = 0;
    const int value = std::stoi(text, &consumed);
    if (consumed != text.size() || value < minimum || value > maximum) {
        throw std::runtime_error(std::string("invalid integer environment variable: ") + name);
    }
    return value;
}

std::vector<int> media_channels() {
    const auto list = environment("SENTRIUM_MEDIA_CHANNELS", false);
    if (list.empty()) {
        return {integer_environment("SENTRIUM_MEDIA_CHANNEL", 0, 0, 4095)};
    }

    std::vector<int> channels;
    std::set<int> unique;
    std::stringstream stream(list);
    std::string token;
    while (std::getline(stream, token, ',')) {
        if (token.empty()) {
            throw std::runtime_error("SENTRIUM_MEDIA_CHANNELS contains an empty entry");
        }

        std::size_t consumed = 0;
        const int channel = std::stoi(token, &consumed);
        if (consumed != token.size() || channel < 0 || channel > 4095) {
            throw std::runtime_error("SENTRIUM_MEDIA_CHANNELS contains an invalid channel");
        }
        if (!unique.insert(channel).second) {
            throw std::runtime_error("SENTRIUM_MEDIA_CHANNELS contains a duplicate channel");
        }
        channels.push_back(channel);
    }

    if (channels.empty() || channels.size() > 16) {
        throw std::runtime_error("SENTRIUM_MEDIA_CHANNELS must contain between 1 and 16 channels");
    }
    return channels;
}

std::filesystem::path environment_path(
    const wchar_t* name,
    const char* missing_message) {
    const DWORD required = GetEnvironmentVariableW(name, nullptr, 0);
    if (required == 0) {
        throw std::runtime_error(missing_message);
    }

    std::wstring buffer(static_cast<std::size_t>(required), L'\0');
    const DWORD written =
        GetEnvironmentVariableW(name, buffer.data(), required);
    if (written == 0 || written >= required) {
        throw std::runtime_error("SDK root environment variable could not be read");
    }
    buffer.resize(written);
    return std::filesystem::path(buffer);
}

std::filesystem::path netsdk_root() {
    return environment_path(
        L"SENTRIUM_INTELBRAS_SDK_ROOT",
        "SENTRIUM_INTELBRAS_SDK_ROOT is not available");
}

std::filesystem::path playsdk_root() {
    return environment_path(
        L"SENTRIUM_PLAYSDK_ROOT",
        "SENTRIUM_PLAYSDK_ROOT is not available");
}

template <class T>
T load_export(HMODULE module, const char* name) {
    const auto address = GetProcAddress(module, name);
    if (!address) {
        throw std::runtime_error(std::string("missing SDK export: ") + name);
    }
    return reinterpret_cast<T>(address);
}

class Module final {
public:
    explicit Module(const std::filesystem::path& path) {
        handle_ = LoadLibraryExW(
            path.c_str(), nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!handle_) {
            throw std::runtime_error(
                "LoadLibraryExW failed for SDK module (win32="
                + std::to_string(GetLastError()) + ")");
        }
    }

    ~Module() {
        if (handle_) {
            FreeLibrary(handle_);
        }
    }

    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;

    HMODULE get() const noexcept { return handle_; }

private:
    HMODULE handle_ = nullptr;
};

struct Metrics {
    std::atomic<std::uint64_t> callback_packets{0};
    std::atomic<std::uint64_t> callback_bytes{0};
    std::atomic<std::uint64_t> ignored_packets{0};
    std::atomic<std::uint64_t> play_input_failures{0};
    std::atomic<std::uint64_t> play_input_retries{0};
    std::atomic<std::uint64_t> decoded_callbacks{0};
    std::atomic<long> last_width{0};
    std::atomic<long> last_height{0};
    std::atomic<long> last_frame_rate{0};
    std::atomic<long> last_frame_type{0};
};

struct CallbackContext {
    PacketQueue* queue = nullptr;
    Metrics* metrics = nullptr;
    std::atomic<std::uint64_t> sequence{0};
};

template <class T>
void* callback_user_pointer(T value) noexcept {
    using Value = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_pointer_v<Value>) {
        return const_cast<void*>(reinterpret_cast<const void*>(value));
    } else {
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
    }
}

template <class Callback>
struct RealDataCallbackBridge;

template <class R, class... Args>
struct RealDataCallbackBridge<R (CALLBACK *)(Args...)> {
    static R CALLBACK invoke(Args... args) noexcept {
        static_assert(sizeof...(Args) == 6, "Unexpected NetSDK real-data callback ABI");
        auto values = std::forward_as_tuple(args...);

        auto* context = static_cast<CallbackContext*>(
            callback_user_pointer(std::get<5>(values)));
        auto* buffer = std::get<2>(values);
        const auto size = static_cast<std::size_t>(std::get<3>(values));
        const auto data_type = static_cast<std::uint32_t>(std::get<1>(values));

        if (!context || !context->queue || !context->metrics || !buffer || size == 0) {
            if constexpr (!std::is_void_v<R>) {
                return R{};
            } else {
                return;
            }
        }

        constexpr std::size_t max_packet_bytes = 16U * 1024U * 1024U;
        if (size > max_packet_bytes) {
            ++context->metrics->ignored_packets;
            if constexpr (!std::is_void_v<R>) {
                return R{};
            } else {
                return;
            }
        }

        try {
            Packet packet;
            packet.data_type = data_type;
            packet.sequence =
                context->sequence.fetch_add(1, std::memory_order_relaxed);
            packet.arrival = std::chrono::steady_clock::now();
            packet.bytes.resize(size);
            std::memcpy(packet.bytes.data(), buffer, size);

            ++context->metrics->callback_packets;
            context->metrics->callback_bytes.fetch_add(
                static_cast<std::uint64_t>(size), std::memory_order_relaxed);
            context->queue->push(std::move(packet));
        } catch (...) {
            ++context->metrics->ignored_packets;
        }

        if constexpr (!std::is_void_v<R>) {
            return R{};
        }
    }
};

template <class Callback>
struct DecodeCallbackBridge;

template <class R, class... Args>
struct DecodeCallbackBridge<R (CALLBACK *)(Args...)> {
    static R CALLBACK invoke(Args... args) noexcept {
        static_assert(sizeof...(Args) == 6, "Unexpected PlaySDK decode callback ABI");
        auto values = std::forward_as_tuple(args...);

        auto* frame = std::get<3>(values);
        auto* metrics = static_cast<Metrics*>(
            callback_user_pointer(std::get<4>(values)));

        if (metrics && frame) {
            ++metrics->decoded_callbacks;
            metrics->last_width.store(frame->nWidth, std::memory_order_relaxed);
            metrics->last_height.store(frame->nHeight, std::memory_order_relaxed);
            metrics->last_frame_rate.store(
                frame->nFrameRate, std::memory_order_relaxed);
            metrics->last_frame_type.store(
                frame->nType, std::memory_order_relaxed);
        }

        if constexpr (!std::is_void_v<R>) {
            return R{};
        }
    }
};

class PlaySdk final {
public:
    explicit PlaySdk(const std::filesystem::path& root)
        : module_(root / L"DLL" / L"dhplay.dll") {
        get_free_port_ = load_export<decltype(&PLAY_GetFreePort)>(
            module_.get(), "PLAY_GetFreePort");
        release_port_ = load_export<decltype(&PLAY_ReleasePort)>(
            module_.get(), "PLAY_ReleasePort");
        set_open_mode_ = load_export<decltype(&PLAY_SetStreamOpenMode)>(
            module_.get(), "PLAY_SetStreamOpenMode");
        open_stream_ = load_export<decltype(&PLAY_OpenStream)>(
            module_.get(), "PLAY_OpenStream");
        set_dec_stream_ = load_export<decltype(&PLAY_SetDecCBStream)>(
            module_.get(), "PLAY_SetDecCBStream");
        set_dec_callback_ = load_export<decltype(&PLAY_SetDecCallBackEx)>(
            module_.get(), "PLAY_SetDecCallBackEx");
        play_ = load_export<decltype(&PLAY_Play)>(
            module_.get(), "PLAY_Play");
        input_ = load_export<decltype(&PLAY_InputData)>(
            module_.get(), "PLAY_InputData");
        stop_ = load_export<decltype(&PLAY_Stop)>(
            module_.get(), "PLAY_Stop");
        close_ = load_export<decltype(&PLAY_CloseStream)>(
            module_.get(), "PLAY_CloseStream");
        last_error_ = load_export<decltype(&PLAY_GetLastError)>(
            module_.get(), "PLAY_GetLastError");
    }

    ~PlaySdk() {
        close_noexcept();
    }

    PlaySdk(const PlaySdk&) = delete;
    PlaySdk& operator=(const PlaySdk&) = delete;

    void open(Metrics& metrics) {
        if (!get_free_port_(&port_)) {
            throw std::runtime_error("PLAY_GetFreePort failed");
        }
        owns_port_ = true;

        if (!set_open_mode_(port_, STREAME_REALTIME)) {
            fail("PLAY_SetStreamOpenMode");
        }

        constexpr DWORD source_pool_bytes = 4U * 1024U * 1024U;
        if (!open_stream_(port_, nullptr, 0, source_pool_bytes)) {
            fail("PLAY_OpenStream");
        }
        stream_open_ = true;

        if (!set_dec_stream_(port_, 1)) {
            fail("PLAY_SetDecCBStream");
        }
        if (!set_dec_callback_(
                port_,
                &DecodeCallbackBridge<fDecCBFun>::invoke,
                &metrics)) {
            fail("PLAY_SetDecCallBackEx");
        }
        if (!play_(port_, nullptr)) {
            fail("PLAY_Play");
        }
        playing_ = true;
    }

    bool input(const std::byte* bytes, std::size_t size) {
        if (!playing_ || !bytes || size == 0 || size > static_cast<std::size_t>((std::numeric_limits<DWORD>::max)())) {
            return false;
        }
        return input_(
            port_,
            reinterpret_cast<PBYTE>(const_cast<std::byte*>(bytes)),
            static_cast<DWORD>(size)) != FALSE;
    }

    unsigned last_error() const noexcept {
        return owns_port_ ? last_error_(port_) : 0U;
    }

    void close() {
        if (playing_) {
            stop_(port_);
            playing_ = false;
        }
        if (stream_open_) {
            close_(port_);
            stream_open_ = false;
        }
        if (owns_port_) {
            release_port_(port_);
            owns_port_ = false;
            port_ = -1;
        }
    }

private:
    [[noreturn]] void fail(const char* action) {
        const auto error = last_error();
        close_noexcept();
        throw std::runtime_error(
            std::string(action) + " failed (playsdk_error=" + std::to_string(error) + ")");
    }

    void close_noexcept() noexcept {
        if (playing_) {
            stop_(port_);
            playing_ = false;
        }
        if (stream_open_) {
            close_(port_);
            stream_open_ = false;
        }
        if (owns_port_) {
            release_port_(port_);
            owns_port_ = false;
            port_ = -1;
        }
    }

    Module module_;
    decltype(&PLAY_GetFreePort) get_free_port_ = nullptr;
    decltype(&PLAY_ReleasePort) release_port_ = nullptr;
    decltype(&PLAY_SetStreamOpenMode) set_open_mode_ = nullptr;
    decltype(&PLAY_OpenStream) open_stream_ = nullptr;
    decltype(&PLAY_SetDecCBStream) set_dec_stream_ = nullptr;
    decltype(&PLAY_SetDecCallBackEx) set_dec_callback_ = nullptr;
    decltype(&PLAY_Play) play_ = nullptr;
    decltype(&PLAY_InputData) input_ = nullptr;
    decltype(&PLAY_Stop) stop_ = nullptr;
    decltype(&PLAY_CloseStream) close_ = nullptr;
    decltype(&PLAY_GetLastError) last_error_ = nullptr;

    LONG port_ = -1;
    bool owns_port_ = false;
    bool stream_open_ = false;
    bool playing_ = false;
};

class RealPlay final {
public:
    RealPlay(
        const std::filesystem::path& root,
        sentrium::device::Session session,
        int channel,
        DH_RealPlayType stream_type,
        CallbackContext& context)
        : module_(root / L"bin" / L"dhnetsdk.dll") {
        real_play_ = load_export<decltype(&CLIENT_RealPlayEx)>(
            module_.get(), "CLIENT_RealPlayEx");
        set_callback_ = load_export<decltype(&CLIENT_SetRealDataCallBackEx2)>(
            module_.get(), "CLIENT_SetRealDataCallBackEx2");
        stop_ = load_export<decltype(&CLIENT_StopRealPlayEx)>(
            module_.get(), "CLIENT_StopRealPlayEx");
        last_error_ = load_export<decltype(&CLIENT_GetLastError)>(
            module_.get(), "CLIENT_GetLastError");

        handle_ = real_play_(
            static_cast<LLONG>(session.value),
            channel,
            nullptr,
            stream_type);
        if (handle_ == 0) {
            throw std::runtime_error(
                "CLIENT_RealPlayEx failed (sdk_error="
                + std::to_string(last_error_()) + ")");
        }

        constexpr DWORD original_stream_only = 0x00000001U;
        const auto user = static_cast<LDWORD>(
            reinterpret_cast<std::uintptr_t>(&context));
        if (!set_callback_(
                handle_,
                &RealDataCallbackBridge<fRealDataCallBackEx2>::invoke,
                user,
                original_stream_only)) {
            const auto error = last_error_();
            stop_(handle_);
            handle_ = 0;
            throw std::runtime_error(
                "CLIENT_SetRealDataCallBackEx2 failed (sdk_error="
                + std::to_string(error) + ")");
        }
    }

    ~RealPlay() {
        stop();
    }

    RealPlay(const RealPlay&) = delete;
    RealPlay& operator=(const RealPlay&) = delete;

    void stop() noexcept {
        if (handle_ != 0) {
            stop_(handle_);
            handle_ = 0;
        }
    }

private:
    Module module_;
    decltype(&CLIENT_RealPlayEx) real_play_ = nullptr;
    decltype(&CLIENT_SetRealDataCallBackEx2) set_callback_ = nullptr;
    decltype(&CLIENT_StopRealPlayEx) stop_ = nullptr;
    decltype(&CLIENT_GetLastError) last_error_ = nullptr;
    LLONG handle_ = 0;
};

struct StreamResult {
    int channel = 0;
    bool success = false;
    std::string error;
    double duration_seconds = 0.0;
    std::uint64_t packets = 0;
    std::uint64_t bytes = 0;
    double megabits_per_second = 0.0;
    std::uint64_t queue_dropped = 0;
    std::size_t queue_hwm = 0;
    std::size_t queue_depth_at_stop = 0;
    std::uint64_t ignored_packets = 0;
    std::uint64_t input_retries = 0;
    std::uint64_t input_failures = 0;
    std::uint64_t decoded_callbacks = 0;
    double decoded_fps = 0.0;
    long width = 0;
    long height = 0;
    long frame_rate = 0;
    long frame_type = 0;
    std::uint64_t capture_bytes = 0;
};

StreamResult run_stream(
    const std::filesystem::path& net_root,
    const std::filesystem::path& play_root,
    sentrium::device::Session session,
    int channel,
    DH_RealPlayType stream_type,
    int duration_seconds,
    const std::string& capture_path_text) {
    StreamResult result;
    result.channel = channel;

    try {
        PacketQueue queue(128);
        Metrics metrics;
        CallbackContext callback_context{&queue, &metrics};

        PlaySdk play(play_root);
        play.open(metrics);

        std::ofstream capture;
        constexpr std::uint64_t capture_limit = 8ULL * 1024ULL * 1024ULL;
        std::uint64_t captured = 0;
        if (!capture_path_text.empty()) {
            capture.open(capture_path_text, std::ios::binary | std::ios::trunc);
            if (!capture) {
                throw std::runtime_error("could not create media capture file");
            }
        }

        RealPlay real_play(
            net_root,
            session,
            channel,
            stream_type,
            callback_context);

        std::atomic<bool> stop_consumer{false};
        std::jthread consumer([&](std::stop_token token) {
            while (!token.stop_requested()) {
                auto packet = queue.try_pop();
                if (!packet) {
                    if (stop_consumer.load()) {
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }

                if (packet->data_type != 0) {
                    continue;
                }

                if (capture.is_open() && captured < capture_limit) {
                    const auto remaining = capture_limit - captured;
                    const auto count = static_cast<std::streamsize>(
                        std::min<std::uint64_t>(remaining, packet->bytes.size()));
                    capture.write(
                        reinterpret_cast<const char*>(packet->bytes.data()),
                        count);
                    if (!capture) {
                        throw std::runtime_error("media capture write failed");
                    }
                    captured += static_cast<std::uint64_t>(count);
                }

                bool accepted = play.input(packet->bytes.data(), packet->bytes.size());
                for (int retry = 0;
                     !accepted && retry < 50 && !token.stop_requested();
                     ++retry) {
                    ++metrics.play_input_retries;
                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                    accepted = play.input(packet->bytes.data(), packet->bytes.size());
                }
                if (!accepted) {
                    ++metrics.play_input_failures;
                }
            }
        });

        const auto started = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::seconds(duration_seconds));
        const auto finished = std::chrono::steady_clock::now();

        real_play.stop();
        stop_consumer = true;
        consumer.join();
        play.close();

        if (capture.is_open()) {
            capture.flush();
            if (!capture) {
                throw std::runtime_error("media capture flush failed");
            }
            capture.close();
        }

        const auto queue_metrics = queue.metrics();
        result.duration_seconds =
            std::chrono::duration<double>(finished - started).count();
        result.packets = metrics.callback_packets.load();
        result.bytes = metrics.callback_bytes.load();
        result.queue_dropped = queue_metrics.dropped;
        result.queue_hwm = queue_metrics.high_water_mark;
        result.queue_depth_at_stop = queue_metrics.depth;
        result.ignored_packets = metrics.ignored_packets.load();
        result.input_retries = metrics.play_input_retries.load();
        result.input_failures = metrics.play_input_failures.load();
        result.decoded_callbacks = metrics.decoded_callbacks.load();
        result.width = metrics.last_width.load();
        result.height = metrics.last_height.load();
        result.frame_rate = metrics.last_frame_rate.load();
        result.frame_type = metrics.last_frame_type.load();
        result.capture_bytes = captured;

        if (result.duration_seconds > 0.0) {
            result.megabits_per_second =
                (static_cast<double>(result.bytes) * 8.0 / 1'000'000.0)
                / result.duration_seconds;
            result.decoded_fps =
                static_cast<double>(result.decoded_callbacks)
                / result.duration_seconds;
        }

        result.success =
            result.packets > 0
            && result.decoded_callbacks > 0
            && result.input_failures == 0;
    } catch (const std::exception& error) {
        result.error = error.what();
        result.success = false;
    }

    return result;
}

std::string json_bool(bool value) {
    return value ? "true" : "false";
}

} // namespace

int main() {
    using namespace sentrium;

    try {
        const auto host = environment("SENTRIUM_MEDIA_HOST");
        const auto user = environment("SENTRIUM_MEDIA_USER");
        const auto password = environment("SENTRIUM_MEDIA_PASSWORD");
        const int port = integer_environment("SENTRIUM_MEDIA_PORT", 37777, 1, 65535);
        const auto channels = media_channels();
        const int duration_seconds =
            integer_environment("SENTRIUM_MEDIA_DURATION", 10, 2, 3600);
        const auto capture_path_text = environment("SENTRIUM_MEDIA_CAPTURE", false);

        const auto requested_stream = environment("SENTRIUM_MEDIA_STREAM", false);
        std::string stream_name = "Principal";
        DH_RealPlayType stream_type = DH_RType_Realplay;
        if (!requested_stream.empty()) {
            if (requested_stream == "principal") {
                stream_name = "Principal";
                stream_type = DH_RType_Realplay;
            } else if (requested_stream == "extra") {
                stream_name = "Extra";
                stream_type = DH_RType_Realplay_1;
            } else {
                throw std::runtime_error(
                    "SENTRIUM_MEDIA_STREAM must be principal or extra");
            }
        }

        device::LoginRequest request{
            host,
            static_cast<std::uint16_t>(port),
            user,
            password};
        request.validate();

        intelbras::Adapter adapter;
        const auto login = adapter.connect(request);
        if (!login.success) {
            throw std::runtime_error(login.diagnostic);
        }

        struct DisconnectGuard {
            intelbras::Adapter& adapter;
            device::Session session;
            ~DisconnectGuard() { static_cast<void>(adapter.disconnect(session)); }
        } disconnect_guard{adapter, login.session};

        for (const int channel : channels) {
            if (channel >= login.channel_count) {
                throw std::runtime_error(
                    "requested channel exceeds device channel count");
            }
        }

        if (channels.size() > 1 && !capture_path_text.empty()) {
            throw std::runtime_error(
                "SENTRIUM_MEDIA_CAPTURE is supported only for a single channel");
        }

        const auto net_root = netsdk_root();
        const auto play_root = playsdk_root();
        std::vector<StreamResult> results(channels.size());
        std::vector<std::jthread> workers;
        workers.reserve(channels.size());

        const auto wall_started = std::chrono::steady_clock::now();
        for (std::size_t index = 0; index < channels.size(); ++index) {
            workers.emplace_back([&, index] {
                results[index] = run_stream(
                    net_root,
                    play_root,
                    login.session,
                    channels[index],
                    stream_type,
                    duration_seconds,
                    capture_path_text);
            });
        }
        for (auto& worker : workers) {
            worker.join();
        }
        const auto wall_finished = std::chrono::steady_clock::now();
        const double wall_seconds =
            std::chrono::duration<double>(wall_finished - wall_started).count();

        bool pass = true;
        std::uint64_t total_packets = 0;
        std::uint64_t total_bytes = 0;
        std::uint64_t total_dropped = 0;
        std::uint64_t total_input_retries = 0;
        std::uint64_t total_input_failures = 0;
        std::uint64_t total_decoded = 0;
        std::size_t max_hwm = 0;

        for (const auto& result : results) {
            pass = pass && result.success;
            total_packets += result.packets;
            total_bytes += result.bytes;
            total_dropped += result.queue_dropped;
            total_input_retries += result.input_retries;
            total_input_failures += result.input_failures;
            total_decoded += result.decoded_callbacks;
            max_hwm = std::max(max_hwm, result.queue_hwm);
        }

        const double aggregate_mbps =
            wall_seconds > 0.0
                ? (static_cast<double>(total_bytes) * 8.0 / 1'000'000.0)
                    / wall_seconds
                : 0.0;
        const double aggregate_decoded_fps =
            wall_seconds > 0.0
                ? static_cast<double>(total_decoded) / wall_seconds
                : 0.0;

        std::cout
            << "{"
            << "\"Result\":\"" << (pass ? "PASS" : "FAIL") << "\","
            << "\"Foundation\":\"" << core::foundation << "\","
            << "\"Version\":\"" << core::version << "\","
            << "\"BuildId\":\"" << core::build_id << "\","
            << "\"Backend\":\"PlaySDK\","
            << "\"Transport\":\"NetSDK-LAN\","
            << "\"Stream\":\"" << stream_name << "\","
            << "\"ChannelCount\":" << channels.size() << ","
            << "\"WallSeconds\":" << wall_seconds << ","
            << "\"Packets\":" << total_packets << ","
            << "\"Bytes\":" << total_bytes << ","
            << "\"MegabitsPerSecond\":" << aggregate_mbps << ","
            << "\"QueueDropped\":" << total_dropped << ","
            << "\"MaxQueueHighWaterMark\":" << max_hwm << ","
            << "\"InputRetries\":" << total_input_retries << ","
            << "\"InputFailures\":" << total_input_failures << ","
            << "\"DecodedCallbacks\":" << total_decoded << ","
            << "\"DecodedFps\":" << aggregate_decoded_fps << ","
            << "\"Channels\":[";

        for (std::size_t index = 0; index < results.size(); ++index) {
            const auto& result = results[index];
            if (index != 0) {
                std::cout << ",";
            }
            std::cout
                << "{"
                << "\"ChannelIndex\":" << result.channel << ","
                << "\"Result\":\"" << (result.success ? "PASS" : "FAIL") << "\","
                << "\"DurationSeconds\":" << result.duration_seconds << ","
                << "\"Packets\":" << result.packets << ","
                << "\"Bytes\":" << result.bytes << ","
                << "\"MegabitsPerSecond\":" << result.megabits_per_second << ","
                << "\"QueueDropped\":" << result.queue_dropped << ","
                << "\"QueueHighWaterMark\":" << result.queue_hwm << ","
                << "\"QueueDepthAtStop\":" << result.queue_depth_at_stop << ","
                << "\"IgnoredPackets\":" << result.ignored_packets << ","
                << "\"InputRetries\":" << result.input_retries << ","
                << "\"InputFailures\":" << result.input_failures << ","
                << "\"DecodedCallbacks\":" << result.decoded_callbacks << ","
                << "\"DecodedFps\":" << result.decoded_fps << ","
                << "\"LastWidth\":" << result.width << ","
                << "\"LastHeight\":" << result.height << ","
                << "\"LastFrameRate\":" << result.frame_rate << ","
                << "\"LastFrameType\":" << result.frame_type << ","
                << "\"CaptureWritten\":" << json_bool(result.capture_bytes > 0) << ","
                << "\"CaptureBytes\":" << result.capture_bytes;
            if (!result.error.empty()) {
                std::cout << ",\"Error\":\"stream-failed\"";
            }
            std::cout << "}";
        }

        std::cout << "]}\n";
        return pass ? 0 : 6;
    } catch (const std::exception& error) {
        std::cerr
            << "{\"Result\":\"FAIL\",\"Component\":\"sentrium-media-lab\","
            << "\"Message\":\"" << error.what() << "\"}\n";
        return 5;
    }
}
