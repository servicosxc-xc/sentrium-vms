#include <sentrium/intelbras.hpp>

#include <Windows.h>
#include <dhnetsdk.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>

namespace sentrium::intelbras {
namespace {

std::filesystem::path sdk_root() {
    const DWORD required = GetEnvironmentVariableW(L"SENTRIUM_INTELBRAS_SDK_ROOT", nullptr, 0);
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
T load_export(HMODULE module, const char* export_name) {
    const auto address = GetProcAddress(module, export_name);
    if (!address) {
        throw std::runtime_error(std::string("Required NetSDK export missing: ") + export_name);
    }
    return reinterpret_cast<T>(address);
}

std::string win32_failure(std::string_view action) {
    return std::string(action) + " failed (win32=" + std::to_string(GetLastError()) + ")";
}

std::string login_failure(int login_error, int sdk_error) {
    return "NetSDK login failed (login_error=" + std::to_string(login_error)
        + ", sdk_error=" + std::to_string(sdk_error) + ")";
}

} // namespace

struct Adapter::Impl {
    using InitFn = decltype(&CLIENT_InitEx);
    using AutoReconnectFn = decltype(&CLIENT_SetAutoReconnect);
    using LoginFn = decltype(&CLIENT_LoginEx2);
    using LastErrorFn = decltype(&CLIENT_GetLastError);
    using LogoutFn = decltype(&CLIENT_Logout);
    using CleanupFn = decltype(&CLIENT_Cleanup);
    using ListenFn = decltype(&CLIENT_ListenServer);
    using StopListenFn = decltype(&CLIENT_StopListenServer);
    using RealPlayFn = decltype(&CLIENT_RealPlayEx);
    using StopRealPlayFn = decltype(&CLIENT_StopRealPlayEx);

    HMODULE module = nullptr;
    InitFn init = nullptr;
    AutoReconnectFn set_auto_reconnect = nullptr;
    LoginFn login = nullptr;
    LastErrorFn last_error = nullptr;
    LogoutFn logout = nullptr;
    CleanupFn cleanup = nullptr;
    ListenFn listen = nullptr;
    StopListenFn stop_listen = nullptr;
    RealPlayFn real_play = nullptr;
    StopRealPlayFn stop_real_play = nullptr;
    LLONG listener_handle = 0;
    std::mutex listener_mutex;
    RegistrationCallback listener_callback;
    std::mutex session_callback_mutex;
    SessionStatusCallback session_callback;
    bool initialized = false;

    static void dispatch_session_status(LLONG login_id, LDWORD user, bool online) noexcept {
        if (!user || !login_id) return;
        auto* self = reinterpret_cast<Impl*>(static_cast<std::uintptr_t>(user));
        SessionStatusCallback callback;
        {
            std::lock_guard lock(self->session_callback_mutex);
            callback = self->session_callback;
        }
        if (!callback) return;
        try {
            callback(device::Session{static_cast<std::int64_t>(login_id)}, online);
        } catch (...) {
            // Never unwind across a native SDK callback boundary.
        }
    }

    static void CALLBACK disconnected(LLONG login_id, char*, LONG, LDWORD user) noexcept {
        dispatch_session_status(login_id, user, false);
    }

