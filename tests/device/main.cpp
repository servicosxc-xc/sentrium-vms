#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sentrium/device.hpp>
#include <sentrium/cloud.hpp>
#include <sentrium/cloud_wire.hpp>
#include <sentrium/cloud_udp.hpp>
#include <sentrium/cloud_udp_socket.hpp>
#include <sentrium/cloud_signaling.hpp>
#include <sentrium/cloud_ptcp.hpp>
#include <sentrium/cloud_ptcp_session.hpp>
#include <sentrium/cloud_ptcp_receive_window.hpp>
#include <sentrium/cloud_ptcp_channel.hpp>
#include <sentrium/cloud_ptcp_stream.hpp>
#include <sentrium/cloud_loopback_bridge.hpp>
#include <sentrium/cloud_stream_tunnel.hpp>
#include <sentrium/cloud_auth.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <thread>
#include <string_view>
#include <cstring>
#include <deque>
#include <mutex>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace sentrium::device;

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <class F>
void rejects(F action) {
    try {
        action();
    } catch (const std::invalid_argument&) {
        return;
    }
    throw std::runtime_error("expected invalid_argument");
}

class FakeAdapter final : public Adapter {
public:
    ConnectResult connect(const LoginRequest& request) override {
        request.validate();
        last_endpoint = request.endpoint();
        return ConnectResult{true, Session{42}, 4, "fake connected"};
    }

    DisconnectResult disconnect(Session session) override {
        require(session.value == 42, "fake session");
        disconnected = true;
        return DisconnectResult{true, {}};
    }

    std::string last_endpoint;
    bool disconnected = false;
};

// Deterministic lifecycle test double only. This is NOT a Cloud provider.
class TraceTunnel final : public CloudTunnel {
public:
    explicit TraceTunnel(std::vector<std::string>& trace) : trace_(trace) {}
    CloudTunnelEndpoint endpoint() const override { return {59143, CloudRoute::Relay}; }
    void close() noexcept override {
        if (!closed_) {
            closed_ = true;
            trace_.push_back("tunnel-close");
        }
    }
private:
    std::vector<std::string>& trace_;
    bool closed_ = false;
};

class TraceTransport final : public CloudTransport {
public:
    explicit TraceTransport(std::vector<std::string>& trace) : trace_(trace) {}
    std::unique_ptr<CloudTunnel> open(const CloudConnectRequest& request) override {
        request.validate();
        trace_.push_back("tunnel-open");
        return std::make_unique<TraceTunnel>(trace_);
    }
private:
    std::vector<std::string>& trace_;
};

class TraceNetSdk final : public Adapter {
public:
    explicit TraceNetSdk(std::vector<std::string>& trace) : trace_(trace) {}
    ConnectResult connect(const LoginRequest& login) override {
        login.validate();
        require(login.host == "127.0.0.1" && login.port == 59143,
                "NetSDK must receive dynamic local tunnel endpoint");
        trace_.push_back("netsdk-login");
        return {true, Session{79}, 16, "test login"};
    }
    DisconnectResult disconnect(Session session) override {
        require(session.value == 79, "Unexpected NetSDK session");
        trace_.push_back("netsdk-logout");
        return {true, {}};
    }
private:
    std::vector<std::string>& trace_;
};

class TestOnlyEchoStream final : public CloudByteStream {
public:
    // Loopback fixture only. This is NOT an authorized Cloud connection.
    int read(char* buffer, int capacity, int) noexcept override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) return -1;
        if (size_ == 0) return 0;
        if (capacity < static_cast<int>(size_)) return -1;
        std::memcpy(buffer, pending_.data(), size_);
        const int amount = static_cast<int>(size_);
        size_ = 0;
        return amount;
    }
    bool write(std::string_view bytes) noexcept override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_ || size_ != 0 || bytes.size() > pending_.size()) return false;
        std::memcpy(pending_.data(), bytes.data(), bytes.size());
        size_ = bytes.size();
        return true;
    }
    void close() noexcept override {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;
    }
private:
    std::mutex mutex_;
    std::array<char, 512> pending_{};
    std::size_t size_ = 0;
    bool closed_ = false;
};

class FixtureStreamTransport final : public CloudTransport {
public:
    std::unique_ptr<CloudTunnel> open(const CloudConnectRequest& request) override {
        request.validate();
        ++opens;
        return CloudStreamTunnel::create(std::make_unique<TestOnlyEchoStream>());
    }
    int opens = 0;
};

// Only a test Adapter: the NetSDK-shaped flow below has no real NVR login.
class FixtureNetSdkSocket final : public Adapter {
public:
    explicit FixtureNetSdkSocket(int channelCount, bool failFirstLogout = false)
        : count_(channelCount), fail_first_(failFirstLogout) {}
    ~FixtureNetSdkSocket() override {
        if (client_ != INVALID_SOCKET) ::closesocket(client_);
    }
    ConnectResult connect(const LoginRequest& request) override {
        request.validate();
        require(request.host == "127.0.0.1" && request.port != 0,
                "only local dynamic NetSDK endpoint allowed");
        client_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        require(client_ != INVALID_SOCKET, "fake SDK socket");
        const DWORD timeout = 2000;
        require(::setsockopt(client_, SOL_SOCKET, SO_RCVTIMEO,
                    reinterpret_cast<const char*>(&timeout),
                    static_cast<int>(sizeof(timeout))) == 0, "socket recv timeout");
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(request.port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        require(::connect(client_, reinterpret_cast<const sockaddr*>(&addr),
                    static_cast<int>(sizeof(addr))) == 0, "fixture TCP connected");
        const std::string_view data = "synthetic-netsdk-probe";
        require(::send(client_, data.data(), static_cast<int>(data.size()), 0)
                    == static_cast<int>(data.size()), "fixture TCP sent");
        std::array<char,128> response{};
        std::string echoed;
        while (echoed.size() < data.size()) {
            const int n = ::recv(client_, response.data(),
                                static_cast<int>(response.size()), 0);
            require(n > 0, "fixture TCP echo received");
            echoed.append(response.data(), static_cast<std::size_t>(n));
        }
        require(echoed == data, "exact ordered byte stream");
        ++logins;
        return {true, Session{615}, count_, "fixture only"};
    }
    DisconnectResult disconnect(Session session) override {
        require(session.value == 615, "fixture session");
        ++logouts;
        if (fail_first_) {
            fail_first_ = false;
            return {false, "synthetic failure"};
        }
        if (client_ != INVALID_SOCKET) {
            ::shutdown(client_, SD_BOTH);
            ::closesocket(client_);
            client_ = INVALID_SOCKET;
        }
        return {true, {}};
    }
    int logins = 0;
    int logouts = 0;
private:
    int count_;
    bool fail_first_;
    SOCKET client_ = INVALID_SOCKET;
};

class ScriptedVerifiedPtcpLink final : public VerifiedPtcpDatagramLink {
public:
    explicit ScriptedVerifiedPtcpLink(
        bool autoEcho = false, std::uint32_t firstPeerId = 50)
        : auto_echo_(autoEcho), next_peer_id_(firstPeerId) {}

    int receive(char* buffer, int capacity, int) noexcept override {
        if (closed_.load()) return -1;
        if (inbound.empty()) return 0;
        const auto& next = inbound.front();
        if (static_cast<int>(next.size()) > capacity) return -1;
        std::memcpy(buffer, next.data(), next.size());
        const int count = static_cast<int>(next.size());
        inbound.pop_front();
        return count;
    }

