#pragma once
#include <sentrium/cloud_ptcp.hpp>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

namespace sentrium::device {

// Offline session bookkeeping only. The caller MUST independently authenticate
// and pin the peer before providing its packets. This is not a Cloud provider.
enum class PtcpSessionError {
    None, InvalidFrame, WindowFull, InvalidAck, Duplicate,
    SequenceGap, ClockReversed, Expired
};
struct PtcpSendResult {
    PtcpSessionError error = PtcpSessionError::InvalidFrame;
    std::string datagram; // May contain sensitive handshake payload.
    [[nodiscard]] bool ok() const noexcept { return error == PtcpSessionError::None; }
};
struct PtcpReceiveResult {
    PtcpSessionError error = PtcpSessionError::InvalidFrame;
    PtcpPacket packet; // Never log raw/authenticated content.
    [[nodiscard]] bool ok() const noexcept { return error == PtcpSessionError::None; }
};
struct PtcpSessionStats {
    std::uint32_t sent_bytes = 0;
    std::uint32_t received_bytes = 0;
    std::uint32_t peer_acked_bytes = 0;
    std::uint32_t peer_sent_snapshot = 0;
    std::uint32_t next_local_message_id = 0;
    std::uint32_t last_remote_message_id = 0;
    std::size_t in_flight_packets = 0;
    std::size_t in_flight_bytes = 0;
    std::uint64_t retransmissions = 0;
    std::uint64_t duplicates = 0;
};

// Bounded, single-worker-thread primitive. Detected gaps require provider
// recovery/reconnect; there is no invented stream reassembly or NAT handshake.
class PtcpReliableSession final {
public:
    PtcpReliableSession() = default;
    PtcpReliableSession(const PtcpReliableSession&) = delete;
    PtcpReliableSession& operator=(const PtcpReliableSession&) = delete;
    [[nodiscard]] PtcpSendResult send(const PtcpPacket& packet, std::uint64_t now_ms);
    [[nodiscard]] PtcpReceiveResult receive(std::string_view datagram);
    [[nodiscard]] std::vector<std::string> dueRetransmissions(std::uint64_t now_ms);
    [[nodiscard]] PtcpSessionStats stats() const noexcept;
    [[nodiscard]] bool expired() const noexcept { return expired_; }
private:
    struct Pending {
        std::string wire;
        std::uint32_t end_sent = 0;
        std::uint64_t last_sent_ms = 0;
        unsigned attempts = 1;
    };
    static constexpr std::size_t kMaxPending = 32;
    static constexpr std::size_t kMaxPendingBytes = 65536;
    static constexpr std::uint64_t kRetryIntervalMs = 400;
    static constexpr unsigned kMaxAttempts = 4;
    static bool forwardOrEqual(std::uint32_t a, std::uint32_t b) noexcept;
    bool clockValid(std::uint64_t now_ms) noexcept;
    void acknowledge(std::uint32_t peer_received) noexcept;
    std::deque<Pending> pending_;
    std::uint32_t sent_bytes_ = 0, received_bytes_ = 0, peer_acked_ = 0;
    std::uint32_t peer_sent_snapshot_ = 0, next_lmid_ = 0, remote_lmid_ = 0;
    std::uint32_t next_pid_ = 0;
    std::size_t pending_bytes_ = 0;
    std::uint64_t clock_ms_ = 0, retransmissions_ = 0, duplicates_ = 0;
    bool have_clock_ = false, have_remote_ = false, expired_ = false;
};
} // namespace sentrium::device
