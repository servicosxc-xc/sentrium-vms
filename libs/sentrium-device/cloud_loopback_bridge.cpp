#include <sentrium/cloud_loopback_bridge.hpp>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <array>
#include <atomic>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <utility>

namespace sentrium::device {
namespace {
constexpr int kChunk = 8192;
constexpr int kPollUs = 20000;
constexpr int kUpstreamReadMs = 20;
constexpr DWORD kSendTimeoutMs = 500;
}

struct CloudLoopbackBridge::Impl {
    explicit Impl(std::unique_ptr<CloudByteStream> source) noexcept
        : stream(std::move(source)) {}
    ~Impl() noexcept { close(); }
    std::unique_ptr<CloudByteStream> stream;
    SOCKET listener = INVALID_SOCKET;
    std::thread worker;
    std::atomic_bool stopped{false};
    std::uint16_t port = 0;
    bool winsock = false;

    void close() noexcept {
        // Idempotent teardown: do not close an upstream stream twice when
        // caller close() is followed by the bridge/Impl destructor.
        if (stopped.exchange(true, std::memory_order_acq_rel)) return;
        if (stream) stream->close();
        if (worker.joinable()) worker.join();
        // Worker alone reads/writes listener; join synchronizes teardown.
        if (listener != INVALID_SOCKET) {
            closesocket(listener);
            listener = INVALID_SOCKET;
        }
        port = 0;
        if (winsock) { WSACleanup(); winsock = false; }
    }

    void run() noexcept {
        SOCKET client = INVALID_SOCKET;
        while (!stopped.load(std::memory_order_acquire)) {
            fd_set reads;
            FD_ZERO(&reads);
            FD_SET(listener, &reads);
            timeval deadline{0, kPollUs};
            const int result = select(0, &reads, nullptr, nullptr, &deadline);
            if (result == SOCKET_ERROR) break;
            if (result == 0) continue;
            client = accept(listener, nullptr, nullptr);
            break;
        }
        // Close after first accept, never pass a second client to the remote.
        if (listener != INVALID_SOCKET) {
            closesocket(listener);
            listener = INVALID_SOCKET;
        }
        if (client == INVALID_SOCKET || stopped.load(std::memory_order_acquire)) {
            if (client != INVALID_SOCKET) closesocket(client);
            return;
        }
        if (setsockopt(client, SOL_SOCKET, SO_SNDTIMEO,
                       reinterpret_cast<const char*>(&kSendTimeoutMs),
                       static_cast<int>(sizeof(kSendTimeoutMs))) == SOCKET_ERROR) {
            closesocket(client);
            return;
        }
        std::array<char, kChunk> uplink{};
        std::array<char, kChunk> downlink{};
        while (!stopped.load(std::memory_order_acquire)) {
            fd_set reads;
            FD_ZERO(&reads);
            FD_SET(client, &reads);
            timeval deadline{0, kPollUs};
            const int ready = select(0, &reads, nullptr, nullptr, &deadline);
            if (ready == SOCKET_ERROR) break;
            if (ready > 0) {
                const int count = recv(client, uplink.data(),
                                       static_cast<int>(uplink.size()), 0);
                if (count <= 0 ||
                    !stream->write(std::string_view(
                        uplink.data(), static_cast<std::size_t>(count)))) break;
            }
            // Caller/provider MUST honor timeout and wake on close.
            const int count = stream->read(
                downlink.data(), static_cast<int>(downlink.size()), kUpstreamReadMs);
            if (count < 0 || count > static_cast<int>(downlink.size())) break;
            int offset = 0;
            while (offset < count && !stopped.load(std::memory_order_acquire)) {
                const int sent = send(client, downlink.data() + offset,
                                      count - offset, 0);
                if (sent <= 0) { offset = -1; break; }
                offset += sent;
            }
            if (offset < 0) break;
        }
        ::shutdown(client, SD_BOTH);
        closesocket(client);
    }
};

CloudLoopbackBridge::CloudLoopbackBridge(std::unique_ptr<Impl> impl) noexcept
    : impl_(std::move(impl)) {}
CloudLoopbackBridge::~CloudLoopbackBridge() noexcept { close(); }

std::unique_ptr<CloudLoopbackBridge> CloudLoopbackBridge::start(
    std::unique_ptr<CloudByteStream> authenticated_stream) {
    if (!authenticated_stream)
        throw std::invalid_argument("Cloud loopback needs a verified stream");
    auto state = std::make_unique<Impl>(std::move(authenticated_stream));
    WSADATA data{};
    if (WSAStartup(MAKEWORD(2,2), &data) != 0)
        throw std::runtime_error("Cloud loopback network init failed");
    state->winsock = true;
    state->listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (state->listener == INVALID_SOCKET)
        throw std::runtime_error("Cloud loopback socket failed");
    const BOOL exclusive = TRUE;
    if (setsockopt(state->listener, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
                   reinterpret_cast<const char*>(&exclusive),
                   static_cast<int>(sizeof(exclusive))) == SOCKET_ERROR)
        throw std::runtime_error("Cloud loopback socket policy failed");
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(0);
    if (bind(state->listener, reinterpret_cast<const sockaddr*>(&address),
             static_cast<int>(sizeof(address))) == SOCKET_ERROR)
        throw std::runtime_error("Cloud loopback bind failed");
    sockaddr_in bound{};
    int length = static_cast<int>(sizeof(bound));
    if (getsockname(state->listener, reinterpret_cast<sockaddr*>(&bound),
                    &length) == SOCKET_ERROR)
        throw std::runtime_error("Cloud loopback port query failed");
    state->port = ntohs(bound.sin_port);
    if (state->port == 0 || listen(state->listener, 1) == SOCKET_ERROR)
        throw std::runtime_error("Cloud loopback listen failed");

    // Construct owner BEFORE starting the thread for exception-safe cleanup.
    auto bridge = std::unique_ptr<CloudLoopbackBridge>(
        new CloudLoopbackBridge(std::move(state)));
    bridge->impl_->worker = std::thread(
        [impl = bridge->impl_.get()] { impl->run(); });
    return bridge;
}
std::uint16_t CloudLoopbackBridge::localPort() const noexcept {
    return impl_ && !impl_->stopped.load(std::memory_order_acquire)
        ? impl_->port : 0;
}
void CloudLoopbackBridge::close() noexcept {
    if (impl_) impl_->close();
}
} // namespace sentrium::device
