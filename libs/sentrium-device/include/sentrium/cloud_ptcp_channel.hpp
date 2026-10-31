#pragma once

#include <sentrium/cloud_ptcp_receive_window.hpp>
#include <sentrium/cloud_ptcp_session.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace sentrium::device {

// Data-plane for an ALREADY authorized, authenticated and pinned peer.
// Does not discover the Cloud provider, authenticate a peer, select Direct or
// Relay, open UDP sockets, or enable the NetSDK tunnel on its own.
// The caller supplies the peer's initial LMID from a verified handshake.
enum class PtcpChannelError {
    None,
    Duplicate,
    InvalidInput,
    InvalidDatagram,
    WrongRealm,
    OrderingFailed,
    AccountingFailed,
    Backpressure,
    Expired,
    RemoteClosed,
    Closed
};

struct PtcpChannelSend {
    PtcpChannelError error = PtcpChannelError::InvalidInput;
    std::vector<std::string> datagrams; // Opaque, potentially sensitive.
    [[nodiscard]] bool ok() const noexcept {
        return error == PtcpChannelError::None;
    }
};

struct PtcpChannelReceive {
    PtcpChannelError error = PtcpChannelError::InvalidDatagram;
    std::vector<std::string> ordered_bytes; // Confidential NetSDK payloads.
    [[nodiscard]] bool ok() const noexcept {
        return error == PtcpChannelError::None ||
               error == PtcpChannelError::Duplicate;
    }
};

struct PtcpChannelStats {
    PtcpSessionStats session;
    std::size_t reorder_packets = 0;
    std::size_t reorder_bytes = 0;
};

// Single event-loop thread only. Protocol-specific bootstrap, authentication,
// peer pinning, retransmission scheduling and session lifecycle belong to
// the authorized provider. Never feed this from an untrusted UDP socket.
class PtcpPostAuthChannel final {
public:
    PtcpPostAuthChannel(
        std::uint32_t negotiated_realm,
        std::uint32_t verified_first_peer_lmid) noexcept;

    PtcpPostAuthChannel(const PtcpPostAuthChannel&) = delete;
    PtcpPostAuthChannel& operator=(const PtcpPostAuthChannel&) = delete;

    // Up to 16 KiB per call, split into <=1280-byte PTCP payload datagrams.
    // Admission of the entire write is preflighted against the in-flight
    // window so the caller can retry on backpressure without partial writes.
    [[nodiscard]] PtcpChannelSend sendBytes(
        std::string_view bytes, std::uint64_t monotonic_ms);

    // ACK-only heartbeat. Caller decides interval according to the verified
    // protocol contract. No automatic network transmission occurs here.
    [[nodiscard]] PtcpChannelSend heartbeat(std::uint64_t monotonic_ms);

    // Strictly frame-check, reorder by LMID, then update cumulative ACK and
    // byte counters. Payloads are delivered only in reliable order.
    [[nodiscard]] PtcpChannelReceive receiveVerified(std::string_view datagram);

    // Return original bytes of packets due for retransmission. The caller
    // sends them to the *already authenticated* remote peer.
    [[nodiscard]] PtcpChannelSend retransmissions(std::uint64_t monotonic_ms);

    [[nodiscard]] PtcpChannelStats stats() const noexcept;
    [[nodiscard]] bool closed() const noexcept { return closed_; }
    void close() noexcept;

private:
    static constexpr std::size_t kMaxWriteBytes = 16384;
    static constexpr std::size_t kPayloadBytes = 1280;
    static constexpr std::size_t kPendingPackets = 32;
    static constexpr std::size_t kPendingBodyBytes = 65536;

    PtcpReliableSession sender_;
    PtcpReceiveWindow receiver_;
    std::uint32_t realm_ = 0;
    bool closed_ = false;
};

} // namespace sentrium::device
