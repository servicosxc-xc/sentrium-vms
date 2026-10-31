#include <sentrium/cloud_signaling.hpp>

#include <utility>

namespace sentrium::device {

CloudSignalingResult exchangeCloudSignaling(
    const CloudUdpSettings& settings, std::string_view authenticatedRequest) {
    const CloudUdpResult received = exchangeCloudDatagram(settings, authenticatedRequest);
    if (!received.ok()) {
        CloudSignalingStatus status = CloudSignalingStatus::NetworkUnavailable;
        switch (received.error) {
        case CloudUdpError::InvalidConfiguration:
            status = CloudSignalingStatus::InvalidConfiguration;
            break;
        case CloudUdpError::Timeout:
            status = CloudSignalingStatus::Timeout;
            break;
        case CloudUdpError::InvalidReply:
            status = CloudSignalingStatus::MalformedResponse;
            break;
        default:
            status = CloudSignalingStatus::NetworkUnavailable;
            break;
        }
        return {status, 0, received.attempts_used, {}};
    }

    CloudWireResult parsed = parseCloudWireResponse(received.reply);
    if (!parsed.ok()) {
        return {CloudSignalingStatus::MalformedResponse, 0,
                received.attempts_used, {}};
    }
    const int code = parsed.response.status_code;
    // A syntactically valid 401/403/404 must NEVER be reported as connected.
    const auto status = code == 200
        ? CloudSignalingStatus::Accepted : CloudSignalingStatus::Rejected;
    return {status, code, received.attempts_used, std::move(parsed.response)};
}

} // namespace sentrium::device
