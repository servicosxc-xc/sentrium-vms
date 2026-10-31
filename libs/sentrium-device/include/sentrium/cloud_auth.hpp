#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace sentrium::device {

// Device-owned credentials only. NO vendor-app key, bootstrap identity,
// official-client password or device-info decryption secret is embedded here.
struct CloudDeviceAuth {
    // SENSITIVE: do not print, serialize, persist or include in diagnostics.
    CloudDeviceAuth() = default;
    CloudDeviceAuth(const CloudDeviceAuth&) = delete;
    CloudDeviceAuth& operator=(const CloudDeviceAuth&) = delete;
    CloudDeviceAuth(CloudDeviceAuth&&) noexcept = default;
    CloudDeviceAuth& operator=(CloudDeviceAuth&&) noexcept = default;
    ~CloudDeviceAuth() noexcept;

    std::string device_key_hex;
    std::string signature_base64;
    std::array<std::uint8_t,32> address_key{};
};

// Auth material for a device-owned channel, not authorization to the Cloud
// service itself. Signs nonce(decimal) || unixTime(decimal) || opaque payload.
// Uses Windows CNG; MD5 is retained solely for documented device interop.
[[nodiscard]] CloudDeviceAuth prepareCloudDeviceAuth(
    std::string_view username,
    std::string_view password,
    std::string_view device_randsalt,
    std::uint32_t nonce,
    std::uint64_t unix_seconds,
    std::string_view payload);

// AES-256-OFB transform for the P2P device address field. The protocol's IV
// must come from a verified protocol configuration; no app secret is shipped.
// Applying this transform twice with the same key/IV recovers plaintext.
// NOT authenticated encryption: callers must validate the signed envelope.
[[nodiscard]] std::vector<std::uint8_t> cryptCloudAddress(
    const std::array<std::uint8_t,32>& derived_key,
    const std::array<std::uint8_t,16>& iv,
    std::string_view input);

} // namespace sentrium::device
