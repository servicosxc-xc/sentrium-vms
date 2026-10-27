#pragma once
#include <sentrium/cloud.hpp>
#include <sentrium/cloud_loopback_bridge.hpp>

#include <memory>

namespace sentrium::device {

// Adapter between a verified remote byte stream and the already-existing
// CloudDeviceSession/NetSDK orchestration. No CloudTransport implementation
// is supplied: only an independently authorized provider may call create()
// after its own bootstrap, peer verification and reliable byte ordering.
// This wrapper deliberately NEVER reports Direct/Relay without evidence.
class CloudStreamTunnel final : public CloudTunnel {
public:
    static std::unique_ptr<CloudStreamTunnel> create(
        std::unique_ptr<CloudByteStream> authenticated_stream);
    ~CloudStreamTunnel() noexcept override;

    CloudStreamTunnel(const CloudStreamTunnel&) = delete;
    CloudStreamTunnel& operator=(const CloudStreamTunnel&) = delete;

    [[nodiscard]] CloudTunnelEndpoint endpoint() const override;
    void close() noexcept override;

private:
    explicit CloudStreamTunnel(std::unique_ptr<CloudLoopbackBridge> bridge) noexcept;
    std::unique_ptr<CloudLoopbackBridge> bridge_;
};

} // namespace sentrium::device
