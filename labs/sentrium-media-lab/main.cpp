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
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

namespace {

using sentrium::media::Packet;
using sentrium::media::PacketQueue;

std::string environment(const char* name, bool required = true) {
    const char* value = std::getenv(name);
    if (!value || *value == '\0') {
        if (required) {
            throw std::runtime_error(std::string("missing environment variable: ") + name);
        }
        return {};
    }
    return value;
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

std::filesystem::path sdk_root() {
    const DWORD required =
        GetEnvironmentVariableW(L"SENTRIUM_INTELBRAS_SDK_ROOT", nullptr, 0);
    if (required == 0) {
        throw std::runtime_error("SENTRIUM_INTELBRAS_SDK_ROOT is not available");
    }

    std::wstring buffer(static_cast<std::size_t>(required), L'\0');
    const DWORD written = GetEnvironmentVariableW(
        L"SENTRIUM_INTELBRAS_SDK_ROOT", buffer.data(), required);
    if (written == 0 || written >= required) {
        throw std::runtime_error("SENTRIUM_INTELBRAS_SDK_ROOT could not be read");
    }
    buffer.resize(written);
    return std::filesystem::path(buffer);
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

void CALLBACK real_data_callback(
    LLONG,
    DWORD data_type,
    BYTE* buffer,
    DWORD size,
    LONG,
    LDWORD user) noexcept {
    auto* context = reinterpret_cast<CallbackContext*>(
        static_cast<std::uintptr_t>(user));
    if (!context || !context->queue || !context->metrics || !buffer || size == 0) {
        return;
    }

    // A malformed callback must not force an unbounded allocation in this lab.
    constexpr DWORD max_packet_bytes = 16U * 1024U * 1024U;
    if (size > max_packet_bytes) {
        ++context->metrics->ignored_packets;
        return;
    }

    try {
        Packet packet;
        packet.data_type = data_type;
        packet.sequence = context->sequence.fetch_add(1, std::memory_order_relaxed);
        packet.arrival = std::chrono::steady_clock::now();
        packet.bytes.resize(size);
        std::memcpy(packet.bytes.data(), buffer, size);

        ++context->metrics->callback_packets;
        context->metrics->callback_bytes.fetch_add(size, std::memory_order_relaxed);
        context->queue->push(std::move(packet));
    } catch (...) {
        ++context->metrics->ignored_packets;
    }
}

void CALLBACK decode_callback(
    LONG,
    char*,
    LONG,
    FRAME_INFO* frame,
    void* user,
    LONG) noexcept {
    auto* metrics = static_cast<Metrics*>(user);
    if (!metrics || !frame) {
        return;
    }

    ++metrics->decoded_callbacks;
    metrics->last_width.store(frame->nWidth, std::memory_order_relaxed);
    metrics->last_height.store(frame->nHeight, std::memory_order_relaxed);
    metrics->last_frame_rate.store(frame->nFrameRate, std::memory_order_relaxed);
    metrics->last_frame_type.store(frame->nType, std::memory_order_relaxed);
}

class PlaySdk final {
public:
    explicit PlaySdk(const std::filesystem::path& root)
        : module_(root / L"bin" / L"dhplay.dll") {
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
        if (!set_dec_callback_(port_, &decode_callback, &metrics)) {
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
            DH_RType_Realplay);
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
                &real_data_callback,
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
        const int channel = integer_environment("SENTRIUM_MEDIA_CHANNEL", 0, 0, 4095);
        const int duration_seconds =
            integer_environment("SENTRIUM_MEDIA_DURATION", 10, 2, 3600);
        const auto capture_path_text = environment("SENTRIUM_MEDIA_CAPTURE", false);

        // First authorized slice intentionally proves Principal only. Extra mapping
        // is added after the official enum values are inspected in the local kit.
        const auto requested_stream = environment("SENTRIUM_MEDIA_STREAM", false);
        if (!requested_stream.empty() && requested_stream != "principal") {
            throw std::runtime_error(
                "this first Media Lab slice accepts SENTRIUM_MEDIA_STREAM=principal only");
        }

        device::LoginRequest request{
            host,
            static_cast<std::uint16_t>(port),
            user,
            password};
        request.validate();

        intelbras::Adapter adapter;
        const auto login = adapter.connect(request);
        if (!login.ok) {
            throw std::runtime_error(login.diagnostic);
        }

        struct DisconnectGuard {
            intelbras::Adapter& adapter;
            device::Session session;
            ~DisconnectGuard() { static_cast<void>(adapter.disconnect(session)); }
        } disconnect_guard{adapter, login.session};

        if (channel >= login.channel_count) {
            throw std::runtime_error("requested channel exceeds device channel count");
        }

        PacketQueue queue(128);
        Metrics metrics;
        CallbackContext callback_context{&queue, &metrics};

        PlaySdk play(sdk_root());
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
            sdk_root(),
            login.session,
            channel,
            callback_context);

        std::atomic<bool> stop_consumer{false};
        std::jthread consumer([&](std::stop_token token) {
            while (!token.stop_requested() && !stop_consumer.load()) {
                auto packet = queue.try_pop();
                if (!packet) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }

                // The NetSDK original-stream flag is expected to produce dwDataType=0
                // for the stream bytes consumed by PlaySDK. Other types are retained
                // in ingest metrics but are not fed to the decoder.
                if (packet->data_type != 0) {
                    continue;
                }

                if (capture && captured < capture_limit) {
                    const auto remaining = capture_limit - captured;
                    const auto count = static_cast<std::streamsize>(
                        std::min<std::uint64_t>(remaining, packet->bytes.size()));
                    capture.write(
                        reinterpret_cast<const char*>(packet->bytes.data()),
                        count);
                    captured += static_cast<std::uint64_t>(count);
                }

                bool accepted = play.input(packet->bytes.data(), packet->bytes.size());
                for (int retry = 0; !accepted && retry < 50 && !token.stop_requested(); ++retry) {
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
        consumer.request_stop();
        consumer.join();
        play.close();
        if (capture) {
            capture.flush();
            capture.close();
        }

        const auto queue_metrics = queue.metrics();
        const auto elapsed =
            std::chrono::duration<double>(finished - started).count();
        const auto packet_count = metrics.callback_packets.load();
        const auto byte_count = metrics.callback_bytes.load();
        const auto decoded = metrics.decoded_callbacks.load();
        const double megabits_per_second =
            elapsed > 0.0 ? (static_cast<double>(byte_count) * 8.0 / 1'000'000.0) / elapsed : 0.0;
        const double decoded_fps =
            elapsed > 0.0 ? static_cast<double>(decoded) / elapsed : 0.0;

        const bool pass = packet_count > 0 && decoded > 0;

        std::cout
            << "{"
            << "\"Result\":\"" << (pass ? "PASS" : "FAIL") << "\","
            << "\"Foundation\":\"" << core::foundation << "\","
            << "\"Version\":\"" << core::version << "\","
            << "\"BuildId\":\"" << core::build_id << "\","
            << "\"Backend\":\"PlaySDK\","
            << "\"Transport\":\"NetSDK-LAN\","
            << "\"Stream\":\"Principal\","
            << "\"ChannelIndex\":" << channel << ","
            << "\"DurationSeconds\":" << elapsed << ","
            << "\"Packets\":" << packet_count << ","
            << "\"Bytes\":" << byte_count << ","
            << "\"MegabitsPerSecond\":" << megabits_per_second << ","
            << "\"QueueDropped\":" << queue_metrics.dropped << ","
            << "\"QueueHighWaterMark\":" << queue_metrics.high_water_mark << ","
            << "\"QueueDepthAtStop\":" << queue_metrics.depth << ","
            << "\"IgnoredPackets\":" << metrics.ignored_packets.load() << ","
            << "\"InputRetries\":" << metrics.play_input_retries.load() << ","
            << "\"InputFailures\":" << metrics.play_input_failures.load() << ","
            << "\"DecodedCallbacks\":" << decoded << ","
            << "\"DecodedFps\":" << decoded_fps << ","
            << "\"LastWidth\":" << metrics.last_width.load() << ","
            << "\"LastHeight\":" << metrics.last_height.load() << ","
            << "\"LastFrameRate\":" << metrics.last_frame_rate.load() << ","
            << "\"LastFrameType\":" << metrics.last_frame_type.load() << ","
            << "\"CaptureWritten\":" << json_bool(captured > 0) << ","
            << "\"CaptureBytes\":" << captured
            << "}\n";

        return pass ? 0 : 6;
    } catch (const std::exception& error) {
        std::cerr
            << "{\"Result\":\"FAIL\",\"Component\":\"sentrium-media-lab\","
            << "\"Message\":\"" << error.what() << "\"}\n";
        return 5;
    }
}
