#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

namespace sentrium::device {

// Low-level Windows UDP I/O to a caller-selected IPv4 endpoint.
// A UDP "connect" only filters remote IP/port at the kernel: it DOES NOT
// authenticate a peer or authorize use of a proprietary Cloud service.
// This class intentionally does NOT implement VerifiedPtcpDatagramLink.
// An authorized provider must authenticate bootstrap/session and verify
// datagrams independently BEFORE passing them to the R8 byte stream.
class PinnedUdpSocket final {
public:
    // No hostname resolution or hard-coded endpoints. Caller supplies an
    // explicitly authorized IPv4 unicast literal and service port.
    static std::unique_ptr<PinnedUdpSocket> open(
        std::string_view ipv4, std::uint16_t port);

    ~PinnedUdpSocket() noexcept;
    PinnedUdpSocket(const PinnedUdpSocket&) = delete;
    PinnedUdpSocket& operator=(const PinnedUdpSocket&) = delete;

    // A successful send transmits one whole datagram; false means error.
    [[nodiscard]] bool sendDatagram(std::string_view data) noexcept;

    // 1..capacity = whole datagram; 0 = timeout/no datagram; -1 = error or
    // closed. Reject truncated datagrams, empty UDP payloads and invalid
    // arguments. timeout_ms is bounded to 0..1000.
    [[nodiscard]] int receiveDatagram(
        char* buffer, int capacity, int timeout_ms) noexcept;

    // Returns OS-assigned local source port while active, else zero.
    [[nodiscard]] std::uint16_t localPort() const noexcept;
    [[nodiscard]] bool closed() const noexcept;
    // May run concurrently with receive/send. Poll interval <=20 ms.
    void close() noexcept;

private:
    struct Impl;
    explicit PinnedUdpSocket(std::unique_ptr<Impl> impl) noexcept;
    std::unique_ptr<Impl> impl_;
};

} // namespace sentrium::device
