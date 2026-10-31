#include <sentrium/cloud_ptcp_receive_window.hpp>
#include <utility>

namespace sentrium::device {

PtcpReceiveWindow::PtcpReceiveWindow(std::uint32_t expected) noexcept
    : expected_(expected) {}

std::size_t PtcpReceiveWindow::storedSize(const PtcpPacket& packet) noexcept {
    return 24u + packet.body.size() + packet.status.size();
}

bool PtcpReceiveWindow::sameWirePacket(
    const PtcpPacket& a, const PtcpPacket& b) noexcept {
    return a.sent == b.sent && a.received == b.received &&
        a.pid == b.pid &&
        a.local_message_id == b.local_message_id &&
        a.remote_message_id == b.remote_message_id &&
        a.kind == b.kind && a.realm == b.realm &&
        a.bind_port == b.bind_port && a.body == b.body &&
        a.status == b.status;
}

PtcpOrderedBatch PtcpReceiveWindow::ingest(const PtcpPacket& packet) {
    if (closed_) return {PtcpOrderError::Closed, {}};

    // This bound applies even if a caller bypassed the PTCP wire parser.
    // Only a verified parser/peer can feed this queue in production.
    const auto bytes = storedSize(packet);
    if (bytes > kMaximumPacketBytes) {
        closed_ = true;
        pending_.clear();
        queued_bytes_ = 0;
        return {PtcpOrderError::InvalidPacket, {}};
    }

    // Unsigned modulo distance handles 32-bit LMID wrap. A backwards
    // difference in the upper half range is stale; do not deliver twice.
    const std::uint32_t distance =
        packet.local_message_id - expected_;
    if (distance >= 0x80000000u)
        return {PtcpOrderError::Duplicate, {}};

    if (distance > kMaximumGap) {
        close();
        return {PtcpOrderError::GapTooLarge, {}};
    }

    const auto duplicate = pending_.find(packet.local_message_id);
    if (duplicate != pending_.end()) {
        if (sameWirePacket(duplicate->second, packet))
            return {PtcpOrderError::Duplicate, {}};
        close(); // Conflicting LMID/PID/bytes is a protocol violation.
        return {PtcpOrderError::ConflictingDuplicate, {}};
    }

    if (distance != 0) {
        if (pending_.size() >= kMaximumQueuedPackets ||
            bytes > kMaximumQueuedBytes - queued_bytes_) {
            close();
            return {PtcpOrderError::WindowFull, {}};
        }
        pending_.emplace(packet.local_message_id, packet);
        queued_bytes_ += bytes;
        return {PtcpOrderError::None, {}};
    }

    PtcpOrderedBatch output{PtcpOrderError::None, {}};
    output.ready.push_back(packet);
    ++expected_;
    // Ordered extraction across wraparound uses exact LMID key lookup,
    // not std::map iteration order or signed 32-bit subtraction.
    while (true) {
        auto next = pending_.find(expected_);
        if (next == pending_.end()) break;
        queued_bytes_ -= storedSize(next->second);
        output.ready.push_back(std::move(next->second));
        pending_.erase(next);
        ++expected_;
    }
    return output;
}

void PtcpReceiveWindow::close() noexcept {
    closed_ = true;
    pending_.clear();
    queued_bytes_ = 0;
}

std::uint32_t PtcpReceiveWindow::expectedMessageId() const noexcept {
    return expected_;
}
std::size_t PtcpReceiveWindow::pendingPackets() const noexcept {
    return pending_.size();
}
std::size_t PtcpReceiveWindow::pendingBytes() const noexcept {
    return queued_bytes_;
}
bool PtcpReceiveWindow::closed() const noexcept {
    return closed_;
}

} // namespace sentrium::device
