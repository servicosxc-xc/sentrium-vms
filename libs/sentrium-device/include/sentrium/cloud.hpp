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

struct CloudSessionResult {
    bool success = false;
    int channel_count = 0;
    CloudRoute route = CloudRoute::Unknown;
    std::string diagnostic;
};

// Reuses the existing NetSDK adapter. The tunnel outlives login and RealPlay,
// and logout always precedes closing the tunnel. This is orchestration, not
// a P2P provider: without a real CloudTransport, no Cloud connection exists.
// Caller must ensure transport and NetSDK adapter outlive this object.
class CloudDeviceSession final {
public:
    CloudDeviceSession(CloudTransport& transport, Adapter& net_sdk) noexcept;
    ~CloudDeviceSession() noexcept;

    CloudDeviceSession(const CloudDeviceSession&) = delete;
    CloudDeviceSession& operator=(const CloudDeviceSession&) = delete;

    CloudSessionResult connect(
        const CloudConnectRequest& cloud,
        std::string username,
        std::string password);
    DisconnectResult disconnect() noexcept;
    bool connected() const noexcept;
    CloudRoute route() const noexcept;

private:
    void closeTunnel() noexcept;

    CloudTransport& transport_;
    Adapter& net_sdk_;
    std::unique_ptr<CloudTunnel> tunnel_;
    Session session_{};
    CloudRoute route_ = CloudRoute::Unknown;
};

} // namespace sentrium::device
