#include <sentrium/device.hpp>

#include <cstdio>
#include <stdexcept>

namespace sentrium::device {

std::string_view name(ConnectionState state) noexcept {
    switch (state) {
    case ConnectionState::Disconnected: return "Disconnected";
    case ConnectionState::Connecting: return "Connecting";
    case ConnectionState::Connected: return "Connected";
    case ConnectionState::Disconnecting: return "Disconnecting";
    case ConnectionState::Failed: return "Failed";
    }
    return "Unknown";
}

std::string_view name(AccessMethod method) noexcept {
    switch (method) {
    case AccessMethod::Lan: return "LAN";
    case AccessMethod::Cloud: return "Cloud";
    case AccessMethod::AutoRegistration: return "AutoRegistration";
    }
    return "Unknown";
}

std::string_view name(CloudRoute route) noexcept {
    switch (route) {
    case CloudRoute::Unknown: return "Unknown";
    case CloudRoute::Direct: return "Direct";
    case CloudRoute::Relay: return "Relay";
    }
    return "Unknown";
}

void LoginRequest::validate() const {
    if (host.empty() || host.size() > 253) {
        throw std::invalid_argument("Host must contain 1 to 253 characters");
    }
    for (const unsigned char c : host) {
        if (c <= 0x20 || c == 0x7f) {
            throw std::invalid_argument("Host contains whitespace or control characters");
        }
    }
    if (port == 0) {
        throw std::invalid_argument("Port must be between 1 and 65535");
    }
    if (username.empty() || username.size() > 128) {
        throw std::invalid_argument("Username must contain 1 to 128 characters");
    }
    if (password.size() > 256) {
        throw std::invalid_argument("Password exceeds the accepted in-memory bound");
    }
}

std::string LoginRequest::endpoint() const {
    validate();
    return host + ":" + std::to_string(port);
}

std::vector<Channel> make_channels(int count) {
    if (count < 0 || count > 4096) {
        throw std::invalid_argument("Channel count outside accepted bounds");
    }

    std::vector<Channel> channels;
    channels.reserve(static_cast<std::size_t>(count));
    for (int index = 0; index < count; ++index) {
        char label[32]{};
        const auto written = std::snprintf(label, sizeof(label), "Canal %02d", index + 1);
        if (written <= 0 || written >= static_cast<int>(sizeof(label))) {
            throw std::runtime_error("Channel label generation failed");
        }
        channels.push_back(Channel{index, label});
    }
    return channels;
}

} // namespace sentrium::device
