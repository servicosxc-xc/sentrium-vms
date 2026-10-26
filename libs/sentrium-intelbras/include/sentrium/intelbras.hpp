#pragma once

#include <sentrium/device.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace sentrium::intelbras {

struct RegistrationNotice {
    std::string serial;
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

    // NetSDK reverse-connect server. Explicit opt-in: starts listening only
    // after the user chooses bind address/port in the client.
    ListenerResult startRegistration(
        const std::string& bind_address,
        std::uint16_t port,
        RegistrationCallback callback);
    void stopRegistration() noexcept;
    device::ConnectResult connectRegistered(
        const device::LoginRequest& peer,
        const std::string& expected_serial);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace sentrium::intelbras