    bool transmit(std::string_view wire) noexcept override {
        if (closed_.load() || outbound.size() == fail_at_write) return false;
        outbound.emplace_back(wire);
        if (auto_echo_) {
            const auto frame = parsePtcpPacket(wire);
            if (!frame.ok()) return false;
            if (frame.packet.kind == PtcpKind::Payload) {
                PtcpPacket peer{};
                peer.kind = PtcpKind::Payload;
                peer.local_message_id = next_peer_id_++;
                peer.realm = frame.packet.realm;
                peer.received = frame.packet.sent +
                    static_cast<std::uint32_t>(wire.size() - 24u);
                peer.body = frame.packet.body;
                std::string reply;
                if (encodePtcpPacket(peer, reply) != PtcpError::None)
                    return false;
                inbound.push_back(std::move(reply));
            }
        }
        return true;
    }

    void close() noexcept override {
        closed_.store(true);
        ++closes;
    }

    std::vector<std::string> outbound;
    std::deque<std::string> inbound;
    std::size_t fail_at_write = static_cast<std::size_t>(-1);
    int closes = 0;

private:
    bool auto_echo_ = false;
    std::uint32_t next_peer_id_ = 50;
    std::atomic_bool closed_{false};
};

// Only a controlled Windows UDP test peer. This does not implement the
// Intelbras Cloud service or authenticate a remote NVR.
class LocalUdpTestPeer final {
public:
    LocalUdpTestPeer() {
        if (WSAStartup(MAKEWORD(2, 2), &wsa_) != 0)
            throw std::runtime_error("UDP test fixture Winsock failed");
        started_ = true;
        socket_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socket_ == INVALID_SOCKET)
            throw std::runtime_error("UDP test socket unavailable");
        sockaddr_in local{};
        local.sin_family = AF_INET;
        local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        local.sin_port = htons(0);
        require(::bind(socket_, reinterpret_cast<sockaddr*>(&local),
                    static_cast<int>(sizeof(local))) == 0,
                "fixture local UDP bind");
        sockaddr_in actual{};
        int size = static_cast<int>(sizeof(actual));
        require(::getsockname(socket_, reinterpret_cast<sockaddr*>(&actual),
                    &size) == 0, "fixture local UDP port");
        port_ = ntohs(actual.sin_port);
        const DWORD deadline = 1000;
        require(::setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO,
                    reinterpret_cast<const char*>(&deadline),
                    static_cast<int>(sizeof(deadline))) == 0,
                "fixture UDP recv deadline");
    }
    ~LocalUdpTestPeer() noexcept {
        if (socket_ != INVALID_SOCKET) ::closesocket(socket_);
        if (started_) WSACleanup();
    }
    LocalUdpTestPeer(const LocalUdpTestPeer&) = delete;
    LocalUdpTestPeer& operator=(const LocalUdpTestPeer&) = delete;

    [[nodiscard]] std::uint16_t port() const noexcept { return port_; }
    std::string receiveOne() {
        std::array<char, 8192> buffer{};
        last_sender_size_ = static_cast<int>(sizeof(last_sender_));
        const int received = ::recvfrom(socket_, buffer.data(),
            static_cast<int>(buffer.size()), 0,
            reinterpret_cast<sockaddr*>(&last_sender_), &last_sender_size_);
        require(received > 0, "fixture UDP inbound datagram");
        return std::string(buffer.data(), static_cast<std::size_t>(received));
    }
    void replyToLast(std::string_view bytes) {
        require(last_sender_size_ == static_cast<int>(sizeof(last_sender_)),
                "test peer must receive first");
        require(::sendto(socket_, bytes.data(), static_cast<int>(bytes.size()),
                    0, reinterpret_cast<const sockaddr*>(&last_sender_),
                    last_sender_size_) == static_cast<int>(bytes.size()),
                "fixture UDP reply to client");
    }
    void sendToLocalPort(std::uint16_t port, std::string_view bytes) {
        sockaddr_in dest{};
        dest.sin_family = AF_INET;
        dest.sin_port = htons(port);
        dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        require(::sendto(socket_, bytes.data(), static_cast<int>(bytes.size()),
                    0, reinterpret_cast<const sockaddr*>(&dest),
                    static_cast<int>(sizeof(dest))) ==
                static_cast<int>(bytes.size()), "fixture raw UDP send");
    }
private:
    WSADATA wsa_{};
    SOCKET socket_ = INVALID_SOCKET;
    bool started_ = false;
    std::uint16_t port_ = 0;
    sockaddr_in last_sender_{};
    int last_sender_size_ = 0;
};
} // namespace

