#pragma once
#include <sentrium/cloud_ptcp.hpp>
#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

namespace sentrium::device {

// Packet-ordering primitive for datagrams that have ALREADY been verified,
// authenticated and peer-pinned by a future authorized Cloud provider.
// No protocol bootstrap, stream socket, peer authentication or login occurs.
enum class PtcpOrderError {
    None,
    Duplicate,
    InvalidPacket,
    GapTooLarge,
    WindowFull,
    ConflictingDuplicate,
    Closed
};

struct PtcpOrderedBatch {
    PtcpOrderError error = PtcpOrderError::InvalidPacket;
    std::vector<PtcpPacket> ready;  // Contiguous packets; payloads stay private.
    [[nodiscard]] bool accepted() const noexcept {
        return error == PtcpOrderError::None;
    }
};

// LMIDs are across all PTCP packet kinds, including heartbeat and controls:
// do not assume consecutive PAYLOAD IDs. Caller must pin expected LMID to
// an authenticated session. One worker thread only; no internal sockets.
class PtcpReceiveWindow final {
public:
    explicit PtcpReceiveWindow(std::uint32_t expected_local_message_id) noexcept;
    PtcpReceiveWindow(const PtcpReceiveWindow&) = delete;
    PtcpReceiveWindow& operator=(const PtcpReceiveWindow&) = delete;

    [[nodiscard]] PtcpOrderedBatch ingest(const PtcpPacket& packet);
    void close() noexcept;

    [[nodiscard]] std::uint32_t expectedMessageId() const noexcept;
    [[nodiscard]] std::size_t pendingPackets() const noexcept;
    [[nodiscard]] std::size_t pendingBytes() const noexcept;
    [[nodiscard]] bool closed() const noexcept;

private:
    static constexpr std::uint32_t kMaximumGap = 32;
    static constexpr std::size_t kMaximumQueuedPackets = 32;
    static constexpr std::size_t kMaximumQueuedBytes = 262144;
    static constexpr std::size_t kMaximumPacketBytes = 65507;

    static std::size_t storedSize(const PtcpPacket& packet) noexcept;
    static bool sameWirePacket(const PtcpPacket& a, const PtcpPacket& b) noexcept;

    std::map<std::uint32_t, PtcpPacket> pending_;
    std::uint32_t expected_;
    std::size_t queued_bytes_ = 0;
    bool closed_ = false;
};

} // namespace sentrium::device
