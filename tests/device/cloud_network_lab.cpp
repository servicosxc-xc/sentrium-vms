// R10: ONLY a localhost network lab. No real Cloud, NVR, or authenticated peer.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sentrium/cloud_udp_socket.hpp>
#include <sentrium/cloud_ptcp_stream.hpp>
#include <sentrium/cloud_ptcp.hpp>
#include <sentrium/cloud_loopback_bridge.hpp>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

using namespace sentrium::device;
namespace {
constexpr std::uint32_t realm = 77;
constexpr std::uint32_t initialPeerLmid = 500;
void check(bool ok, const char* error) {
    if (!ok) throw std::runtime_error(error);
}

// Remote peer is a real loopback UDP socket but only SYNTHETIC PTCP.
// Its source address does NOT authenticate its application identity.
class LabPeer final {
public:
    enum class Mode { Echo, DropFirst, InvalidRealm, ForgedAck, Silence };
    explicit LabPeer(Mode mode = Mode::Echo): mode_(mode) {
        if (::WSAStartup(MAKEWORD(2,2), &startup_) != 0)
            throw std::runtime_error("WSAStartup failed");
        winsock_ = true;
        sock_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        check(sock_ != INVALID_SOCKET, "lab socket");
        sockaddr_in local{};
        local.sin_family = AF_INET;
        local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        local.sin_port = htons(0);
        check(::bind(sock_, reinterpret_cast<const sockaddr*>(&local),
                     static_cast<int>(sizeof(local))) == 0, "lab bind");
        sockaddr_in actual{};
        int length = static_cast<int>(sizeof(actual));
        check(::getsockname(sock_, reinterpret_cast<sockaddr*>(&actual),
                            &length) == 0, "lab local port");
        port_ = ntohs(actual.sin_port);
        const DWORD deadline = 100;
        check(::setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO,
                           reinterpret_cast<const char*>(&deadline),
                           static_cast<int>(sizeof(deadline))) == 0,
              "lab receive timeout");
        worker_ = std::thread([this] { serve(); });
    }
    ~LabPeer() noexcept {
        stop_.store(true);
        if (worker_.joinable()) worker_.join();
        if (sock_ != INVALID_SOCKET) ::closesocket(sock_);
        if (winsock_) ::WSACleanup();
    }
    LabPeer(const LabPeer&) = delete;
    LabPeer& operator=(const LabPeer&) = delete;
    std::uint16_t port() const noexcept { return port_; }
    int packets() const noexcept { return seen_.load(); }
    void inject(std::uint16_t receiverPort, const std::string& bytes) {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(receiverPort);
        check(::sendto(sock_, bytes.data(), static_cast<int>(bytes.size()), 0,
                       reinterpret_cast<const sockaddr*>(&address),
                       static_cast<int>(sizeof(address))) ==
              static_cast<int>(bytes.size()), "inject UDP wrong peer");
    }
private:
    void serve() noexcept {
        std::uint32_t lmid = initialPeerLmid;
        std::array<char, 2048> bytes{};
        while (!stop_.load()) {
            sockaddr_in client{};
            int length = static_cast<int>(sizeof(client));
            const int n = ::recvfrom(sock_, bytes.data(),
                         static_cast<int>(bytes.size()), 0,
                         reinterpret_cast<sockaddr*>(&client), &length);
            if (n <= 0) continue;
            try {
                const auto wire = std::string_view(
                    bytes.data(), static_cast<std::size_t>(n));
                const auto message = parsePtcpPacket(wire);
                if (!message.ok() || message.packet.kind != PtcpKind::Payload)
                    continue;
                const int count = ++seen_;
                if (mode_ == Mode::Silence ||
                    (mode_ == Mode::DropFirst && count == 1)) continue;
                PtcpPacket reply{};
                reply.kind = PtcpKind::Payload;
                reply.realm = mode_ == Mode::InvalidRealm
                    ? realm + 1 : message.packet.realm;
                reply.local_message_id = lmid++;
                reply.received = mode_ == Mode::ForgedAck
                    ? 999999u : message.packet.sent +
                    static_cast<std::uint32_t>(wire.size() - 24u);
                reply.body = message.packet.body;
                std::string output;
                if (encodePtcpPacket(reply, output) != PtcpError::None)
                    continue;
                static_cast<void>(::sendto(sock_, output.data(),
                    static_cast<int>(output.size()), 0,
                    reinterpret_cast<const sockaddr*>(&client), length));
            } catch (...) { return; }
        }
    }
    WSADATA startup_{};
    SOCKET sock_ = INVALID_SOCKET;
    bool winsock_ = false;
    Mode mode_;
    std::uint16_t port_ = 0;
    std::atomic_bool stop_{false};
    std::atomic<int> seen_{0};
    std::thread worker_;
};

// This adapter is deliberately compiled ONLY into the TEST executable.
// It does NOT verify identity and must NEVER be registered as a production
// VerifiedPtcpDatagramLink. Its sole role is end-to-end local OS I/O testing.
class UnverifiedLabLink final : public VerifiedPtcpDatagramLink {
public:
    explicit UnverifiedLabLink(std::uint16_t port)
        : socket_(PinnedUdpSocket::open("127.0.0.1", port)) {}
    int receive(char* p, int n, int ms) noexcept override {
        return socket_->receiveDatagram(p, n, ms);
    }
    bool transmit(std::string_view bytes) noexcept override {
        return socket_->sendDatagram(bytes);
    }
    void close() noexcept override { socket_->close(); }
    std::uint16_t localPort() const noexcept { return socket_->localPort(); }
private:
    std::unique_ptr<PinnedUdpSocket> socket_;
};

