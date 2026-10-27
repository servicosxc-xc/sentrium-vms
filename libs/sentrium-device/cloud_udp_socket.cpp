#include <sentrium/cloud_udp_socket.hpp>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace sentrium::device {
namespace {

constexpr int kMaximumDatagram = 2048; // R8 expects <=2048 bytes.
constexpr int kPollMilliseconds = 20;
constexpr int kMaximumWaitMilliseconds = 1000;

bool isUnicastIpv4(std::uint32_t hostOrder) noexcept {
    const auto first = (hostOrder >> 24) & 0xffu;
    return hostOrder != 0u && hostOrder != 0xffffffffu &&
           first != 0u && first < 224u &&
           first != 255u;
}

} // namespace

struct PinnedUdpSocket::Impl {
    SOCKET sock = INVALID_SOCKET;
    bool winsock = false;
    std::mutex io;
    std::atomic_bool stopping{false};
    std::atomic<std::uint16_t> local_port{0};

    // Invoked with io locked or from the constructor before publication.
    void closeLocked() noexcept {
        stopping.store(true, std::memory_order_release);
        local_port.store(0, std::memory_order_release);
        if (sock != INVALID_SOCKET) {
            closesocket(sock);
            sock = INVALID_SOCKET;
        }
        if (winsock) {
            WSACleanup();
            winsock = false;
        }
    }
    void close() noexcept {
        stopping.store(true, std::memory_order_release);
        std::lock_guard<std::mutex> lock(io);
        closeLocked();
    }
    ~Impl() noexcept { close(); }
};

PinnedUdpSocket::PinnedUdpSocket(std::unique_ptr<Impl> impl) noexcept
    : impl_(std::move(impl)) {}
PinnedUdpSocket::~PinnedUdpSocket() noexcept { close(); }

std::unique_ptr<PinnedUdpSocket> PinnedUdpSocket::open(
    std::string_view ipv4, std::uint16_t port) {
    if (port == 0 || ipv4.empty() || ipv4.size() > 45u ||
        ipv4.find('\0') != std::string_view::npos)
        throw std::invalid_argument("UDP peer needs a valid IPv4/port");

    auto impl = std::make_unique<Impl>();
    WSADATA info{};
    if (WSAStartup(MAKEWORD(2, 2), &info) != 0)
        throw std::runtime_error("Winsock initialization failed");
    impl->winsock = true;
    sockaddr_in peer{};
    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);
    const std::string literal(ipv4);
    if (InetPtonA(AF_INET, literal.c_str(), &peer.sin_addr) != 1 ||
        !isUnicastIpv4(ntohl(peer.sin_addr.s_addr)))
        throw std::invalid_argument("Only unicast IPv4 literals accepted");

    impl->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (impl->sock == INVALID_SOCKET)
        throw std::runtime_error("UDP socket creation failed");

    const BOOL exclusive = TRUE;
    if (setsockopt(impl->sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
                   reinterpret_cast<const char*>(&exclusive),
                   static_cast<int>(sizeof(exclusive))) == SOCKET_ERROR)
        throw std::runtime_error("UDP exclusive address policy failed");

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(0);
    if (bind(impl->sock, reinterpret_cast<sockaddr*>(&local),
             static_cast<int>(sizeof(local))) == SOCKET_ERROR)
        throw std::runtime_error("UDP local bind failed");
    // A connected UDP socket drops packets from other source endpoints.
    // This is endpoint pinning, NOT cryptographic authentication.
    if (::connect(impl->sock, reinterpret_cast<const sockaddr*>(&peer),
                  static_cast<int>(sizeof(peer))) == SOCKET_ERROR)
        throw std::runtime_error("UDP peer connect failed");

    sockaddr_in actual{};
    int addressLength = static_cast<int>(sizeof(actual));
    if (getsockname(impl->sock, reinterpret_cast<sockaddr*>(&actual),
                    &addressLength) == SOCKET_ERROR ||
        ntohs(actual.sin_port) == 0)
        throw std::runtime_error("UDP local port unavailable");
    impl->local_port.store(ntohs(actual.sin_port),
                           std::memory_order_release);
    return std::unique_ptr<PinnedUdpSocket>(
        new PinnedUdpSocket(std::move(impl)));
}

bool PinnedUdpSocket::sendDatagram(std::string_view bytes) noexcept {
    if (!impl_ || impl_->stopping.load(std::memory_order_acquire) ||
        bytes.empty() || bytes.size() > static_cast<std::size_t>(kMaximumDatagram))
        return false;
    std::lock_guard<std::mutex> lock(impl_->io);
    if (impl_->stopping.load(std::memory_order_acquire))
        return false;
    const int sent = ::send(impl_->sock, bytes.data(),
                             static_cast<int>(bytes.size()), 0);
    if (sent != static_cast<int>(bytes.size())) {
        impl_->closeLocked();
        return false;
    }
    return true;
}

int PinnedUdpSocket::receiveDatagram(
    char* buffer, int capacity, int timeout_ms) noexcept {
    if (!impl_ || !buffer || capacity <= 0 ||
        capacity > kMaximumDatagram ||
        timeout_ms < 0 || timeout_ms > kMaximumWaitMilliseconds)
        return -1;
    if (impl_->stopping.load(std::memory_order_acquire)) return -1;

    using Clock = std::chrono::steady_clock;
    const auto deadline = Clock::now() + std::chrono::milliseconds(timeout_ms);
    for (;;) {
        if (impl_->stopping.load(std::memory_order_acquire)) return -1;
        const auto remaining = std::chrono::duration_cast<
            std::chrono::milliseconds>(deadline - Clock::now()).count();
        const int wait = static_cast<int>(std::max<std::int64_t>(
            0, std::min<std::int64_t>(kPollMilliseconds, remaining)));
        {
            std::lock_guard<std::mutex> lock(impl_->io);
            if (impl_->stopping.load(std::memory_order_acquire))
                return -1;
            fd_set readable;
            FD_ZERO(&readable);
            FD_SET(impl_->sock, &readable);
            timeval tv{wait / 1000, (wait % 1000) * 1000};
            const int ready = select(0, &readable, nullptr, nullptr, &tv);
            if (ready == SOCKET_ERROR) {
                impl_->closeLocked();
                return -1;
            }
            if (ready > 0) {
                const int count = recv(impl_->sock, buffer, capacity, 0);
                if (count < 0) {
                    // WSAEMSGSIZE truncates packets. Never deliver partial
                    // PTCP, since that would corrupt the ordered byte stream.
                    impl_->closeLocked();
                    return -1;
                }
                if (count > 0) return count;
                // Zero-length UDP packet: distinguish from timeout by
                // treating it as a malformed, fail-closed datagram.
                impl_->closeLocked();
                return -1;
            }
        }
        if (Clock::now() >= deadline || timeout_ms == 0) return 0;
    }
}

std::uint16_t PinnedUdpSocket::localPort() const noexcept {
    return impl_ ? impl_->local_port.load(std::memory_order_acquire) : 0;
}
bool PinnedUdpSocket::closed() const noexcept {
    return !impl_ || impl_->stopping.load(std::memory_order_acquire);
}
void PinnedUdpSocket::close() noexcept {
    if (impl_) impl_->close();
}

} // namespace sentrium::device
