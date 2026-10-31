#include <sentrium/cloud_ptcp_channel.hpp>

#include <algorithm>
#include <utility>

namespace sentrium::device {

PtcpPostAuthChannel::PtcpPostAuthChannel(
    std::uint32_t realm, std::uint32_t expected_peer_lmid) noexcept
    : receiver_(expected_peer_lmid), realm_(realm) {}

PtcpChannelSend PtcpPostAuthChannel::sendBytes(
    std::string_view data, std::uint64_t now_ms) {
    if (closed_) return {PtcpChannelError::Closed, {}};
    if (data.empty() || data.size() > kMaxWriteBytes)
        return {PtcpChannelError::InvalidInput, {}};

    const std::size_t count =
        (data.size() + kPayloadBytes - 1u) / kPayloadBytes;
    const auto before = sender_.stats();

    // Each payload requires a 12-byte PTCP body prefix. Check the entire
    // write before changing sender counters. No partial commit on saturation.
    if (count > kPendingPackets - before.in_flight_packets ||
        data.size() + count * 12u >
            kPendingBodyBytes - before.in_flight_bytes)
        return {PtcpChannelError::Backpressure, {}};

    PtcpChannelSend result{PtcpChannelError::None, {}};
    result.datagrams.reserve(count);
    for (std::size_t offset = 0; offset < data.size();) {
        const std::size_t n = std::min(kPayloadBytes, data.size() - offset);
        PtcpPacket frame{};
        frame.kind = PtcpKind::Payload;
        frame.realm = realm_;
        frame.body.assign(data.substr(offset, n));
        auto created = sender_.send(frame, now_ms);
        if (!created.ok()) {
            // A failed send after pre-admission must never be followed by
            // automatic retransmission as if the whole write succeeded.
            close();
            if (created.error == PtcpSessionError::Expired)
                return {PtcpChannelError::Expired, {}};
            return {PtcpChannelError::AccountingFailed, {}};
        }
        result.datagrams.push_back(std::move(created.datagram));
        offset += n;
    }
    return result;
}

PtcpChannelSend PtcpPostAuthChannel::heartbeat(std::uint64_t now_ms) {
    if (closed_) return {PtcpChannelError::Closed, {}};
    PtcpPacket frame{};
    frame.kind = PtcpKind::Heartbeat;
    auto output = sender_.send(frame, now_ms);
    if (!output.ok()) {
        if (output.error == PtcpSessionError::ClockReversed)
            return {PtcpChannelError::InvalidInput, {}};
        close();
        return {PtcpChannelError::AccountingFailed, {}};
    }
    PtcpChannelSend result{PtcpChannelError::None, {}};
    result.datagrams.push_back(std::move(output.datagram));
    return result;
}

PtcpChannelReceive PtcpPostAuthChannel::receiveVerified(
    std::string_view datagram) {
    if (closed_) return {PtcpChannelError::Closed, {}};
    auto parsed = parsePtcpPacket(datagram);
    if (!parsed.ok()) {
        close();
        return {PtcpChannelError::InvalidDatagram, {}};
    }

    const auto& packet = parsed.packet;
    // Reject unexpected post-auth handshake, SYNC, BIND or data realms.
    // Realm is only meaningful on payload/status; heartbeat/empty have 0.
    if ((packet.kind == PtcpKind::Payload ||
         packet.kind == PtcpKind::Status) &&
        packet.realm != realm_) {
        close();
        return {PtcpChannelError::WrongRealm, {}};
    }
    if (packet.kind != PtcpKind::Payload &&
        packet.kind != PtcpKind::Status &&
        packet.kind != PtcpKind::Heartbeat &&
        packet.kind != PtcpKind::Empty) {
        close();
        return {PtcpChannelError::InvalidDatagram, {}};
    }

    auto ordered = receiver_.ingest(packet);
    if (ordered.error == PtcpOrderError::Duplicate)
        return {PtcpChannelError::Duplicate, {}};
    if (!ordered.accepted()) {
        close();
        return {PtcpChannelError::OrderingFailed, {}};
    }
    PtcpChannelReceive result{PtcpChannelError::None, {}};
    for (const auto& item : ordered.ready) {
        // The receive window yields packets, not original raw datagrams.
        // Encode canonically and let the session verify cumulative ACKs and
        // account byte counts AFTER contiguous LMID ordering.
        std::string canonical;
        if (encodePtcpPacket(item, canonical) != PtcpError::None ||
            !sender_.receive(canonical).ok()) {
            close();
            return {PtcpChannelError::AccountingFailed, {}};
        }
        if (item.kind == PtcpKind::Payload) {
            result.ordered_bytes.push_back(item.body);
        } else if (item.kind == PtcpKind::Status &&
                   item.status == "DISC") {
            close();
            return {PtcpChannelError::RemoteClosed, {}};
        }
    }
    return result;
}

PtcpChannelSend PtcpPostAuthChannel::retransmissions(
    std::uint64_t now_ms) {
    if (closed_) return {PtcpChannelError::Closed, {}};
    auto due = sender_.dueRetransmissions(now_ms);
    if (sender_.expired()) {
        close();
        return {PtcpChannelError::Expired, {}};
    }
    return {PtcpChannelError::None, std::move(due)};
}

PtcpChannelStats PtcpPostAuthChannel::stats() const noexcept {
    return {sender_.stats(), receiver_.pendingPackets(),
            receiver_.pendingBytes()};
}

void PtcpPostAuthChannel::close() noexcept {
    closed_ = true;
    receiver_.close();
}

} // namespace sentrium::device
