#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace sentrium::device {

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Disconnecting,
    Failed
};

std::string_view name(ConnectionState state) noexcept;

struct LoginRequest {
    std::string host;
    std::uint16_t port = 37777;
    std::string username;
    std::string password;

    void validate() const;
    std::string endpoint() const;
};

struct Channel {
    int index = 0;
    std::string name;
};

std::vector<Channel> make_channels(int count);

struct Session {
    std::int64_t value = 0;
    explicit operator bool() const noexcept { return value != 0; }
};

struct ConnectResult {
    bool success = false;
    Session session;
    int channel_count = 0;
    std::string diagnostic;
};

struct DisconnectResult {
    bool success = false;
    std::string diagnostic;
};

class Adapter {
public:
    virtual ~Adapter() = default;
    virtual ConnectResult connect(const LoginRequest& request) = 0;
    virtual DisconnectResult disconnect(Session session) = 0;
};

struct Device {
    std::string id;
    std::string display_name;
    std::string host;
    std::uint16_t port = 37777;
    std::string manufacturer = "Intelbras";
    ConnectionState state = ConnectionState::Disconnected;
    std::vector<Channel> channels;
    std::string diagnostic;
};

} // namespace sentrium::device
