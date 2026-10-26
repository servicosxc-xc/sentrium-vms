#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace sentrium::device {

// One synchronous, bounded datagram exchange to a caller-authorized IPv4
// endpoint. Does not discover Cloud servers, authenticate or establish P2P.
// Call only from a worker thread, never the Qt UI thread.
struct CloudUdpSettings {
    std::string server_ipv4;
    std::uint16_t server_port = 0;
    int timeout_ms = 1500;
    int attempts = 2;

    void validate() const;
};

enum class CloudUdpError {
    None,
    InvalidConfiguration,
    NetworkInitialization,
    SocketFailure,
    SendFailure,
    Timeout,
    ReceiveFailure,
    InvalidReply
};

struct CloudUdpResult {
    CloudUdpError error = CloudUdpError::InvalidConfiguration;
    std::string reply; // Sensitive wire data. Never persist or log.
    int attempts_used = 0;
    [[nodiscard]] bool ok() const noexcept { return error == CloudUdpError::None; }
};

// The request is created by an independently verified, authorized protocol
// layer. This transport NEVER invents bootstrap endpoints, identities or keys.
// A connected UDP socket prevents packets from unrelated peer addresses.
[[nodiscard]] CloudUdpResult exchangeCloudDatagram(
    const CloudUdpSettings& settings, std::string_view request);

} // namespace sentrium::device
