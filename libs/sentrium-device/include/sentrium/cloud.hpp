#pragma once

#include <sentrium/device.hpp>

#include <cstdint>
#include <memory>
#include <string>

namespace sentrium::device {

// Cloud UUID and cloud service port are not the same as NetSDK host/port.
// No port is assumed: the observed 37000 applies only to one legacy device.
struct CloudConnectRequest {
    std::string cloud_id;
    std::uint16_t service_port = 0;

    void validate() const;
};

// Created only by a real Cloud provider after a tunnel is ready. The native
// rendezvous implementation is intentionally not guessed here.
struct CloudTunnelEndpoint {
    std::uint16_t local_port = 0;
    CloudRoute observed_route = CloudRoute::Unknown;

    void validate() const;
    LoginRequest netSdkLogin(std::string username, std::string password) const;
};

// Ownership belongs to the logical device, not a RealPlay tile. The caller
// MUST logout every NetSDK session before calling close()/destroying tunnel.
class CloudTunnel {
public:
    virtual ~CloudTunnel() = default;
    virtual CloudTunnelEndpoint endpoint() const = 0;
    virtual void close() noexcept = 0;
};

// Native standalone implementation will satisfy this contract only after the
// real bootstrap/protocol and distribution rights are verified. No SIM Next
// DLL or artificial "working" tunnel is provided by this interface.
class CloudTransport {
public:
    virtual ~CloudTransport() = default;
    virtual std::unique_ptr<CloudTunnel> open(const CloudConnectRequest& request) = 0;
};

} // namespace sentrium::device
