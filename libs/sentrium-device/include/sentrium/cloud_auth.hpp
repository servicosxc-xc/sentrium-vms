#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace sentrium::device {

// Device-owned credentials only. NO vendor-app key, bootstrap identity,
// official-client password or device-info decryption secret is embedded here.
struct CloudDeviceAuth {
    // SENSITIVE: do not print, serialize, persist or include in diagnostics.
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

} // namespace sentrium::device
