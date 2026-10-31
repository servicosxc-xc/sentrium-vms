#pragma once

#include <sentrium/cloud_udp.hpp>
#include <sentrium/cloud_wire.hpp>

#include <string_view>

namespace sentrium::device {

enum class CloudSignalingStatus {
    InvalidConfiguration,
    NetworkUnavailable,
    Timeout,
    MalformedResponse,
    Rejected,
    Accepted
};

struct CloudSignalingResult {
    CloudSignalingStatus status = CloudSignalingStatus::InvalidConfiguration;
    int response_code = 0;
    int attempts_used = 0;
    CloudWireResponse response; // Contains sensitive body; never log.
    [[nodiscard]] bool accepted() const noexcept {
        return status == CloudSignalingStatus::Accepted;
    }
};

// Complete transport + framing exchange for an already-signed and authorized
// signaling request, but NOT Cloud bootstrap or device authentication.
// The caller must supply the legitimate endpoint and authenticated request.
// No vendor identities or secrets are embedded in SENTRIUM.
[[nodiscard]] CloudSignalingResult exchangeCloudSignaling(
    const CloudUdpSettings& settings, std::string_view authenticatedRequest);

} // namespace sentrium::device
