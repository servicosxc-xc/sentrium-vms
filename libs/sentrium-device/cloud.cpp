#include <sentrium/cloud.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace sentrium::device {

void CloudConnectRequest::validate() const {
    if (cloud_id.empty() || cloud_id.size() > 128) {
        throw std::invalid_argument("Cloud identifier must contain 1 to 128 bytes");
    }
    for (const unsigned char c : cloud_id) {
        if (c <= 0x20 || c == 0x7f) {
            throw std::invalid_argument("Cloud identifier contains whitespace or control bytes");
        }
    }
    if (service_port == 0) {
        throw std::invalid_argument("Cloud service port must be specified by the device");
    }
}

void CloudTunnelEndpoint::validate() const {
    if (local_port == 0) {
        throw std::invalid_argument("Cloud tunnel has no ready local TCP port");
    }
}

LoginRequest CloudTunnelEndpoint::netSdkLogin(
    std::string username, std::string password) const {
    validate();
    // Historical evidence: Cloud/T2U exposes an ephemeral local TCP listener.
    // NetSDK is not given the remote Cloud service port or the Cloud UUID.
    LoginRequest login{
        "127.0.0.1", local_port, std::move(username), std::move(password)};
    login.validate();
    return login;
}


CloudDeviceSession::CloudDeviceSession(CloudTransport& transport, Adapter& net_sdk) noexcept
    : transport_(transport), net_sdk_(net_sdk) {}

CloudDeviceSession::~CloudDeviceSession() noexcept {
    static_cast<void>(disconnect());
}

CloudSessionResult CloudDeviceSession::connect(
    const CloudConnectRequest& cloud,
    std::string username,
    std::string password) {
    if (session_ || tunnel_) {
        return {false, 0, CloudRoute::Unknown, "Cloud session already exists"};
    }

    std::unique_ptr<CloudTunnel> pending;
    try {
        cloud.validate();
        pending = transport_.open(cloud);
        if (!pending) {
            return {false, 0, CloudRoute::Unknown, "Native Cloud transport is unavailable"};
        }

        // The provider must have established the tunnel before returning.
        // A remote service port (e.g. one recorder used 37000) is never used
        // as the NetSDK endpoint; only the live local port is accepted.
        const CloudTunnelEndpoint endpoint = pending->endpoint();
        const auto login = endpoint.netSdkLogin(
            std::move(username), std::move(password));
        const ConnectResult connected = net_sdk_.connect(login);
        if (!connected.success || !connected.session ||
            connected.channel_count < 0 || connected.channel_count > 4096) {
            if (connected.session) {
                // A broken adapter response must not leave a login handle.
                static_cast<void>(net_sdk_.disconnect(connected.session));
            }
            pending->close();
            return {false, 0, CloudRoute::Unknown,
                    "NetSDK login over the Cloud tunnel failed"};
        }

        // Device owns its login and the tunnel. Closing an individual camera
        // stream MUST NOT destroy this CloudDeviceSession.
        session_ = connected.session;
        route_ = endpoint.observed_route;
        tunnel_ = std::move(pending);
        return {true, connected.channel_count, route_,
                "NetSDK connected through native Cloud transport"};
    } catch (const std::exception&) {
        if (pending) {
            pending->close();
        }
        // Avoid logging Cloud IDs, credentials or native provider internals.
        return {false, 0, CloudRoute::Unknown, "Cloud connection initialization failed"};
    }
}

void CloudDeviceSession::closeTunnel() noexcept {
    if (tunnel_) {
        tunnel_->close();
        tunnel_.reset();
    }
    route_ = CloudRoute::Unknown;
}

DisconnectResult CloudDeviceSession::disconnect() noexcept {
    if (session_) {
        try {
            // Never destroy a live remote transport after NetSDK reports
            // failed logout. Retain handle + tunnel so caller can retry.
            const DisconnectResult outcome = net_sdk_.disconnect(session_);
            if (!outcome.success) {
                return {false, "NetSDK logout failed; Cloud tunnel retained for retry"};
            }
        } catch (...) {
            return {false, "NetSDK logout threw; Cloud tunnel retained for retry"};
        }
        session_ = {};
    }
    closeTunnel();
    return {true, {}};
}

bool CloudDeviceSession::connected() const noexcept {
    return static_cast<bool>(session_) && static_cast<bool>(tunnel_);
}

CloudRoute CloudDeviceSession::route() const noexcept {
    return connected() ? route_ : CloudRoute::Unknown;
}

} // namespace sentrium::device
