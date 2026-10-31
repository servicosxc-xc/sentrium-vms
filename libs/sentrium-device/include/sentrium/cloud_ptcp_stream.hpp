#pragma once

#include <sentrium/cloud_loopback_bridge.hpp>
#include <sentrium/cloud_ptcp_channel.hpp>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <string_view>

namespace sentrium::device {

// Injection point for a VERIFIED and AUTHORIZED provider-owned datagram
// channel. This interface does not establish or prove authorization.
// receive: >0 whole datagram, 0 deadline, -1 closed/error. NEVER truncate.
// transmit: true only when the whole datagram was accepted.
// close must unblock concurrent receive/transmit and may be called from
// CloudLoopbackBridge::close() on a different thread.
class VerifiedPtcpDatagramLink {
public:
    virtual ~VerifiedPtcpDatagramLink() = default;
    virtual int receive(char* data, int capacity, int timeout_ms) noexcept = 0;
    virtual bool transmit(std::string_view datagram) noexcept = 0;
    virtual void close() noexcept = 0;
};

// Converts an already-authenticated datagram link into an ordered byte stream
// suitable for CloudLoopbackBridge. No Cloud bootstrap, UDP socket creation,
// STUN/NAT, provider identity, Direct/Relay selection or SDK login is here.
// read()/write() must run on ONE worker thread, close() may be concurrent.
class PtcpVerifiedByteStream final : public CloudByteStream {
public:
    static std::unique_ptr<PtcpVerifiedByteStream> create(
        std::unique_ptr<VerifiedPtcpDatagramLink> verified_link,
        std::uint32_t authenticated_realm,
        std::uint32_t authenticated_first_peer_lmid);

    ~PtcpVerifiedByteStream() noexcept override;
    PtcpVerifiedByteStream(const PtcpVerifiedByteStream&) = delete;
    PtcpVerifiedByteStream& operator=(const PtcpVerifiedByteStream&) = delete;

    int read(char* buffer, int capacity, int timeout_ms) noexcept override;
    bool write(std::string_view bytes) noexcept override;
    void close() noexcept override;

private:
    PtcpVerifiedByteStream(
        std::unique_ptr<VerifiedPtcpDatagramLink> link,
        std::uint32_t realm, std::uint32_t first_peer_lmid) noexcept;

    static constexpr std::size_t kMaxIncomingBytes = 131072;
    static constexpr std::size_t kMaxLocalWrite = 16384;
    static constexpr int kMaxDatagram = 2048;
    static constexpr int kMaxReadDeadlineMs = 1000;
    static constexpr int kBackpressureIterations = 50;

    static std::uint64_t monotonicMilliseconds() noexcept;
    bool progress(int timeout_ms) noexcept;
    bool retryDue() noexcept;
    int drain(char* output, int capacity) noexcept;
    bool fail() noexcept;

    std::unique_ptr<VerifiedPtcpDatagramLink> link_;
    PtcpPostAuthChannel channel_;
    std::atomic_bool closed_{false};
    std::deque<std::string> incoming_;
    std::size_t buffered_bytes_ = 0;
    std::size_t front_offset_ = 0;
};

} // namespace sentrium::device