std::unique_ptr<PtcpVerifiedByteStream> streamTo(std::uint16_t port) {
    return PtcpVerifiedByteStream::create(
        std::make_unique<UnverifiedLabLink>(port), realm, initialPeerLmid);
}
std::string receiveBytes(PtcpVerifiedByteStream& stream, std::size_t count,
                         int tries = 50) {
    std::array<char, 1024> buffer{};
    std::string answer;
    for (int attempt = 0; attempt < tries && answer.size() < count; ++attempt) {
        const int n = stream.read(buffer.data(),
                                  static_cast<int>(buffer.size()), 60);
        check(n >= 0, "real UDP stream unexpectedly closed");
        answer.append(buffer.data(), static_cast<std::size_t>(n));
    }
    return answer;
}
void d41() {
    LabPeer peer;
    auto stream = streamTo(peer.port());
    const std::string data(3000, 'A');
    check(stream->write(data), "send actual R10 UDP packets");
    check(receiveBytes(*stream, data.size()) == data,
          "R10 datagram-to-byte stream over real UDP");
    check(peer.packets() == 3, "three UDP payload fragments");
}
void d42() {
    LabPeer peer;
    auto bridge = CloudLoopbackBridge::start(streamTo(peer.port()));
    check(bridge->localPort() != 0, "R10 local NetSDK-shaped TCP port");
    SOCKET client = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    check(client != INVALID_SOCKET, "local TCP socket");
    const DWORD timeout = 3000;
    check(::setsockopt(client, SOL_SOCKET, SO_RCVTIMEO,
                       reinterpret_cast<const char*>(&timeout),
                       static_cast<int>(sizeof(timeout))) == 0,
          "bounded R10 socket receive");
    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    dest.sin_port = htons(bridge->localPort());
    check(::connect(client, reinterpret_cast<const sockaddr*>(&dest),
                    static_cast<int>(sizeof(dest))) == 0, "connect local TCP");
    const std::string probe(3000, 'B');
    check(::send(client, probe.data(), static_cast<int>(probe.size()), 0) ==
          static_cast<int>(probe.size()), "send TCP bytes");
    std::string received;
    std::array<char, 2048> bytes{};
    while (received.size() < probe.size()) {
        const int n = ::recv(client, bytes.data(),
                             static_cast<int>(bytes.size()), 0);
        check(n > 0, "TCP receive via real UDP bridge");
        received.append(bytes.data(), static_cast<std::size_t>(n));
    }
    check(received == probe, "R10 TCP-to-PTCP-to-real-UDP exact roundtrip");
    ::closesocket(client);
    bridge->close();
    check(bridge->localPort() == 0, "bridge closed");
}
void d43() {
    LabPeer peer;
    LabPeer impostor(LabPeer::Mode::Silence);
    auto link = std::make_unique<UnverifiedLabLink>(peer.port());
    const std::uint16_t port = link->localPort();
    auto stream = PtcpVerifiedByteStream::create(
        std::move(link), realm, initialPeerLmid);
    PtcpPacket spoof{};
    spoof.kind = PtcpKind::Payload;
    spoof.realm = realm;
    spoof.local_message_id = initialPeerLmid;
    spoof.body = "bad-peer";
    std::string bytes;
    check(encodePtcpPacket(spoof, bytes) == PtcpError::None,
          "encode spoof for local test");
    impostor.inject(port, bytes);
    char response[128]{};
    check(stream->read(response, 128, 80) == 0,
          "connected UDP filters foreign port");
    check(stream->write("right-peer"), "send to pinned port");
    check(receiveBytes(*stream, 10) == "right-peer",
          "actual peer still works after spoof");
}
void d44() {
    for (const auto mode : {LabPeer::Mode::InvalidRealm,
                            LabPeer::Mode::ForgedAck}) {
        LabPeer peer(mode);
        auto stream = streamTo(peer.port());
        check(stream->write("probe"), "send lab failure probe");
        char buffer[64]{};
        int status = 0;
        for (int n = 0; n < 20 && status == 0; ++n)
            status = stream->read(buffer, 64, 60);
        check(status == -1 && !stream->write("after-failure"),
              "invalid realm/ACK from real UDP fails closed");
    }
}
void d45() {
    LabPeer peer(LabPeer::Mode::DropFirst);
    auto stream = streamTo(peer.port());
    const std::string data = "R10-retransmit";
    check(stream->write(data), "initial real UDP send");
    check(receiveBytes(*stream, data.size(), 35) == data,
          "PTCP retransmits lost UDP datagram");
    check(peer.packets() >= 2, "peer observed dropped first + retry");
}
void d46() {
    LabPeer peer(LabPeer::Mode::Silence);
    auto stream = streamTo(peer.port());
    std::atomic<int> status{10};
    std::thread consumer([&] {
        char data[64]{};
        status.store(stream->read(data, 64, 1000));
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    stream->close();
    consumer.join();
    check(status.load() == -1 && !stream->write("after-close"),
          "concurrent close interrupts real UDP read");
}
} // namespace

int main(int argc, char** argv) {
    try {
        check(argc == 2, "R10 test number required");
        switch (std::stoi(argv[1])) {
        case 41: d41(); break;
        case 42: d42(); break;
        case 43: d43(); break;
        case 44: d44(); break;
        case 45: d45(); break;
        case 46: d46(); break;
        default: throw std::runtime_error("unknown R10 test number");
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "R10 FAILED: " << e.what() << '\n';
        return 1;
    }
}
