#include <sentrium/intelbras.hpp>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <dhnetsdk.h>

#include <algorithm>
#include <filesystem>
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

    HMODULE module = nullptr;
    InitFn init = nullptr;
    AutoReconnectFn set_auto_reconnect = nullptr;
    LoginFn login = nullptr;
    LastErrorFn last_error = nullptr;
    LogoutFn logout = nullptr;
    CleanupFn cleanup = nullptr;
    bool initialized = false;

    static void CALLBACK disconnected(LLONG, char*, LONG, LDWORD) noexcept {}
    static void CALLBACK reconnected(LLONG, char*, LONG, LDWORD) noexcept {}

    ~Impl() {
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

            if (!init(&Impl::disconnected, 0, nullptr)) {
                const int error = last_error ? last_error() : 0;
                throw std::runtime_error(
                    "NetSDK initialization failed (sdk_error=" + std::to_string(error) + ")");
            }

            initialized = true;
            set_auto_reconnect(&Impl::reconnected, 0);
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

} // namespace sentrium::intelbras