    static int CALLBACK service_notice(
        LLONG, char* peer, WORD port, LONG command,
        void* data, DWORD bytes, LDWORD user) noexcept {
        if (!user || !data) {
            return 0;
        }
        auto* self = reinterpret_cast<Impl*>(static_cast<std::uintptr_t>(user));
        if (command != DH_DVR_SERIAL_RETURN &&
            command != DH_DVR_DISCONNECT &&
            command != NET_DEV_AUTOREGISTER_RETURN) {
            return 0;
        }

        try {
            // This callback is issued by an SDK-owned thread. Copy only bounded
            // fields, never persist token/credentials or hold SDK buffers.
            const char* registration_id = nullptr;
            std::size_t bound = 0;
            if (command == NET_DEV_AUTOREGISTER_RETURN) {
                if (bytes < sizeof(NET_CB_AUTOREGISTER)) {
                    return 0;
                }
                const auto* payload = static_cast<const NET_CB_AUTOREGISTER*>(data);
                registration_id = payload->szDevSerial;
                bound = sizeof(payload->szDevSerial);
            } else {
                registration_id = static_cast<const char*>(data);
                bound = (std::min)(static_cast<std::size_t>(bytes),
                                   static_cast<std::size_t>(DH_DEV_SERIALNO_LEN));
            }
            if (!registration_id || bound == 0) {
                return 0;
            }
            std::size_t length = 0;
            while (length < bound && registration_id[length] != '\0') {
                const unsigned char ch = static_cast<unsigned char>(registration_id[length]);
                if (!(std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.')) {
                    return 0;
                }
                ++length;
            }
            // Legacy registration ID callbacks may pass an exact byte length without a
            // trailing NUL. Fixed NET_CB_AUTOREGISTER buffers must be terminated.
            if (length == 0 ||
                (length == bound &&
                 (command == NET_DEV_AUTOREGISTER_RETURN ||
                  length >= static_cast<std::size_t>(DH_DEV_SERIALNO_LEN)))) {
                return 0;
            }
            RegistrationNotice notice;
            notice.registration_id.assign(registration_id, length);
            notice.peer_port = port;
            notice.disconnected = command == DH_DVR_DISCONNECT;
            if (peer) {
                const std::size_t ip_limit = 64;
                std::size_t ip_length = 0;
                while (ip_length < ip_limit && peer[ip_length] != '\0') {
                    ++ip_length;
                }
                if (ip_length == ip_limit) {
                    return 0;
                }
                notice.peer_address.assign(peer, ip_length);
            }
            RegistrationCallback handler;
            {
                std::lock_guard lock(self->listener_mutex);
                handler = self->listener_callback;
            }
            if (handler) {
                handler(notice);
            }
        } catch (...) {
            // Never propagate exceptions through an SDK callback.
        }
        return 0;
    }
    static void CALLBACK reconnected(LLONG login_id, char*, LONG, LDWORD user) noexcept {
        dispatch_session_status(login_id, user, true);
    }

    ~Impl() {
        if (listener_handle && stop_listen) {
            stop_listen(listener_handle);
            listener_handle = 0;
        }
        {
            std::lock_guard lock(listener_mutex);
            listener_callback = {};
        }
        {
            std::lock_guard lock(session_callback_mutex);
            session_callback = {};
        }
        if (initialized && cleanup) {
            cleanup();
            initialized = false;
        }
        if (module) {
            FreeLibrary(module);
            module = nullptr;
        }
    }

    void ensure_initialized() {
        if (initialized) {
            return;
        }

        const auto dll = sdk_root() / L"bin" / L"dhnetsdk.dll";
        module = LoadLibraryExW(
            dll.c_str(), nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!module) {
            throw std::runtime_error(win32_failure("NetSDK load"));
        }

        try {
            init = load_export<InitFn>(module, "CLIENT_InitEx");
            set_auto_reconnect =
                load_export<AutoReconnectFn>(module, "CLIENT_SetAutoReconnect");
            login = load_export<LoginFn>(module, "CLIENT_LoginEx2");
            last_error = load_export<LastErrorFn>(module, "CLIENT_GetLastError");
            logout = load_export<LogoutFn>(module, "CLIENT_Logout");
            cleanup = load_export<CleanupFn>(module, "CLIENT_Cleanup");
            listen = load_export<ListenFn>(module, "CLIENT_ListenServer");
            stop_listen = load_export<StopListenFn>(module, "CLIENT_StopListenServer");
            real_play = load_export<RealPlayFn>(module, "CLIENT_RealPlayEx");
            stop_real_play = load_export<StopRealPlayFn>(module, "CLIENT_StopRealPlayEx");

            const auto user_context = static_cast<LDWORD>(
                reinterpret_cast<std::uintptr_t>(this));
            if (!init(&Impl::disconnected, user_context, nullptr)) {
                const int error = last_error ? last_error() : 0;
                throw std::runtime_error(
                    "NetSDK initialization failed (sdk_error=" + std::to_string(error) + ")");
            }

            initialized = true;
            set_auto_reconnect(&Impl::reconnected, user_context);
        } catch (...) {
            if (initialized && cleanup) {
                cleanup();
                initialized = false;
            }
            FreeLibrary(module);
            module = nullptr;
            throw;
        }
    }
};

Adapter::Adapter() : impl_(std::make_unique<Impl>()) {}
Adapter::~Adapter() = default;

void Adapter::setSessionStatusCallback(SessionStatusCallback callback) {
    std::lock_guard lock(impl_->session_callback_mutex);
    impl_->session_callback = std::move(callback);
}

device::ConnectResult Adapter::connect(const device::LoginRequest& request) {
    try {
        request.validate();
        impl_->ensure_initialized();

        NET_DEVICEINFO_Ex info{};
        int login_error = 0;
        const LLONG login_id = impl_->login(
            request.host.c_str(),
            static_cast<WORD>(request.port),
            request.username.c_str(),
            request.password.c_str(),
            EM_LOGIN_SPEC_CAP_TCP,
            nullptr,
            &info,
            &login_error);

        if (login_id == 0) {
            const int sdk_error = impl_->last_error ? impl_->last_error() : 0;
            return device::ConnectResult{
                false, {}, 0, login_failure(login_error, sdk_error)};
        }

        const int channel_count = info.nChanNum;
        if (channel_count < 0 || channel_count > 4096) {
            impl_->logout(login_id);
            return device::ConnectResult{
                false, {}, 0, "NetSDK returned an invalid channel count"};
        }

        return device::ConnectResult{
            true,
            device::Session{static_cast<std::int64_t>(login_id)},
            channel_count,
            "Connected by NetSDK over IP/LAN"};
    } catch (const std::exception& error) {
        return device::ConnectResult{false, {}, 0, error.what()};
    }
}

device::DisconnectResult Adapter::disconnect(device::Session session) {
    if (!session) {
        return device::DisconnectResult{true, {}};
    }

    try {
        impl_->ensure_initialized();
        const BOOL ok = impl_->logout(static_cast<LLONG>(session.value));
        if (!ok) {
            const int sdk_error = impl_->last_error ? impl_->last_error() : 0;
            return device::DisconnectResult{
                false,
                "NetSDK logout failed (sdk_error=" + std::to_string(sdk_error) + ")"};
        }
        return device::DisconnectResult{true, {}};
    } catch (const std::exception& error) {
        return device::DisconnectResult{false, error.what()};
    }
}

device::ConnectResult Adapter::startLive(
    device::Session loginSession,
    int zeroBasedChannel,
    std::uintptr_t nativeWindow,
    bool extraStream) {
    // Decode/display is owned by the vendor runtime in a separately created
    // native Qt window. We do not claim success until an operator sees video.
    if (!loginSession || zeroBasedChannel < 0 || zeroBasedChannel >= 4096
        || nativeWindow == 0) {
        return {false, {}, 0, "Invalid live preview session, channel or window"};
    }
    try {
        impl_->ensure_initialized();
        const auto playType = extraStream
            ? DH_RType_Realplay_1 : DH_RType_Realplay;
        const auto hwnd = reinterpret_cast<HWND>(nativeWindow);
        const LLONG live = impl_->real_play(
            static_cast<LLONG>(loginSession.value),
            zeroBasedChannel, hwnd, playType);
        if (live == 0) {
            const int error = impl_->last_error ? impl_->last_error() : 0;
            return {false, {}, 0,
                    "NetSDK RealPlayEx failed (sdk_error=" + std::to_string(error) + ")"};
        }
        return {true, device::Session{static_cast<std::int64_t>(live)}, 0,
                "NetSDK RealPlayEx handle started; visual frame pending validation"};
    } catch (const std::exception&) {
        return {false, {}, 0, "NetSDK RealPlayEx initialization failed"};
    }
}

device::DisconnectResult Adapter::stopLive(device::Session liveSession) {
    if (!liveSession) return {true, {}};
    try {
        impl_->ensure_initialized();
        if (!impl_->stop_real_play(static_cast<LLONG>(liveSession.value))) {
            const int error = impl_->last_error ? impl_->last_error() : 0;
            return {false, "NetSDK StopRealPlayEx failed (sdk_error="
                    + std::to_string(error) + ")"};
        }
        return {true, {}};
    } catch (const std::exception&) {
        return {false, "NetSDK StopRealPlayEx failed"};
    }
}

ListenerResult Adapter::startRegistration(
    const std::string& bind_address,
    std::uint16_t port,
    RegistrationCallback callback) {
    try {
        if (!callback || bind_address.empty() || bind_address.size() > 15 || port == 0) {
            return {false, "Invalid registration bind configuration"};
        }
        // Limit listener binding to literal IPv4. Public exposure/firewall changes
        // are never performed by SENTRIUM.
        int parts = 0;
        std::size_t start = 0;
        while (start < bind_address.size()) {
            const std::size_t end = bind_address.find('.', start);
            const std::size_t last = end == std::string::npos ? bind_address.size() : end;
            const std::string token = bind_address.substr(start, last - start);
            if (token.empty() || token.size() > 3 ||
                !std::all_of(token.begin(), token.end(), [](unsigned char c) { return c >= '0' && c <= '9'; }) ||
                std::stoi(token) > 255) {
                return {false, "Registration bind address must be literal IPv4"};
            }
            ++parts;
            if (end == std::string::npos) break;
            start = end + 1;
        }
        if (parts != 4 || bind_address.back() == '.') {
            return {false, "Registration bind address must be literal IPv4"};
        }
        impl_->ensure_initialized();
        if (impl_->listener_handle != 0) {
            return {false, "Registration listener is already active"};
        }

        // Register callback before opening the service. NetSDK callback can
        // arrive as soon as ListenServer starts on its internal worker thread.
        {
            std::lock_guard lock(impl_->listener_mutex);
            impl_->listener_callback = std::move(callback);
        }
        std::string bind = bind_address;
        const LLONG handle = impl_->listen(
            bind.data(), static_cast<WORD>(port), 1000,
            &Impl::service_notice,
            static_cast<LDWORD>(reinterpret_cast<std::uintptr_t>(impl_.get())));
        if (handle == 0) {
            const int code = impl_->last_error ? impl_->last_error() : 0;
            std::lock_guard lock(impl_->listener_mutex);
            impl_->listener_callback = {};
            return {false, "NetSDK registration listener failed (sdk_error=" + std::to_string(code) + ")"};
        }
        impl_->listener_handle = handle;
        return {true, "Registration listener active; waiting for configured device registration IDs"};
    } catch (const std::exception&) {
        return {false, "Registration listener initialization failed"};
    }
}

void Adapter::stopRegistration() noexcept {
    if (impl_->listener_handle != 0 && impl_->stop_listen) {
        impl_->stop_listen(impl_->listener_handle);
        impl_->listener_handle = 0;
    }
    std::lock_guard lock(impl_->listener_mutex);
    impl_->listener_callback = {};
}

device::ConnectResult Adapter::connectRegistered(
    const device::LoginRequest& peer,
    const std::string& expected_registration_id) {
    try {
        peer.validate();
        if (expected_registration_id.empty() || expected_registration_id.size() >= DH_DEV_SERIALNO_LEN ||
            !std::all_of(expected_registration_id.begin(), expected_registration_id.end(),
                [](unsigned char c) { return std::isalnum(c) || c == '-' || c == '_' || c == '.'; })) {
            return {false, {}, 0, "Invalid expected auto-registration device ID"};
        }
        impl_->ensure_initialized();
        NET_DEVICEINFO_Ex info{};
        int login_error = 0;
        // Verified against NetSDK 3.050 Win64 header and official
        // VehicleGPS demo: SERVER_CONN passes the user-configured device registration ID
        // via pCapParam, despite the vendor naming the callback buffer DevSerial.
        std::string registration_id = expected_registration_id;
        const LLONG handle = impl_->login(
            peer.host.c_str(), static_cast<WORD>(peer.port),
            peer.username.c_str(), peer.password.c_str(),
            EM_LOGIN_SPEC_CAP_SERVER_CONN,
            registration_id.data(), &info, &login_error);
        if (!handle) {
            const int sdk_error = impl_->last_error ? impl_->last_error() : 0;
            return {false, {}, 0, login_failure(login_error, sdk_error)};
        }
        if (info.nChanNum < 0 || info.nChanNum > 4096) {
            impl_->logout(handle);
            return {false, {}, 0, "NetSDK returned an invalid channel count"};
        }
        return {true, device::Session{static_cast<std::int64_t>(handle)},
                info.nChanNum, "Connected by NetSDK auto-registration"};
    } catch (const std::exception&) {
        return {false, {}, 0, "NetSDK auto-registration login rejected"};
    }
}

} // namespace sentrium::intelbras
