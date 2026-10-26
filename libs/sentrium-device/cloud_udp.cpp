#include <sentrium/cloud_udp.hpp>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#include <array>
#include <stdexcept>
#include <string_view>

namespace sentrium::device {
namespace {
constexpr std::size_t kMaximumRequestBytes = 8192;
constexpr int kMaximumReplyBytes = 65507;

class WinsockRuntime {
public:
    WinsockRuntime() noexcept {
        WSADATA data{};
        initialized_ = WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }
    ~WinsockRuntime() {
        if (initialized_) WSACleanup();
    }
    [[nodiscard]] bool ready() const noexcept { return initialized_; }
    WinsockRuntime(const WinsockRuntime&) = delete;
    WinsockRuntime& operator=(const WinsockRuntime&) = delete;
private:
    bool initialized_ = false;
};

class SocketHandle {
public:
    explicit SocketHandle(SOCKET value) noexcept : value_(value) {}
    ~SocketHandle() { if (value_ != INVALID_SOCKET) closesocket(value_); }
    [[nodiscard]] SOCKET get() const noexcept { return value_; }
    SocketHandle(const SocketHandle&) = delete;
    SocketHandle& operator=(const SocketHandle&) = delete;
private:
    SOCKET value_;
};

bool isNumericIpv4(std::string_view address) noexcept {
    if (address.empty() || address.size() > 15) return false;
    int dots = 0;
    for (const unsigned char ch : address) {
        if (ch == '.') { ++dots; continue; }
        if (ch < '0' || ch > '9') return false;
    }
    return dots == 3;
}

CloudUdpResult failure(CloudUdpError error, int attempt = 0) {
    return {error, {}, attempt};
}
} // namespace

void CloudUdpSettings::validate() const {
    if (!isNumericIpv4(server_ipv4)) {
        throw std::invalid_argument("Cloud UDP server must be an explicit numeric IPv4 address");
    }
    in_addr parsed{};
    // InetPtonA validates the actual octet values; no hostname resolution.
    if (InetPtonA(AF_INET, server_ipv4.c_str(), &parsed) != 1) {
        throw std::invalid_argument("Cloud UDP server IPv4 address is invalid");
    }
    if (parsed.S_un.S_addr == INADDR_ANY || parsed.S_un.S_addr == INADDR_BROADCAST) {
        throw std::invalid_argument("Cloud UDP server cannot be wildcard or broadcast");
    }
    if (server_port == 0 || timeout_ms < 100 || timeout_ms > 10000 ||
        attempts < 1 || attempts > 3) {
        throw std::invalid_argument("Cloud UDP settings exceed permitted bounds");
    }
}

CloudUdpResult exchangeCloudDatagram(
    const CloudUdpSettings& settings, std::string_view request) {
    try {
        settings.validate();
    } catch (const std::invalid_argument&) {
        return failure(CloudUdpError::InvalidConfiguration);
    }
    if (request.empty() || request.size() > kMaximumRequestBytes) {
        return failure(CloudUdpError::InvalidConfiguration);
    }

    WinsockRuntime runtime;
    if (!runtime.ready()) return failure(CloudUdpError::NetworkInitialization);

    SocketHandle socketHandle(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (socketHandle.get() == INVALID_SOCKET) return failure(CloudUdpError::SocketFailure);

    const DWORD ms = static_cast<DWORD>(settings.timeout_ms);
    if (setsockopt(socketHandle.get(), SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&ms), sizeof(ms)) == SOCKET_ERROR) {
        return failure(CloudUdpError::SocketFailure);
    }

    sockaddr_in remote{};
    remote.sin_family = AF_INET;
    remote.sin_port = htons(settings.server_port);
    if (InetPtonA(AF_INET, settings.server_ipv4.c_str(), &remote.sin_addr) != 1) {
        return failure(CloudUdpError::InvalidConfiguration);
    }
    if (::connect(socketHandle.get(), reinterpret_cast<const sockaddr*>(&remote),
                  static_cast<int>(sizeof(remote))) == SOCKET_ERROR) {
        return failure(CloudUdpError::SocketFailure);
    }

    std::array<char, kMaximumReplyBytes> receiveBuffer{};
    for (int attempt = 1; attempt <= settings.attempts; ++attempt) {
        const int sent = ::send(socketHandle.get(), request.data(),
                                static_cast<int>(request.size()), 0);
        if (sent != static_cast<int>(request.size())) {
            return failure(CloudUdpError::SendFailure, attempt);
        }
        const int count = ::recv(socketHandle.get(), receiveBuffer.data(),
                                 static_cast<int>(receiveBuffer.size()), 0);
        if (count == SOCKET_ERROR) {
            const int error = WSAGetLastError();
            if (error == WSAETIMEDOUT || error == WSAEWOULDBLOCK) {
                if (attempt < settings.attempts) continue;
                return failure(CloudUdpError::Timeout, attempt);
            }
            return failure(CloudUdpError::ReceiveFailure, attempt);
        }
        if (count == 0) return failure(CloudUdpError::InvalidReply, attempt);
        return {CloudUdpError::None,
                std::string(receiveBuffer.data(), static_cast<std::size_t>(count)),
                attempt};
    }
    return failure(CloudUdpError::Timeout, settings.attempts);
}

} // namespace sentrium::device
