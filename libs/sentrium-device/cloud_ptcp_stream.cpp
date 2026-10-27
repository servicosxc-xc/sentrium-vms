#include <sentrium/cloud_ptcp_stream.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace sentrium::device {

PtcpVerifiedByteStream::PtcpVerifiedByteStream(
    std::unique_ptr<VerifiedPtcpDatagramLink> link,
    std::uint32_t realm, std::uint32_t first_peer_lmid) noexcept
    : link_(std::move(link)), channel_(realm, first_peer_lmid) {}

std::unique_ptr<PtcpVerifiedByteStream> PtcpVerifiedByteStream::create(
    std::unique_ptr<VerifiedPtcpDatagramLink> link,
    std::uint32_t realm, std::uint32_t first_peer_lmid) {
    if (!link) {
        throw std::invalid_argument("Verified PTCP datagram link required");
    }
    return std::unique_ptr<PtcpVerifiedByteStream>(
        new PtcpVerifiedByteStream(std::move(link), realm, first_peer_lmid));
}

PtcpVerifiedByteStream::~PtcpVerifiedByteStream() noexcept { close(); }

std::uint64_t PtcpVerifiedByteStream::monotonicMilliseconds() noexcept {
    using namespace std::chrono;
    return static_cast<std::uint64_t>(duration_cast<milliseconds>(
        steady_clock::now().time_since_epoch()).count());
}

void PtcpVerifiedByteStream::close() noexcept {
    if (!closed_.exchange(true, std::memory_order_acq_rel)) {
        // The provider MUST interrupt an outstanding read/write promptly.
        // Do not mutate channel_ from here: close may run on another thread.
        link_->close();
    }
}

bool PtcpVerifiedByteStream::fail() noexcept {
    close();
    channel_.close(); // Called only on the owning bridge worker thread.
    incoming_.clear();
    buffered_bytes_ = 0;
    front_offset_ = 0;
    return false;
}

int PtcpVerifiedByteStream::drain(char* out, int capacity) noexcept {
    if (incoming_.empty()) return 0;
    int copied = 0;
    while (copied < capacity && !incoming_.empty()) {
        const std::string& front = incoming_.front();
        const std::size_t left = front.size() - front_offset_;
        const std::size_t count = std::min(
            left, static_cast<std::size_t>(capacity - copied));
        std::memcpy(out + copied, front.data() + front_offset_, count);
        copied += static_cast<int>(count);
        buffered_bytes_ -= count;
        front_offset_ += count;
        if (front_offset_ == front.size()) {
            incoming_.pop_front();
            front_offset_ = 0;
        }
    }
    return copied;
}

bool PtcpVerifiedByteStream::retryDue() noexcept {
    if (closed_.load(std::memory_order_acquire)) return false;
    const auto due = channel_.retransmissions(monotonicMilliseconds());
    if (!due.ok()) return fail();
    for (const auto& datagram : due.datagrams) {
        if (closed_.load(std::memory_order_acquire) ||
            !link_->transmit(datagram)) return fail();
    }
    return true;
}

bool PtcpVerifiedByteStream::progress(int timeout_ms) noexcept {
    if (!retryDue()) return false;
    std::array<char, kMaxDatagram> raw{};
    const int received = link_->receive(
        raw.data(), static_cast<int>(raw.size()), timeout_ms);
    if (closed_.load(std::memory_order_acquire)) return false;
    if (received < 0 || received > static_cast<int>(raw.size()))
        return fail();

    if (received > 0) {
        // VerifiedPtcpDatagramLink MUST pin the authenticated remote peer.
        const auto accepted = channel_.receiveVerified(
            std::string_view(raw.data(), static_cast<std::size_t>(received)));
        if (!accepted.ok()) return fail();
        std::size_t additional = 0;
        for (const auto& bytes : accepted.ordered_bytes) {
            if (bytes.size() > kMaxIncomingBytes - buffered_bytes_ - additional)
                return fail();
            additional += bytes.size();
        }
        for (const auto& bytes : accepted.ordered_bytes) {
            if (!bytes.empty()) incoming_.push_back(bytes);
        }
        buffered_bytes_ += additional;
    }
    return retryDue();
}

int PtcpVerifiedByteStream::read(
    char* buffer, int capacity, int timeout_ms) noexcept {
    if (closed_.load(std::memory_order_acquire)) return -1;
    if (!buffer || capacity <= 0 ||
        timeout_ms < 0 || timeout_ms > kMaxReadDeadlineMs)
        return -1;
    if (!retryDue()) return -1;
    const int available = drain(buffer, capacity);
    if (available > 0) return available;
    if (!progress(timeout_ms)) return -1;
    return drain(buffer, capacity);
}

bool PtcpVerifiedByteStream::write(std::string_view bytes) noexcept {
    if (closed_.load(std::memory_order_acquire)) return false;
    if (bytes.empty() || bytes.size() > kMaxLocalWrite) return false;

    auto accepted = channel_.sendBytes(bytes, monotonicMilliseconds());
    // Bounded attempts to receive outstanding ACKs. Do not silently drop
    // an entire NetSDK TCP write on transient PTCP congestion.
    for (int attempt = 0; attempt < kBackpressureIterations &&
         accepted.error == PtcpChannelError::Backpressure; ++attempt) {
        if (!progress(20)) return false;
        accepted = channel_.sendBytes(bytes, monotonicMilliseconds());
    }
    if (!accepted.ok()) return fail();
    for (const auto& frame : accepted.datagrams) {
        // If any datagram fails, the exact all-or-nothing write contract of
        // CloudByteStream cannot be met; tear the channel down, never replay.
        if (closed_.load(std::memory_order_acquire) ||
            !link_->transmit(frame)) return fail();
    }
    return true;
}

} // namespace sentrium::device
