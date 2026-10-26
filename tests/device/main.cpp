#include <sentrium/device.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

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