int main(int argc, char** argv) {
    try {
        require(argc == 2, "test id required");
        const int id = std::stoi(argv[1]);

        switch (id) {
        case 1:
            require(name(ConnectionState::Disconnected) == "Disconnected", "Disconnected name");
            require(name(ConnectionState::Connecting) == "Connecting", "Connecting name");
            require(name(ConnectionState::Connected) == "Connected", "Connected name");
            require(name(ConnectionState::Disconnecting) == "Disconnecting", "Disconnecting name");
            require(name(ConnectionState::Failed) == "Failed", "Failed name");
            require(name(AccessMethod::Lan) == "LAN", "LAN method");
            require(name(AccessMethod::Cloud) == "Cloud", "Cloud method");
            require(name(AccessMethod::AutoRegistration) == "AutoRegistration", "AutoRegistration method");
            require(name(CloudRoute::Unknown) == "Unknown", "Unknown cloud route");
            require(name(CloudRoute::Direct) == "Direct", "Direct cloud route");
            require(name(CloudRoute::Relay) == "Relay", "Relay cloud route");
            require(Device{}.cloud_route == CloudRoute::Unknown,
                    "Never guess P2P Direct from an uninitialized route");
            break;

        case 2: {
            LoginRequest request{"192.0.2.10", 37777, "operator", "secret-value"};
            request.validate();
            require(request.endpoint() == "192.0.2.10:37777", "endpoint");
            require(request.endpoint().find(request.username) == std::string::npos, "username not in endpoint");
            require(request.endpoint().find(request.password) == std::string::npos, "password not in endpoint");
            break;
        }

        case 3: {
            rejects([] { LoginRequest{"", 37777, "user", "pass"}.validate(); });
            rejects([] { LoginRequest{"host with space", 37777, "user", "pass"}.validate(); });
            rejects([] { LoginRequest{"127.0.0.1", 0, "user", "pass"}.validate(); });
            rejects([] { LoginRequest{"127.0.0.1", 37777, "", "pass"}.validate(); });
            rejects([] {
                LoginRequest request{"127.0.0.1", 37777, "user", std::string(257, 'x')};
                request.validate();
            });
            break;
        }

        case 4: {
            const auto channels = make_channels(3);
            require(channels.size() == 3, "channel count");
            require(channels[0].index == 0 && channels[0].name == "Canal 01", "channel 1");
            require(channels[1].index == 1 && channels[1].name == "Canal 02", "channel 2");
            require(channels[2].index == 2 && channels[2].name == "Canal 03", "channel 3");
            break;
        }

        case 5:
            rejects([] { static_cast<void>(make_channels(-1)); });
            rejects([] { static_cast<void>(make_channels(4097)); });
            require(make_channels(0).empty(), "zero channels");
            break;

        case 6: {
            FakeAdapter adapter;
            LoginRequest request{"198.51.100.20", 37777, "admin", "never-log-this"};
            const auto connected = adapter.connect(request);
            require(connected.success && connected.session.value == 42, "fake connect");
            require(connected.channel_count == 4, "fake channel count");
            require(adapter.last_endpoint == "198.51.100.20:37777", "fake endpoint");
            const auto disconnected = adapter.disconnect(connected.session);
            require(disconnected.success && adapter.disconnected, "fake disconnect");
            break;
        }

        case 7: {
            const std::string secret = "sentrium-secret-fixture";
            LoginRequest request{"203.0.113.7", 37777, "operator", secret};
            FakeAdapter adapter;
            const auto result = adapter.connect(request);
            require(result.diagnostic.find(secret) == std::string::npos, "secret not in diagnostic");
            require(adapter.last_endpoint.find(secret) == std::string::npos, "secret not in endpoint");
            Device device{
                "device-1",
                "NVR Teste",
                request.host,
                request.port,
                "Intelbras",
                ConnectionState::Connected,
                make_channels(result.channel_count),
                result.diagnostic};
            require(device.diagnostic.find(secret) == std::string::npos, "secret not in model");
            break;
        }

        case 8: {
            // Only a real provider can supply a ready dynamic TCP port.
            CloudConnectRequest request{"cloud-uuid-example", 37000};
            request.validate();
            rejects([] { CloudConnectRequest{"device", 0}.validate(); });
            rejects([] { CloudConnectRequest{"", 37000}.validate(); });
            rejects([] { CloudConnectRequest{"contains spaces", 37000}.validate(); });

            CloudTunnelEndpoint endpoint{59143, CloudRoute::Relay};
            endpoint.validate();
            const auto login = endpoint.netSdkLogin("operator", "secret");
            require(login.host == "127.0.0.1", "Cloud NetSDK host must be loopback");
            require(login.port == 59143, "Cloud NetSDK port must come from live tunnel");
            require(login.port != request.service_port,
                    "Remote service port must not leak into local NetSDK login");
            require(endpoint.observed_route == CloudRoute::Relay, "Relay must stay Relay");
            require(CloudTunnelEndpoint{59144}.observed_route == CloudRoute::Unknown,
                    "Do not infer direct P2P from presence of tunnel");
            rejects([] { CloudTunnelEndpoint{}.validate(); });

            std::vector<std::string> trace;
            TraceTransport transport(trace);
            TraceNetSdk sdk(trace);
            {
                CloudDeviceSession session(transport, sdk);
                const auto result = session.connect(request, "operator", "private-test-password");
                require(result.success && result.channel_count == 16, "NetSDK tunnel login");
                require(result.route == CloudRoute::Relay && session.connected(),
                        "The route must come from the tunnel, not the login");
                require(trace == std::vector<std::string>{"tunnel-open", "netsdk-login"},
                        "Opening the Cloud tunnel must precede NetSDK login");
                const auto closed = session.disconnect();
                require(closed.success && !session.connected(), "Cloud disconnect");
                require(session.route() == CloudRoute::Unknown, "No stale route after disconnect");
            }
            require(trace == std::vector<std::string>{
                        "tunnel-open", "netsdk-login", "netsdk-logout", "tunnel-close"},
                    "NetSDK logout must precede tunnel teardown, exactly once");
            break;
        }

        case 9: {
            // Real DHP2P response framing, not a simulated live Cloud tunnel.
            const std::string xml = "<body><US>example.invalid:8800</US></body>";
            const std::string raw =
                "DHP2P/1.0 200 OK\r\nCSeq: 12\r\nContent-Type: text/xml\r\n"
                "Content-Length: " + std::to_string(xml.size()) + "\r\n\r\n" + xml;
            auto parsed = parseCloudWireResponse(raw);
            require(parsed.ok() && parsed.response.status_code == 200, "valid DH 200");
            require(parsed.response.headers.at("cseq") == "12", "case-folded CSeq");
            require(parsed.response.body == xml, "exact XML payload bytes");
            require(parsed.response.reason == "OK", "response reason");

            parsed = parseCloudWireResponse(
                "DHP2P/1.0 403 Forbidden\nCSeq: 13\n\n<error/>");
            require(parsed.ok() && parsed.response.status_code == 403,
                    "403 is a valid protocol response, not login success");
            require(parsed.response.body == "<error/>", "LF-only message");

            require(parseCloudWireResponse("").error == CloudWireError::Empty,
                    "empty response rejected");
            require(parseCloudWireResponse("DHP2P/1.0 200 OK\r\nCSeq: 1\r\n").error ==
                    CloudWireError::BadHeader, "missing header terminator");
            require(parseCloudWireResponse("HTTP/1.1 200 OK\r\n\r\n").error ==
                    CloudWireError::BadStatus, "wrong protocol rejected");
            require(parseCloudWireResponse("DHP2P/1.0 abc Bad\r\n\r\n").error ==
                    CloudWireError::BadStatus, "invalid code rejected");
            require(parseCloudWireResponse(
                "DHP2P/1.0 200 OK\r\nCSeq: 1\r\ncseq: 2\r\n\r\n").error ==
                    CloudWireError::BadHeader, "duplicated header rejected");
            require(parseCloudWireResponse(
                "DHP2P/1.0 200 OK\r\nContent-Length: 7\r\n\r\ntiny").error ==
                    CloudWireError::BadBodyLength, "truncated content rejected");
            require(parseCloudWireResponse(
                "DHP2P/1.0 200 OK\r\nBad Header: x\r\n\r\n").error ==
                    CloudWireError::BadHeader, "invalid header name rejected");
            require(parseCloudWireResponse(
                "DHP2P/1.0 200 OK\r\nBad: x\rno\n\r\n").error ==
                    CloudWireError::BadHeader, "bare CR rejected");
            require(parseCloudWireResponse(std::string(65508, 'x')).error ==
                    CloudWireError::TooLarge, "oversize datagram rejected");
            break;
        }

        case 10: {
            // Fully offline. No Cloud host or identity is needed to validate
            // config bounds, call-level fail-closed behavior and public API.
            CloudUdpSettings local{"127.0.0.1", 8800, 200, 1};
            local.validate();
            rejects([] { CloudUdpSettings{"", 8800, 200, 1}.validate(); });
            rejects([] { CloudUdpSettings{"example.com", 8800, 200, 1}.validate(); });
            rejects([] { CloudUdpSettings{"256.256.0.1", 8800, 200, 1}.validate(); });
            rejects([] { CloudUdpSettings{"0.0.0.0", 8800, 200, 1}.validate(); });
            rejects([] { CloudUdpSettings{"255.255.255.255", 8800, 200, 1}.validate(); });
            rejects([] { CloudUdpSettings{"224.0.0.1", 8800, 200, 1}.validate(); });
            rejects([] { CloudUdpSettings{"240.0.0.1", 8800, 200, 1}.validate(); });
            rejects([] { CloudUdpSettings{"127.0.0.1", 0, 200, 1}.validate(); });
            rejects([] { CloudUdpSettings{"127.0.0.1", 8800, 99, 1}.validate(); });
            rejects([] { CloudUdpSettings{"127.0.0.1", 8800, 10001, 1}.validate(); });
            rejects([] { CloudUdpSettings{"127.0.0.1", 8800, 200, 4}.validate(); });

            const auto invalid = exchangeCloudDatagram(local, {});
            require(invalid.error == CloudUdpError::InvalidConfiguration &&
                    invalid.attempts_used == 0, "empty wire request rejected before network");
            const auto overlarge = exchangeCloudDatagram(local, std::string(8193, 'x'));
            require(overlarge.error == CloudUdpError::InvalidConfiguration,
                    "oversize payload rejected before network");

            const auto signaling = exchangeCloudSignaling(local, {});
            require(signaling.status == CloudSignalingStatus::InvalidConfiguration &&
                    !signaling.accepted() && signaling.response.body.empty(),
                    "signaling refuses unauthenticated empty request");
            break;
        }

        case 11: {
            // PTCP framing only, not an authenticated Cloud connection.
            PtcpPacket request{};
            request.kind = PtcpKind::Payload;
            request.sent = 12;
            request.received = 4;
            request.pid = 0xffff;
            request.local_message_id = 9;
            request.remote_message_id = 8;
            request.realm = 77;
            request.body = "control-test-bytes";
            std::string raw;
            require(encodePtcpPacket(request,raw) == PtcpError::None, "encode payload");
            const auto round = parsePtcpPacket(raw);
            require(round.ok(), "decode encoded payload");
            require(round.packet.body == request.body &&
                    round.packet.pid == request.pid &&
                    round.packet.local_message_id == 9 &&
                    round.packet.realm == 77, "PTCP payload/header roundtrip");

            const auto oldRaw = raw;
            raw[0] = 'X';
            require(parsePtcpPacket(raw).error == PtcpError::WrongMagic,
                    "reject wrong protocol");
            raw = oldRaw;
            raw[24] = static_cast<char>(0x11);
            require(parsePtcpPacket(raw).error == PtcpError::InvalidLength,
                    "reject incoherent bind length");

            request.kind = PtcpKind::Sync;
            request.body.clear();
            require(encodePtcpPacket(request,raw) == PtcpError::None &&
                    parsePtcpPacket(raw).packet.kind == PtcpKind::Sync, "SYNC frame");
            request.kind = PtcpKind::Heartbeat;
            require(encodePtcpPacket(request,raw) == PtcpError::None &&
                    parsePtcpPacket(raw).packet.kind == PtcpKind::Heartbeat, "heartbeat");
            request.kind = PtcpKind::Status;
            request.status = "CONN";
            require(encodePtcpPacket(request,raw) == PtcpError::None &&
                    parsePtcpPacket(raw).packet.status == "CONN", "CONN");
            request.status = "FAKE";
            require(encodePtcpPacket(request,raw) == PtcpError::InvalidStatus,
                    "unknown PTCP status rejected");
            request.kind = PtcpKind::Bind;
            request.bind_port = 37777;
            require(encodePtcpPacket(request,raw) == PtcpError::None &&
                    parsePtcpPacket(raw).packet.bind_port == 37777, "BIND");
            request.kind = PtcpKind::Payload;
            request.body.assign(1281,'x');
            require(encodePtcpPacket(request,raw) == PtcpError::TooLarge,
                    "respect MTU limit");
            request.kind = PtcpKind::Command;
            request.body.assign(12, '\0');
            request.body[0]=static_cast<char>(0x19);
            require(encodePtcpPacket(request,raw) == PtcpError::None &&
                    parsePtcpPacket(raw).packet.kind == PtcpKind::Command,
                    "opaque auth command");
            require(parsePtcpPacket("PTCP").error == PtcpError::TooShort,
                    "short packet");
            break;
        }

        case 12: {
            // Device-owned credentials, not application bootstrap credentials.
            const auto auth = prepareCloudDeviceAuth(
                "admin","hunter2","abc123",12345,1761500000ULL,"127.0.0.1:49300");
            require(auth.device_key_hex.size() == 32 &&
                    auth.signature_base64.size() == 44,
                    "device authentication encodings");
            require(auth.device_key_hex.find("hunter2") == std::string::npos &&
                    auth.signature_base64.find("hunter2") == std::string::npos,
                    "no cleartext password in auth");
            for (const char c : auth.device_key_hex)
                require((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'),
                        "uppercase MD5 hex");
            const auto repeat = prepareCloudDeviceAuth(
                "admin","hunter2","abc123",12345,1761500000ULL,"127.0.0.1:49300");
            require(auth.device_key_hex == repeat.device_key_hex &&
                    auth.signature_base64 == repeat.signature_base64 &&
                    auth.address_key == repeat.address_key,
                    "reproducible device proof");

            // AES-OFB uses the derived key and a caller-supplied IV; no
            // untrusted vendor-app initialization constant in the binary.
            const std::array<std::uint8_t,16> iv{
                1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
            const std::string address = "127.0.0.1:49300";
            const auto encrypted = cryptCloudAddress(auth.address_key,iv,address);
            require(encrypted.size() == address.size(), "AES-OFB preserves length");
            const std::string encryptedBytes(encrypted.begin(),encrypted.end());
            require(encryptedBytes != address, "AES-OFB must transform address");
            const auto decrypted = cryptCloudAddress(
                auth.address_key,iv,encryptedBytes);
            require(std::string(decrypted.begin(),decrypted.end()) == address,
                    "AES-OFB roundtrip");

            const auto wrongPass = prepareCloudDeviceAuth(
                "admin","other-password","abc123",12345,1761500000ULL,"127.0.0.1:49300");
            require(wrongPass.device_key_hex != auth.device_key_hex &&
                    wrongPass.signature_base64 != auth.signature_base64,
                    "password affects proof");
            const auto wrongNonce = prepareCloudDeviceAuth(
                "admin","hunter2","abc123",12346,1761500000ULL,"127.0.0.1:49300");
            require(wrongNonce.device_key_hex == auth.device_key_hex &&
                    wrongNonce.address_key != auth.address_key &&
                    wrongNonce.signature_base64 != auth.signature_base64,
                    "nonce affects derived key and signature");

            rejects([] {
                static_cast<void>(prepareCloudDeviceAuth(
                    "admin","", "abc123",12345,1761500000ULL,"payload"));
            });
            rejects([] {
                static_cast<void>(prepareCloudDeviceAuth(
                    "admin","pass", "abc123",0,1761500000ULL,"payload"));
            });
            break;
        }

        case 13: {
            PtcpReliableSession s;
            PtcpPacket frame{};
            frame.kind = PtcpKind::Payload;
            frame.realm = 7;
            frame.body = "hello";
            frame.sent = 999; frame.pid = 77;
            const auto first = s.send(frame, 1000);
            require(first.ok(), "R3 first payload");
            const auto decoded = parsePtcpPacket(first.datagram);
            require(decoded.ok() && decoded.packet.sent == 0 &&
                    decoded.packet.received == 0 && decoded.packet.pid == 0xffffu &&
                    decoded.packet.local_message_id == 0 &&
                    decoded.packet.body == "hello", "owned PTCP header");
            require(s.stats().sent_bytes == 17 &&
                    s.stats().in_flight_packets == 1, "body bytes counted");
            PtcpPacket ack{};
            ack.kind = PtcpKind::Heartbeat;
            ack.received = 17;
            ack.local_message_id = 90;
            std::string wire;
            require(encodePtcpPacket(ack,wire) == PtcpError::None &&
                    s.receive(wire).ok(), "peer ACK");
            require(s.stats().peer_acked_bytes == 17 &&
                    s.stats().in_flight_packets == 0, "ACK retires bytes");
            require(s.receive(wire).error == PtcpSessionError::Duplicate &&
                    s.stats().duplicates == 1 &&
                    s.stats().received_bytes == 12, "replay rejected");
            auto second = s.send(frame, 1001);
            const auto next = parsePtcpPacket(second.datagram);
            require(second.ok() && next.ok() && next.packet.sent == 17 &&
                    next.packet.received == 12 && next.packet.pid == 0xfffeu &&
                    next.packet.local_message_id == 1 &&
                    next.packet.remote_message_id == 90, "next frame sequence");
            require(s.send(frame, 999).error == PtcpSessionError::ClockReversed,
                    "monotonic time enforced");
            break;
        }
        case 14: {
            PtcpReliableSession s;
            PtcpPacket frame{};
            frame.kind = PtcpKind::Payload;
            frame.body = "abc";
            const auto first = s.send(frame, 1000);
            require(first.ok() && s.dueRetransmissions(1200).empty(),
                    "early retry suppressed");
            const auto retry = s.dueRetransmissions(1400);
            require(retry.size() == 1 && retry.front() == first.datagram &&
                    s.stats().sent_bytes == 15 && s.stats().retransmissions == 1,
                    "retry exact wire and no double counting");
            PtcpPacket peer{};
            peer.kind = PtcpKind::Heartbeat;
            peer.local_message_id = 10;
            peer.received = 999;
            std::string raw;
            require(encodePtcpPacket(peer,raw) == PtcpError::None &&
                    s.receive(raw).error == PtcpSessionError::InvalidAck &&
                    s.stats().in_flight_packets == 1, "forged ACK rejected");
            peer.received = 0;
            require(encodePtcpPacket(peer,raw) == PtcpError::None &&
                    s.receive(raw).ok(), "first peer frame");
            peer.local_message_id = 12;
            require(encodePtcpPacket(peer,raw) == PtcpError::None &&
                    s.receive(raw).error == PtcpSessionError::SequenceGap &&
                    s.stats().received_bytes == 12, "gap not delivered");
            require(s.dueRetransmissions(1800).size() == 1, "retry two");
            require(s.dueRetransmissions(2200).size() == 1, "retry three");
            require(s.dueRetransmissions(2600).empty() && s.expired(),
                    "retry cap expires session");
            require(s.send(frame,2700).error == PtcpSessionError::Expired,
                    "closed session refuses write");
            PtcpReliableSession full;
            for(int i=0;i<32;++i) require(full.send(frame,1000).ok(),"window slot");
            require(full.stats().in_flight_packets == 32 &&
                    full.send(frame,1000).error == PtcpSessionError::WindowFull,
                    "window cannot grow without ACK");
            break;
        }

        case 15: {
            // One-shot dynamic loopback. No provider, remote recorder or P2P.
            rejects([] {
                static_cast<void>(CloudLoopbackBridge::start(nullptr));
            });
            auto bridge = CloudLoopbackBridge::start(
                std::make_unique<TestOnlyEchoStream>());
            require(bridge && bridge->localPort() != 0,
                    "OS-assigned local bridge TCP port");
            bridge->close();
            require(bridge->localPort() == 0, "closed bridge exposes no port");
            bridge->close();
            break;
        }
        case 16: {
            // Native Winsock to in-memory echo, not a Cloud/NetSDK login.
            auto bridge = CloudLoopbackBridge::start(
                std::make_unique<TestOnlyEchoStream>());
            require(bridge && bridge->localPort() != 0,
                    "native bridge is listening on loopback");
            SOCKET client = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            require(client != INVALID_SOCKET, "create local test socket");
            const DWORD timeout = 3000;
            require(::setsockopt(client, SOL_SOCKET, SO_RCVTIMEO,
                     reinterpret_cast<const char*>(&timeout),
                     static_cast<int>(sizeof(timeout))) == 0, "recv deadline");

            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            address.sin_port = htons(bridge->localPort());
            require(::connect(client, reinterpret_cast<const sockaddr*>(&address),
                    static_cast<int>(sizeof(address))) == 0, "connect loopback");
            const std::string payload = "SENTRIUM-R4-test-only";
            require(::send(client, payload.data(),
                    static_cast<int>(payload.size()), 0) ==
                    static_cast<int>(payload.size()), "send NetSDK-like bytes");
            std::string echoed;
            std::array<char, 512> buffer{};
            while (echoed.size() < payload.size()) {
                const int count = ::recv(client, buffer.data(),
                    static_cast<int>(buffer.size()), 0);
                require(count > 0, "receive response before deadline");
                echoed.append(buffer.data(), static_cast<std::size_t>(count));
            }
            require(echoed == payload, "full duplex loopback echo");
            ::closesocket(client);
            bridge->close();
            require(bridge->localPort() == 0, "teardown released port");
            break;
        }

        case 17: {
            // Local-only fixtures; returned channel counts do NOT originate
            // from a recorder, and Cloud authentication has NOT occurred.
            for (const int channels : {16, 32}) {
                FixtureStreamTransport transport;
                FixtureNetSdkSocket sdk(channels);
                CloudDeviceSession session(transport,sdk);
                CloudConnectRequest request{"fictional-local-fixture",37777};
                const auto result = session.connect(request,"test-user","test-pass");
                require(result.success && result.channel_count == channels &&
                        session.connected() && result.route == CloudRoute::Unknown,
                        "fixture adapter end-to-end TCP connection");
                require(transport.opens == 1 && sdk.logins == 1,
                        "one local stream and one fixture login");
                require(session.disconnect().success && !session.connected() &&
                        sdk.logouts == 1, "logout before local bridge cleanup");
            }
            break;
        }
        case 18: {
            FixtureStreamTransport transport;
            FixtureNetSdkSocket sdk(32,true);
            CloudDeviceSession session(transport,sdk);
            CloudConnectRequest request{"fictional-local-fixture",37777};
            require(session.connect(request,"fixture","password").success,
                    "test-only connection established");
            require(!session.disconnect().success && session.connected() &&
                    sdk.logouts == 1, "failed logout retains transport");
            require(!session.connect(request,"fixture","password").success &&
                    transport.opens == 1, "cannot duplicate pending connection");
            require(session.disconnect().success && !session.connected() &&
                    sdk.logouts == 2, "retry logout succeeds and closes");
            rejects([] {
                static_cast<void>(CloudStreamTunnel::create(nullptr));
            });
            auto bridge = CloudStreamTunnel::create(
                std::make_unique<TestOnlyEchoStream>());
            require(bridge->endpoint().local_port > 0 &&
                    bridge->endpoint().observed_route == CloudRoute::Unknown,
                    "no fake direct or relay mode");
            bridge->close();
            bool expired = false;
            try { static_cast<void>(bridge->endpoint()); }
            catch (const std::runtime_error&) { expired = true; }
            require(expired, "closed tunnel must not publish stale port");
            break;
        }

        case 19: {
            // Post-auth LMID reorder fixture, NOT live Cloud.
            PtcpReceiveWindow reorder(100);
            PtcpPacket p{};
            p.kind = PtcpKind::Payload;
            p.local_message_id = 102;
            p.body = "third";
            auto queued = reorder.ingest(p);
            require(queued.accepted() && queued.ready.empty() &&
                    reorder.pendingPackets() == 1, "buffer ahead of gap");
            require(reorder.ingest(p).error == PtcpOrderError::Duplicate,
                    "same LMID deduplicated");
            p.local_message_id = 103;
            p.body = "fourth";
            require(reorder.ingest(p).accepted() && reorder.pendingPackets() == 2,
                    "multiple packets queued");
            p.local_message_id = 100;
            p.body = "first";
            auto begin = reorder.ingest(p);
            require(begin.accepted() && begin.ready.size() == 1 &&
                    begin.ready.front().body == "first" &&
                    reorder.expectedMessageId() == 101, "head delivered");
            p.kind = PtcpKind::Heartbeat;
            p.local_message_id = 101;
            p.body.clear();
            auto release = reorder.ingest(p);
            require(release.accepted() && release.ready.size() == 3 &&
                    release.ready[0].kind == PtcpKind::Heartbeat &&
                    release.ready[1].body == "third" &&
                    release.ready[2].body == "fourth" &&
                    reorder.expectedMessageId() == 104 &&
                    reorder.pendingPackets() == 0, "control LMID bridges data");
            require(reorder.ingest(p).error == PtcpOrderError::Duplicate,
                    "old messages do not replay");
            break;
        }
        case 20: {
            PtcpReceiveWindow wrap(0xfffffffeu);
            PtcpPacket p{};
            p.kind = PtcpKind::Payload;
            p.local_message_id = 0;
            p.body = "zero";
            require(wrap.ingest(p).accepted() && wrap.pendingPackets() == 1,
                    "wrap message buffered");
            p.local_message_id = 0xfffffffeu;
            p.body = "before-last";
            require(wrap.ingest(p).ready.size() == 1, "start rollover");
            p.local_message_id = 0xffffffffu;
            p.body = "last";
            auto released = wrap.ingest(p);
            require(released.accepted() && released.ready.size() == 2 &&
                    released.ready[0].body == "last" &&
                    released.ready[1].body == "zero" &&
                    wrap.expectedMessageId() == 1u, "wrap delivers in order");

            PtcpReceiveWindow conflict(50);
            p.local_message_id = 51;
            p.body = "first";
            require(conflict.ingest(p).accepted(), "pending conflict fixture");
            p.body = "changed";
            require(conflict.ingest(p).error == PtcpOrderError::ConflictingDuplicate &&
                    conflict.closed() && conflict.pendingPackets() == 0,
                    "conflicting replay fails closed");
            require(conflict.ingest(p).error == PtcpOrderError::Closed,
                    "closed window cannot resume");
            PtcpReceiveWindow gap(10);
            p.local_message_id = 43;
            require(gap.ingest(p).error == PtcpOrderError::GapTooLarge &&
                    gap.closed(), "large gap fails closed");

            PtcpReceiveWindow memory(1);
            p.body.assign(65000, 'x');
            for (std::uint32_t messageIndex = 2; messageIndex <= 5; ++messageIndex) {
                p.local_message_id = messageIndex;
                require(memory.ingest(p).accepted(), "bounded memory fixture");
            }
            p.local_message_id = 6;
            require(memory.ingest(p).error == PtcpOrderError::WindowFull &&
                    memory.closed() && memory.pendingBytes() == 0,
                    "memory limit fails closed");
            break;
        }

        case 21: {
            // Complete post-auth outbound PTCP segmentation. No Cloud I/O.
            PtcpPostAuthChannel channel(77, 100);
            const std::string data(3000, 'a');
            const auto outbound = channel.sendBytes(data, 1000);
            require(outbound.ok() && outbound.datagrams.size() == 3,
                    "multi-packet send is admitted together");
            for (std::size_t index = 0; index < outbound.datagrams.size(); ++index) {
                const auto parsed = parsePtcpPacket(outbound.datagrams[index]);
                require(parsed.ok() && parsed.packet.kind == PtcpKind::Payload &&
                        parsed.packet.realm == 77 &&
                        parsed.packet.local_message_id == index,
                        "correct outbound header and sequence");
                const std::size_t size = index == 2 ? 440u : 1280u;
                require(parsed.packet.body.size() == size, "bounded UDP payload");
            }
            require(channel.stats().session.sent_bytes == 3036 &&
                    channel.stats().session.in_flight_packets == 3,
                    "byte accounting across three PTCP payloads");
            require(!channel.sendBytes({}, 1000).ok() &&
                    !channel.sendBytes(std::string(16385, 'x'), 1000).ok(),
                    "reject empty or oversized write");
            break;
        }

        case 22: {
            PtcpPostAuthChannel channel(55, 700);
            PtcpPacket peer{};
            peer.kind = PtcpKind::Payload;
            peer.realm = 55;
            peer.local_message_id = 702;
            peer.body = "third";
            std::string wire;
            require(encodePtcpPacket(peer, wire) == PtcpError::None &&
                    channel.receiveVerified(wire).ordered_bytes.empty(),
                    "hold higher LMID payload");
            peer.kind = PtcpKind::Heartbeat;
            peer.local_message_id = 701;
            peer.body.clear();
            require(encodePtcpPacket(peer, wire) == PtcpError::None &&
                    channel.receiveVerified(wire).ordered_bytes.empty(),
                    "heartbeat must also be reordered");
            peer.kind = PtcpKind::Payload;
            peer.realm = 55;
            peer.local_message_id = 700;
            peer.body = "first";
            require(encodePtcpPacket(peer, wire) == PtcpError::None,
                    "encode final missing packet");
            const auto ordered = channel.receiveVerified(wire);
            require(ordered.ok() && ordered.ordered_bytes.size() == 2 &&
                    ordered.ordered_bytes[0] == "first" &&
                    ordered.ordered_bytes[1] == "third" &&
                    channel.stats().reorder_packets == 0,
                    "release ordered NetSDK bytes only");
            require(channel.receiveVerified(wire).error ==
                        PtcpChannelError::Duplicate,
                    "no replay of committed payload");
            break;
        }

        case 23: {
            PtcpPostAuthChannel channel(1, 0);
            const auto original = channel.sendBytes("abc", 1000);
            require(original.ok() && original.datagrams.size() == 1,
                    "send one packet");
            require(channel.retransmissions(1399).datagrams.empty(),
                    "no early retry");
            const auto firstRetry = channel.retransmissions(1400);
            require(firstRetry.ok() && firstRetry.datagrams.size() == 1 &&
                    firstRetry.datagrams.front() == original.datagrams.front(),
                    "retry exact datagram and message id");
            require(channel.retransmissions(1800).datagrams.size() == 1 &&
                    channel.retransmissions(2200).datagrams.size() == 1,
                    "bounded further retries");
            const auto expired = channel.retransmissions(2600);
            require(expired.error == PtcpChannelError::Expired &&
                    channel.closed() &&
                    channel.sendBytes("later", 2700).error ==
                        PtcpChannelError::Closed,
                    "retry exhaustion fails closed");
            break;
        }

        case 24: {
            PtcpPacket peer{};
            peer.kind = PtcpKind::Payload;
            peer.realm = 99;
            peer.local_message_id = 50;
            peer.body = "untrusted";
            std::string wire;
            require(encodePtcpPacket(peer, wire) == PtcpError::None,
                    "wrong realm fixture");
            PtcpPostAuthChannel wrongRealm(1, 50);
            require(wrongRealm.receiveVerified(wire).error ==
                        PtcpChannelError::WrongRealm && wrongRealm.closed(),
                    "isolation by negotiated realm");
            PtcpPostAuthChannel malformed(1, 0);
            require(malformed.receiveVerified("garbage").error ==
                        PtcpChannelError::InvalidDatagram &&
                    malformed.closed(), "malformed framing closes session");

            PtcpPostAuthChannel shutdownChannel(99, 50);
            peer.kind = PtcpKind::Status;
            peer.body.clear();
            peer.status = "DISC";
            require(encodePtcpPacket(peer, wire) == PtcpError::None &&
                    shutdownChannel.receiveVerified(wire).error ==
                        PtcpChannelError::RemoteClosed &&
                    shutdownChannel.closed(), "remote DISC ends channel");
            break;
        }

        case 25: {
            PtcpPostAuthChannel channel(44, 1);
            for (int messageIndex = 0; messageIndex < 32; ++messageIndex) {
                require(channel.sendBytes("x", 1000).ok(), "inflight fixture");
            }
            require(channel.stats().session.in_flight_packets == 32 &&
                    channel.sendBytes("z", 1000).error ==
                        PtcpChannelError::Backpressure &&
                    channel.stats().session.in_flight_packets == 32,
                    "whole-write backpressure without partial commit");
            PtcpPacket ack{};
            ack.kind = PtcpKind::Heartbeat;
            ack.local_message_id = 1;
            ack.received = channel.stats().session.sent_bytes;
            std::string wire;
            require(encodePtcpPacket(ack, wire) == PtcpError::None &&
                    channel.receiveVerified(wire).ok() &&
                    channel.stats().session.in_flight_packets == 0,
                    "valid peer ACK frees outgoing window");
            require(channel.sendBytes("resumed", 1001).ok(),
                    "sending resumes after ACK");
            break;
        }

        case 26: {
            PtcpPostAuthChannel channel(17, 10);
            require(channel.sendBytes("abc", 1000).ok(), "valid initial send");
            PtcpPacket peer{};
            peer.kind = PtcpKind::Heartbeat;
            peer.local_message_id = 10;
            peer.received = 1000;
            std::string forged;
            require(encodePtcpPacket(peer, forged) == PtcpError::None &&
                    channel.receiveVerified(forged).error ==
                        PtcpChannelError::AccountingFailed &&
                    channel.closed(), "forged future ACK rejected");
            PtcpPostAuthChannel fresh(17, 10);
            const auto heartbeat = fresh.heartbeat(1000);
            require(heartbeat.ok() && heartbeat.datagrams.size() == 1 &&
                    parsePtcpPacket(heartbeat.datagrams[0]).packet.kind ==
                        PtcpKind::Heartbeat &&
                    fresh.stats().session.sent_bytes == 12,
                    "explicit idle heartbeat is encoded and accounted");
            peer.received = 12;
            require(encodePtcpPacket(peer, forged) == PtcpError::None &&
                    fresh.receiveVerified(forged).ok(),
                    "peer cumulative ACK for heartbeat is accepted");
            break;
        }

        case 27: {
            // In-memory datagrams, realistic PTCP framing, no Cloud contact.
            auto mock = std::make_unique<ScriptedVerifiedPtcpLink>(true, 50);
            auto* link = mock.get();
            auto stream = PtcpVerifiedByteStream::create(std::move(mock), 77, 50);
            const std::string sent(3000, 'S');
            require(stream->write(sent) && link->outbound.size() == 3,
                    "R8 fragmented byte-stream uplink");
            std::string echoed;
            std::array<char, 1600> buffer{};
            for (int tries = 0; tries < 6 && echoed.size() < sent.size(); ++tries) {
                const int amount = stream->read(
                    buffer.data(), static_cast<int>(buffer.size()), 0);
                require(amount >= 0, "mock PTCP downstream remains alive");
                echoed.append(buffer.data(), static_cast<std::size_t>(amount));
            }
            require(echoed == sent, "R8 ordered full-duplex PTCP byte stream");
            stream->close();
            require(link->closes == 1 &&
                    stream->read(buffer.data(), 1, 0) == -1,
                    "link closed exactly once, read refuses further data");
            stream->close();
            require(link->closes == 1, "idempotent remote close");
            break;
        }
        case 28: {
            auto mock = std::make_unique<ScriptedVerifiedPtcpLink>();
            auto* link = mock.get();
            auto stream = PtcpVerifiedByteStream::create(std::move(mock), 7, 40);
            PtcpPacket peer{};
            peer.kind = PtcpKind::Payload;
            peer.realm = 7;
            for (std::uint32_t peerId : {42u, 41u, 40u}) {
                peer.local_message_id = peerId;
                peer.body = std::to_string(peerId) + ",";
                std::string encoded;
                require(encodePtcpPacket(peer, encoded) == PtcpError::None,
                        "create reverse-order fixture");
                link->inbound.push_back(std::move(encoded));
            }
            std::string output;
            std::array<char, 64> buffer{};
            for (int step = 0; step < 3; ++step) {
                const int n = stream->read(
                    buffer.data(), static_cast<int>(buffer.size()), 0);
                require(n >= 0, "out-of-order buffer is nonfatal");
                output.append(buffer.data(), static_cast<std::size_t>(n));
            }
            require(output == "40,41,42,", "NetSDK bytes never reordered");
            // Old packet is ignored; no duplicated bytes delivered.
            peer.local_message_id = 40;
            peer.body = "40,";
            std::string repeated;
            require(encodePtcpPacket(peer, repeated) == PtcpError::None,
                    "encode old packet");
            link->inbound.push_back(repeated);
            require(stream->read(buffer.data(), 64, 0) == 0,
                    "old packet not replayed into NetSDK stream");
            break;
        }
        case 29: {
            rejects([] {
                static_cast<void>(PtcpVerifiedByteStream::create(nullptr, 1, 0));
            });
            auto mock = std::make_unique<ScriptedVerifiedPtcpLink>();
            auto* link = mock.get();
            mock->fail_at_write = 1;
            auto stream = PtcpVerifiedByteStream::create(std::move(mock), 6, 9);
            require(!stream->write(std::string(3000, 'x')) &&
                    link->outbound.size() == 1 &&
                    link->closes == 1, "partial datagram transmission fails closed");
            std::array<char, 20> buffer{};
            require(stream->read(buffer.data(), 20, 0) == -1 &&
                    !stream->write("more"), "no silent continuation after loss");
            break;
        }
        case 30: {
            auto mock = std::make_unique<ScriptedVerifiedPtcpLink>();
            auto* link = mock.get();
            auto stream = PtcpVerifiedByteStream::create(std::move(mock), 11, 8);
            require(stream->write("hello"), "send first datagram");
            PtcpPacket peer{};
            peer.kind = PtcpKind::Heartbeat;
            peer.local_message_id = 8;
            peer.received = 9999; // ACK for bytes never sent.
            std::string forged;
            require(encodePtcpPacket(peer, forged) == PtcpError::None,
                    "construct forged ACK");
            link->inbound.push_back(forged);
            char data[32]{};
            require(stream->read(data, 32, 0) == -1 &&
                    link->closes == 1, "invalid ACK closes verified link");
            break;
        }
        case 31: {
            auto mock = std::make_unique<ScriptedVerifiedPtcpLink>();
            auto* link = mock.get();
            auto stream = PtcpVerifiedByteStream::create(std::move(mock), 2, 90);
            for (int messageIndex = 0; messageIndex < 32; ++messageIndex)
                require(stream->write("x"), "fill pending window");
            require(link->outbound.size() == 32, "window full");
            PtcpPacket ack{};
            ack.kind = PtcpKind::Heartbeat;
            ack.local_message_id = 90;
            ack.received = 32u * 13u;
            std::string wire;
            require(encodePtcpPacket(ack, wire) == PtcpError::None,
                    "ACK for entire cumulative payload window");
            link->inbound.push_back(wire);
            require(stream->write("unblocked") &&
                    link->outbound.size() == 33 &&
                    link->closes == 0, "backpressure pumps ACK and resumes writes");
            break;
        }
        case 32: {
            // Real loopback TCP crossing the R4 bridge + new R8 adapter.
            // Underlying datagrams remain a scripted LOCAL fixture.
            auto mock = std::make_unique<ScriptedVerifiedPtcpLink>(true, 100);
            auto stream = PtcpVerifiedByteStream::create(std::move(mock), 33, 100);
            auto bridge = CloudLoopbackBridge::start(std::move(stream));
            const auto port = bridge->localPort();
            require(port != 0, "ephemeral Cloud bridge port");
            SOCKET client = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            require(client != INVALID_SOCKET, "make local-only test socket");
            const DWORD deadline = 3000;
            require(::setsockopt(client, SOL_SOCKET, SO_RCVTIMEO,
                     reinterpret_cast<const char*>(&deadline),
                     static_cast<int>(sizeof(deadline))) == 0,
                    "bound socket receive deadline");
            sockaddr_in localhost{};
            localhost.sin_family = AF_INET;
            localhost.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            localhost.sin_port = htons(port);
            require(::connect(client,
                    reinterpret_cast<const sockaddr*>(&localhost),
                    static_cast<int>(sizeof(localhost))) == 0,
                    "bridge accepts loopback client");
            const std::string probe = "R8-local-bridge-datagram-byte-flow";
            require(::send(client, probe.data(),
                    static_cast<int>(probe.size()), 0) ==
                    static_cast<int>(probe.size()), "send NetSDK-shaped bytes");
            std::array<char, 256> output{};
            std::string response;
            while (response.size() < probe.size()) {
                const int count = ::recv(client, output.data(),
                    static_cast<int>(output.size()), 0);
                require(count > 0, "receive through R8 bridged mock datagrams");
                response.append(output.data(), static_cast<std::size_t>(count));
            }
            require(response == probe, "end-to-end deterministic byte ordering");
            ::closesocket(client);
            bridge->close();
            require(bridge->localPort() == 0, "bridge torn down");
            break;
        }

        case 33: {
            // Invalid arguments do not silently truncate or authorize a link.
            auto mock = std::make_unique<ScriptedVerifiedPtcpLink>();
            auto* link = mock.get();
            auto stream = PtcpVerifiedByteStream::create(std::move(mock), 3, 1);
            std::array<char, 10> output{};
            require(stream->read(nullptr, 10, 0) == -1 &&
                    stream->read(output.data(), 0, 0) == -1 &&
                    stream->read(output.data(), 10, 1001) == -1,
                    "invalid pointer, capacity and wait deadline rejected");
            require(!stream->write({}) &&
                    !stream->write(std::string(16385, 'x')) &&
                    link->outbound.empty(), "oversized TCP write not truncated");
            require(stream->write("valid-after-rejection") &&
                    link->outbound.size() == 1, "valid link not contaminated");
            break;
        }
        case 34: {
            auto mock = std::make_unique<ScriptedVerifiedPtcpLink>();
            auto* link = mock.get();
            mock->inbound.emplace_back(2049, 'X');
            auto stream = PtcpVerifiedByteStream::create(std::move(mock), 4, 10);
            std::array<char, 50> output{};
            require(stream->read(output.data(), 50, 0) == -1 &&
                    link->closes == 1, "oversized incoming datagram fails closed");
            require(!stream->write("invalid-after-overflow"),
                    "no further data accepted after upstream error");
            break;
        }

        case 35: {
            // Real OS UDP socket, no host discovery or proprietary Cloud.
            rejects([] { static_cast<void>(PinnedUdpSocket::open("", 554)); });
            rejects([] { static_cast<void>(PinnedUdpSocket::open("127.0.0.1", 0)); });
            rejects([] { static_cast<void>(PinnedUdpSocket::open("example.com", 554)); });
            rejects([] { static_cast<void>(PinnedUdpSocket::open("0.0.0.0", 554)); });
            rejects([] { static_cast<void>(PinnedUdpSocket::open("255.255.255.255", 554)); });
            rejects([] { static_cast<void>(PinnedUdpSocket::open("239.1.2.3", 554)); });
            LocalUdpTestPeer remote;
            auto socket = PinnedUdpSocket::open("127.0.0.1", remote.port());
            require(socket && socket->localPort() != 0 && !socket->closed(),
                    "configured local UDP source port");
            socket->close();
            require(socket->closed() && socket->localPort() == 0,
                    "closing releases UDP source port");
            socket->close();
            break;
        }
        case 36: {
            // Actual Windows UDP localhost datagram exchange (not P2P).
            LocalUdpTestPeer remote;
            auto socket = PinnedUdpSocket::open("127.0.0.1", remote.port());
            require(socket->sendDatagram("R9-wire-ping"), "UDP payload sent");
            require(remote.receiveOne() == "R9-wire-ping",
                    "remote UDP socket receives exact datagram");
            remote.replyToLast("R9-wire-pong");
            std::array<char, 256> response{};
            const int length = socket->receiveDatagram(
                response.data(), static_cast<int>(response.size()), 500);
            require(length == 12 &&
                    std::string_view(response.data(),
                        static_cast<std::size_t>(length)) == "R9-wire-pong",
                    "UDP reply received as a whole datagram");
            break;
        }
        case 37: {
            LocalUdpTestPeer expectedPeer;
            LocalUdpTestPeer unknownPeer;
            auto socket = PinnedUdpSocket::open("127.0.0.1", expectedPeer.port());
            unknownPeer.sendToLocalPort(socket->localPort(), "wrong-source");
            std::array<char, 200> output{};
            require(socket->receiveDatagram(
                        output.data(), static_cast<int>(output.size()), 60) == 0,
                    "kernel-connected UDP filters nonpinned source endpoint");
            require(socket->sendDatagram("legitimate"), "send to pinned peer");
            require(expectedPeer.receiveOne() == "legitimate",
                    "pinned peer receives data");
            expectedPeer.replyToLast("accepted");
            const int count = socket->receiveDatagram(
                output.data(), static_cast<int>(output.size()), 500);
            require(count == 8 &&
                    std::string_view(output.data(),
                        static_cast<std::size_t>(count)) == "accepted",
                    "pinned peer reply still works");
            break;
        }
        case 38: {
            LocalUdpTestPeer expectedPeer;
            auto socket = PinnedUdpSocket::open("127.0.0.1", expectedPeer.port());
            expectedPeer.sendToLocalPort(socket->localPort(),
                                         std::string(1600, 'x'));
            std::array<char, 32> output{};
            require(socket->receiveDatagram(
                        output.data(), static_cast<int>(output.size()), 500) == -1 &&
                    socket->closed() && socket->localPort() == 0,
                    "truncated UDP datagram fails closed");
            require(!socket->sendDatagram("must-not-send"),
                    "no transmissions after malformed datagram");
            break;
        }
        case 39: {
            LocalUdpTestPeer idlePeer;
            auto socket = PinnedUdpSocket::open("127.0.0.1", idlePeer.port());
            std::atomic<int> observed{999};
            std::thread reader([&] {
                std::array<char, 20> output{};
                observed.store(socket->receiveDatagram(
                    output.data(), static_cast<int>(output.size()), 1000));
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
            socket->close();
            reader.join();
            require(observed.load() == -1 && socket->localPort() == 0,
                    "concurrent close interrupts polling without thread kill");
            break;
        }
        case 40: {
            LocalUdpTestPeer remote;
            auto first = PinnedUdpSocket::open("127.0.0.1", remote.port());
            auto second = PinnedUdpSocket::open("127.0.0.1", remote.port());
            require(first->localPort() != 0 && second->localPort() != 0 &&
                    first->localPort() != second->localPort(),
                    "two sessions have separate OS ports");
            require(first->receiveDatagram(nullptr, 1, 0) == -1 &&
                    first->receiveDatagram(nullptr, 0, 0) == -1 &&
                    !first->sendDatagram({}) &&
                    !first->sendDatagram(std::string(2049, 'x')),
                    "invalid API inputs refused without truncation");
            require(first->sendDatagram("one"), "first session sends");
            require(remote.receiveOne() == "one", "first session received by peer");
            remote.replyToLast("alpha");
            std::array<char, 20> data{};
            const int a = first->receiveDatagram(data.data(), 20, 500);
            require(a == 5 && std::string_view(data.data(),5) == "alpha",
                    "first session receives reply");
            require(second->sendDatagram("two"), "second session sends");
            require(remote.receiveOne() == "two", "second session received");
            remote.replyToLast("beta");
            const int c = second->receiveDatagram(data.data(), 20, 500);
            require(c == 4 && std::string_view(data.data(),4) == "beta",
                    "second session independent reply");
            break;
        }

        default:
            throw std::runtime_error("unknown test");
        }

        std::cout << "PASS D-" << id << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL " << error.what() << '\n';
        return 1;
    }
}
