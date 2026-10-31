#include <sentrium/cloud_ptcp.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace sentrium::device {
namespace {

constexpr std::size_t kHeaderSize = 24;
constexpr std::size_t kMaxDatagram = 65507;
constexpr std::size_t kMaxPayload = 1280;

std::uint32_t read32(std::string_view data, std::size_t offset) noexcept {
    return (std::uint32_t(static_cast<unsigned char>(data[offset])) << 24) |
           (std::uint32_t(static_cast<unsigned char>(data[offset+1])) << 16) |
           (std::uint32_t(static_cast<unsigned char>(data[offset+2])) << 8) |
            std::uint32_t(static_cast<unsigned char>(data[offset+3]));
}
void add32(std::string& data, std::uint32_t value) {
    data.push_back(static_cast<char>((value >> 24) & 0xff));
    data.push_back(static_cast<char>((value >> 16) & 0xff));
    data.push_back(static_cast<char>((value >> 8) & 0xff));
    data.push_back(static_cast<char>(value & 0xff));
}
PtcpResult failure(PtcpError reason) { return {reason,{}}; }

} // namespace

PtcpResult parsePtcpPacket(std::string_view data) {
    if (data.size() < kHeaderSize) return failure(PtcpError::TooShort);
    if (data.size() > kMaxDatagram) return failure(PtcpError::TooLarge);
    if (data.substr(0,4) != "PTCP") return failure(PtcpError::WrongMagic);

    PtcpPacket packet;
    packet.sent = read32(data, 4);
    packet.received = read32(data, 8);
    packet.pid = read32(data, 12);
    packet.local_message_id = read32(data, 16);
    packet.remote_message_id = read32(data, 20);
    data.remove_prefix(kHeaderSize);
    if (data.empty()) {
        packet.kind = PtcpKind::Empty;
        return {PtcpError::None, std::move(packet)};
    }
    // SYNC is a four-byte special case, not the normal 12-byte body.
    if (data == std::string_view("\x00\x03\x01\x00",4)) {
        packet.kind = PtcpKind::Sync;
        return {PtcpError::None, std::move(packet)};
    }
    if (data.size() < 12) return failure(PtcpError::TooShort);
    const auto type = static_cast<unsigned char>(data[0]);
    const std::uint32_t length = read32(data, 0) & 0x00ffffff;
    const std::uint32_t realm = read32(data, 4);
    const std::uint32_t padding = read32(data, 8);
    packet.realm = realm;

    switch (type) {
    case 0x10:
        if (padding != 0) return failure(PtcpError::InvalidPadding);
        if (data.size()-12 != length) return failure(PtcpError::InvalidLength);
        packet.kind = PtcpKind::Payload;
        packet.body.assign(data.substr(12));
        return {PtcpError::None, std::move(packet)};
    case 0x11:
        if (padding != 0) return failure(PtcpError::InvalidPadding);
        if (length != 8 || data.size() != 20) return failure(PtcpError::InvalidLength);
        packet.kind = PtcpKind::Bind;
        packet.bind_port = read32(data,12);
        if (packet.bind_port == 0 || packet.bind_port > 65535)
            return failure(PtcpError::InvalidLength);
        return {PtcpError::None, std::move(packet)};
    case 0x12:
        if (padding != 0) return failure(PtcpError::InvalidPadding);
        if (length != data.size()-12) return failure(PtcpError::InvalidLength);
        packet.kind = PtcpKind::Status;
        packet.status.assign(data.substr(12));
        if (packet.status != "CONN" && packet.status != "DISC")
            return failure(PtcpError::InvalidStatus);
        return {PtcpError::None, std::move(packet)};
    case 0x13:
        if (padding != 0) return failure(PtcpError::InvalidPadding);
        if (length != 0 || data.size() != 12) return failure(PtcpError::InvalidLength);
        packet.kind = PtcpKind::Heartbeat;
        return {PtcpError::None, std::move(packet)};
    default:
        // Opaque handshake commands are fixed minimum of 12 bytes and
        // validated by the later authenticated-session layer.
        if (type >= 0x17 && type <= 0x1b) {
            packet.kind = PtcpKind::Command;
            packet.body.assign(data);
            return {PtcpError::None, std::move(packet)};
        }
        return failure(PtcpError::InvalidType);
    }
}

PtcpError encodePtcpPacket(const PtcpPacket& packet, std::string& destination) {
    std::string body;
    switch (packet.kind) {
    case PtcpKind::Empty:
        break;
    case PtcpKind::Sync:
        body.assign("\x00\x03\x01\x00",4);
        break;
    case PtcpKind::Payload:
        if (packet.body.size() > kMaxPayload) return PtcpError::TooLarge;
        add32(body, 0x10000000u | static_cast<std::uint32_t>(packet.body.size()));
        add32(body, packet.realm); add32(body, 0);
        body += packet.body;
        break;
    case PtcpKind::Bind:
        if (packet.bind_port == 0 || packet.bind_port > 65535)
            return PtcpError::InvalidLength;
        add32(body,0x11000008u); add32(body,packet.realm); add32(body,0);
        add32(body,packet.bind_port);
        body.append("\x7f\x00\x00\x01",4);
        break;
    case PtcpKind::Status:
        if (packet.status != "CONN" && packet.status != "DISC")
            return PtcpError::InvalidStatus;
        add32(body,0x12000000u | static_cast<std::uint32_t>(packet.status.size()));
        add32(body,packet.realm); add32(body,0); body += packet.status;
        break;
    case PtcpKind::Heartbeat:
        add32(body,0x13000000u); add32(body,0); add32(body,0);
        break;
    case PtcpKind::Command:
        if (packet.body.size() < 12 || packet.body.size() > kMaxPayload ||
            static_cast<unsigned char>(packet.body.front()) < 0x17 ||
            static_cast<unsigned char>(packet.body.front()) > 0x1b)
            return PtcpError::InvalidType;
        body = packet.body;
        break;
    }
    if (body.size()+kHeaderSize > kMaxDatagram) return PtcpError::TooLarge;
    std::string encoded = "PTCP";
    add32(encoded,packet.sent); add32(encoded,packet.received);
    add32(encoded,packet.pid); add32(encoded,packet.local_message_id);
    add32(encoded,packet.remote_message_id);
    encoded += body;
    destination = std::move(encoded);
    return PtcpError::None;
}

} // namespace sentrium::device
