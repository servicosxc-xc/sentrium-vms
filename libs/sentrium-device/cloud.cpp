#include <sentrium/cloud.hpp>

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

} // namespace sentrium::device
