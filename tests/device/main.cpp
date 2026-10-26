#include <sentrium/device.hpp>
#include <sentrium/cloud.hpp>
#include <sentrium/cloud_wire.hpp>

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
