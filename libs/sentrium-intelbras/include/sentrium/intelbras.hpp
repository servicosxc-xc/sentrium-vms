#pragma once

#include <sentrium/device.hpp>

#include <memory>

namespace sentrium::intelbras {

class Adapter final : public device::Adapter {
public:
    Adapter();
    ~Adapter() override;

    Adapter(const Adapter&) = delete;
    Adapter& operator=(const Adapter&) = delete;

    device::ConnectResult connect(const device::LoginRequest& request) override;
    device::DisconnectResult disconnect(device::Session session) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace sentrium::intelbras
