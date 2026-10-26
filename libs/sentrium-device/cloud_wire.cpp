#include <sentrium/cloud_wire.hpp>

#include <charconv>
#include <string>
#include <string_view>
#include <utility>

namespace sentrium::device {
namespace {

constexpr std::size_t kMaxPacket = 65507; // maximum IPv4 UDP payload
constexpr std::size_t kMaxHeaderBytes = 8192;
constexpr std::size_t kMaxHeaders = 64;

bool isPrintableAscii(std::string_view value) noexcept {
    for (const unsigned char c : value) {
        if (c < 0x20 || c > 0x7e) return false;
    }
    return true;
}

bool isHeaderName(std::string_view name) noexcept {
    if (name.empty() || name.size() > 128) return false;
    for (const unsigned char c : name) {
        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-')) return false;
    }
    return true;
}

std::string lowerAscii(std::string_view value) {
    std::string out;
    out.reserve(value.size());
    for (const unsigned char c : value) {
        out.push_back(static_cast<char>(
            c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c));
    }
    return out;
}

std::string_view trimSpace(std::string_view value) noexcept {
    while (!value.empty() && value.front() == ' ') value.remove_prefix(1);
    while (!value.empty() && value.back() == ' ') value.remove_suffix(1);
    return value;
}

bool readLine(std::string_view packet,
              std::size_t& cursor,
              std::string_view& line) noexcept {
    if (cursor >= packet.size()) return false;
    const std::size_t end = packet.find('\n', cursor);
    if (end == std::string_view::npos) return false;
    line = packet.substr(cursor, end - cursor);
    cursor = end + 1;
    if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
    // Bare CR, control bytes and any non-ASCII header text are rejected.
    return isPrintableAscii(line);
}

CloudWireResult failure(CloudWireError reason) {
    return {reason, {}};
}

} // namespace

CloudWireResult parseCloudWireResponse(std::string_view packet) {
    if (packet.empty()) return failure(CloudWireError::Empty);
    if (packet.size() > kMaxPacket) return failure(CloudWireError::TooLarge);

    std::size_t cursor = 0;
    std::string_view line;
    if (!readLine(packet, cursor, line) || cursor > kMaxHeaderBytes) {
        return failure(CloudWireError::BadStatus);
    }

    // Official replies are DHP2P/1.0 <three-digit-code> <reason>.
    constexpr std::string_view kVersion = "DHP2P/1.0 ";
    if (!line.starts_with(kVersion)) return failure(CloudWireError::BadStatus);
    line.remove_prefix(kVersion.size());
    if (line.size() < 5 || line[3] != ' ') {
        return failure(CloudWireError::BadStatus);
    }

    int status = 0;
    const std::string_view code = line.substr(0, 3);
    const auto parsed = std::from_chars(code.data(), code.data() + code.size(), status);
    if (parsed.ec != std::errc{} || parsed.ptr != code.data() + code.size() ||
        status < 100 || status > 599 || !isPrintableAscii(line.substr(4))) {
        return failure(CloudWireError::BadStatus);
    }
    const std::string_view reason = trimSpace(line.substr(4));
    if (reason.empty()) return failure(CloudWireError::BadStatus);

    CloudWireResponse response;
    response.status_code = status;
    response.reason.assign(reason);

    std::size_t count = 0;
    bool terminated = false;
    while (cursor <= packet.size()) {
        if (cursor > kMaxHeaderBytes) return failure(CloudWireError::TooLarge);
        if (!readLine(packet, cursor, line)) return failure(CloudWireError::BadHeader);
        if (line.empty()) {
            terminated = true;
            break;
        }
        if (++count > kMaxHeaders) return failure(CloudWireError::TooManyHeaders);
        const auto colon = line.find(':');
        if (colon == std::string_view::npos || !isHeaderName(line.substr(0, colon))) {
            return failure(CloudWireError::BadHeader);
        }
        const auto value = trimSpace(line.substr(colon + 1));
        if (value.size() > 4096) return failure(CloudWireError::BadHeader);

        const bool inserted = response.headers.emplace(
            lowerAscii(line.substr(0, colon)), std::string(value)).second;
        if (!inserted) return failure(CloudWireError::BadHeader);
    }
    if (!terminated || cursor > packet.size()) return failure(CloudWireError::BadHeader);

    const std::string_view body = packet.substr(cursor);
    if (auto it = response.headers.find("content-length"); it != response.headers.end()) {
        std::size_t specified = 0;
        const std::string& value = it->second;
        const auto parsedLength = std::from_chars(
            value.data(), value.data() + value.size(), specified);
        if (value.empty() || parsedLength.ec != std::errc{} ||
            parsedLength.ptr != value.data() + value.size() || specified != body.size()) {
            return failure(CloudWireError::BadBodyLength);
        }
    }
    response.body.assign(body);
    return {CloudWireError::None, std::move(response)};
}

} // namespace sentrium::device
