#include <sentrium/cloud_stream_tunnel.hpp>

#include <stdexcept>
#include <utility>

namespace sentrium::device {

CloudStreamTunnel::CloudStreamTunnel(
    std::unique_ptr<CloudLoopbackBridge> bridge) noexcept
    : bridge_(std::move(bridge)) {}

std::unique_ptr<CloudStreamTunnel> CloudStreamTunnel::create(
    std::unique_ptr<CloudByteStream> authenticated_stream) {
    // Fail closed for null, no fallback to an unverified application
    // bridge, hard-coded Cloud host, or locally fabricated Direct/Relay.
    auto local = CloudLoopbackBridge::start(std::move(authenticated_stream));
    return std::unique_ptr<CloudStreamTunnel>(
        new CloudStreamTunnel(std::move(local)));
}

CloudStreamTunnel::~CloudStreamTunnel() noexcept { close(); }

CloudTunnelEndpoint CloudStreamTunnel::endpoint() const {
    const std::uint16_t port = bridge_ ? bridge_->localPort() : 0;
    if (port == 0)
        throw std::runtime_error("Cloud TCP bridge is no longer listening");
    // Unknown is the only defensible route until an authenticated provider
    // exposes proven Direct/Relay negotiation/telemetry.
    return {port, CloudRoute::Unknown};
}

void CloudStreamTunnel::close() noexcept {
    if (bridge_) {
        bridge_->close();
        bridge_.reset();
    }
}

} // namespace sentrium::device
