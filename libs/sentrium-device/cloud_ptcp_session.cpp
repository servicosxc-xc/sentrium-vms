#include <sentrium/cloud_ptcp_session.hpp>
#include <utility>
namespace sentrium::device {

bool PtcpReliableSession::forwardOrEqual(
    std::uint32_t a, std::uint32_t b) noexcept {
    return (a - b) < 0x80000000u;
}
bool PtcpReliableSession::clockValid(std::uint64_t now_ms) noexcept {
    if (have_clock_ && now_ms < clock_ms_) return false;
    clock_ms_ = now_ms;
    have_clock_ = true;
    return true;
}
PtcpSendResult PtcpReliableSession::send(
    const PtcpPacket& body, std::uint64_t now_ms) {
    if (expired_) return {PtcpSessionError::Expired, {}};
    if (have_clock_ && now_ms < clock_ms_)
        return {PtcpSessionError::ClockReversed, {}};

    PtcpPacket packet = body;
    // Never trust headers supplied by a protocol consumer.
    packet.sent = sent_bytes_;
    packet.received = received_bytes_;
    packet.pid = packet.kind == PtcpKind::Sync
        ? 0x0002ffffu : 0x0000ffffu - (next_pid_ & 0xffffu);
    packet.local_message_id = next_lmid_;
    packet.remote_message_id = have_remote_ ? remote_lmid_ : 0u;
    std::string wire;
    if (encodePtcpPacket(packet, wire) != PtcpError::None)
        return {PtcpSessionError::InvalidFrame, {}};

    const std::size_t n = wire.size() - 24u;
    const bool track = packet.kind != PtcpKind::Sync &&
                       packet.kind != PtcpKind::Empty &&
                       packet.kind != PtcpKind::Heartbeat;
    if (track && (pending_.size() >= kMaxPending ||
                  n > kMaxPendingBytes - pending_bytes_))
        return {PtcpSessionError::WindowFull, {}};

    if (!clockValid(now_ms)) return {PtcpSessionError::ClockReversed, {}};
    sent_bytes_ += static_cast<std::uint32_t>(n);
    ++next_lmid_;
    if (packet.kind != PtcpKind::Sync) ++next_pid_;
    if (track) {
        pending_bytes_ += n;
        pending_.push_back({wire, sent_bytes_, now_ms, 1u});
    }
    return {PtcpSessionError::None, std::move(wire)};
}
void PtcpReliableSession::acknowledge(std::uint32_t received) noexcept {
    peer_acked_ = received;
    while (!pending_.empty() &&
           forwardOrEqual(received, pending_.front().end_sent)) {
        pending_bytes_ -= pending_.front().wire.size() - 24u;
        pending_.pop_front();
    }
}
PtcpReceiveResult PtcpReliableSession::receive(std::string_view data) {
    if (expired_) return {PtcpSessionError::Expired, {}};
    auto parsed = parsePtcpPacket(data);
    if (!parsed.ok()) return {PtcpSessionError::InvalidFrame, {}};
    const auto& packet = parsed.packet;
    // ACK is a cumulative body-byte count. Reject ahead-of-send and stale
    // ACKs with modulo-2^32 half-range ordering; never retire forged ACKs.
    if (!forwardOrEqual(packet.received, peer_acked_) ||
        !forwardOrEqual(sent_bytes_, packet.received))
        return {PtcpSessionError::InvalidAck, {}};
    if (have_remote_) {
        const std::uint32_t delta = packet.local_message_id - remote_lmid_;
        if (delta == 0u || delta >= 0x80000000u) {
            ++duplicates_;
            return {PtcpSessionError::Duplicate, {}};
        }
        if (delta != 1u) return {PtcpSessionError::SequenceGap, {}};
    }
    received_bytes_ += static_cast<std::uint32_t>(data.size() - 24u);
    remote_lmid_ = packet.local_message_id;
    have_remote_ = true;
    peer_sent_snapshot_ = packet.sent;
    acknowledge(packet.received);
    return {PtcpSessionError::None, std::move(parsed.packet)};
}
std::vector<std::string> PtcpReliableSession::dueRetransmissions(
    std::uint64_t now_ms) {
    std::vector<std::string> due;
    if (expired_ || !clockValid(now_ms)) return due;
    for (auto& item : pending_) {
        if (now_ms - item.last_sent_ms < kRetryIntervalMs) continue;
        if (item.attempts >= kMaxAttempts) {
            expired_ = true; // Caller tears down; no infinite retries.
            return {};
        }
        due.push_back(item.wire); // Exact bytes, no new PID/LMID or byte count.
        item.last_sent_ms = now_ms;
        ++item.attempts;
        ++retransmissions_;
    }
    return due;
}
PtcpSessionStats PtcpReliableSession::stats() const noexcept {
    return {sent_bytes_, received_bytes_, peer_acked_, peer_sent_snapshot_,
            next_lmid_, remote_lmid_, pending_.size(), pending_bytes_,
            retransmissions_, duplicates_};
}
} // namespace sentrium::device
