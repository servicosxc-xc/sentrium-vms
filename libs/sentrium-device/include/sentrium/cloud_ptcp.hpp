#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace sentrium::device {

// Portable, bounded framing for DH/PTCP over UDP. This is a wire primitive,
// not an authenticated rendezvous, NAT traversal or live Cloud provider.
enum class PtcpKind {
    Empty, Sync, Payload, Bind, Status, Heartbeat, Command
};

enum class PtcpError {
    None, TooShort, TooLarge, WrongMagic, InvalidType,
    InvalidLength, InvalidPadding, InvalidStatus
};

struct PtcpPacket {
    std::uint32_t sent = 0;
    std::uint32_t received = 0;
    std::uint32_t pid = 0;
    std::uint32_t local_message_id = 0;
    std::uint32_t remote_message_id = 0;
    PtcpKind kind = PtcpKind::Empty;
    std::uint32_t realm = 0;
    std::uint32_t bind_port = 0;
    std::string body; // Payload or opaque 0x17..0x1B command: sensitive
    std::string status; // Only CONN or DISC; not "Direct" vs "Relay"
};

struct PtcpResult {
    PtcpError error = PtcpError::TooShort;
    PtcpPacket packet;
    [[nodiscard]] bool ok() const noexcept { return error == PtcpError::None; }
};

// Strictly parse one PTCP UDP datagram (24-byte big-endian header).
// The parser never logs raw packets, Cloud IDs or device credentials.
[[nodiscard]] PtcpResult parsePtcpPacket(std::string_view data);

// Encode only known packet types; rejects inconsistent fields. Caller controls
// unique PID/LMID and ordered writes at the session layer (not implemented).
[[nodiscard]] PtcpError encodePtcpPacket(
    const PtcpPacket& packet, std::string& destination);

} // namespace sentrium::device
