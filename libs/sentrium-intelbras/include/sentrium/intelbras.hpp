#pragma once

#include <sentrium/device.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace sentrium::intelbras {

struct RegistrationNotice {
    std::string registration_id;
    std::string peer_address;
    std::uint16_t peer_port = 0;
    bool disconnected = false;
};

using RegistrationCallback = std::function<void(const RegistrationNotice&)>;

struct ListenerResult {
    bool success = false;
    std::string diagnostic;
};

class Adapter final : public device::Adapter {
public:
    Adapter();
    ~Adapter() override;

    Adapter(const Adapter&) = delete;
    Adapter& operator=(const Adapter&) = delete;

    device::ConnectResult connect(const device::LoginRequest& request) override;
    device::DisconnectResult disconnect(device::Session session) override;

    // Live window MVP: NetSDK owns native rendering to operator-created HWND.
    // This does NOT implement a Qt mosaic or provide decoded frames for AI.
    device::ConnectResult startLive(
        device::Session loginSession, int zeroBasedChannel,
        std::uintptr_t nativeWindow, bool extraStream);
    device::DisconnectResult stopLive(device::Session liveSession);

    // NetSDK reverse-connect server. Explicit opt-in: starts listening only
    // after the user chooses bind address/port in the client.
    ListenerResult startRegistration(
        const std::string& bind_address,
        std::uint16_t port,
        RegistrationCallback callback);
    void stopRegistration() noexcept;
    device::ConnectResult connectRegistered(
        const device::LoginRequest& peer,
        const std::string& expected_registration_id);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace sentrium::intelbras
