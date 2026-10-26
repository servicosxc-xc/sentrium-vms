#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <string_view>

namespace sentrium::device {

// Protocol-level parser only. It neither contacts Cloud servers nor embeds
// bootstrap identities, keys, passwords, vendor SDKs or external executables.
// The DH response format is documented by independent research in F005.
enum class CloudWireError {
    None,
    Empty,
    TooLarge,
    BadStatus,
    BadHeader,
    TooManyHeaders,
    BadBodyLength
};

struct CloudWireResponse {
    int status_code = 0;
    std::string reason;
    // Header names normalized to lowercase ASCII; duplicate keys rejected.
    std::map<std::string, std::string> headers;
    std::string body;
};

struct CloudWireResult {
    CloudWireError error = CloudWireError::Empty;
    CloudWireResponse response;

    [[nodiscard]] bool ok() const noexcept { return error == CloudWireError::None; }
};

// Strictly bounded parser for DHP2P/1.0 response packets. Accepts CRLF and
// LF line endings; body is opaque (XML decoding belongs to a separate layer).
// Never log the wire payload: it can contain Cloud IDs, tokens and addresses.
// A result with HTTP 401/403 is syntactically valid, not authenticated.
[[nodiscard]] CloudWireResult parseCloudWireResponse(std::string_view packet);

} // namespace sentrium::device
