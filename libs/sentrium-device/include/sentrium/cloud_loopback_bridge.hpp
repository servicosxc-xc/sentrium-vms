#pragma once
#include <cstdint>
#include <memory>
#include <string_view>
namespace sentrium::device {
// Implemented by an independently authorized and authenticated transport.
// read(): >0 bytes, 0 timeout, <0 closed/error. close() must interrupt read.
// write(true) must accept all bytes in order; no partial-success allowed.
// Raw bytes may contain NetSDK credentials. Never persist or log payloads.
class CloudByteStream {
public:
    virtual ~CloudByteStream() = default;
    virtual int read(char* buffer, int capacity, int timeout_ms) noexcept = 0;
    virtual bool write(std::string_view bytes) noexcept = 0;
    virtual void close() noexcept = 0;
};
// Only a local 127.0.0.1 dynamic TCP bridge for a previously authenticated
// byte stream. Not rendezvous, bootstrap, Direct/Relay or CloudTransport.
// At most ONE NetSDK TCP client per instance. Close after NetSDK logout.
class CloudLoopbackBridge final {
public:
    static std::unique_ptr<CloudLoopbackBridge> start(
        std::unique_ptr<CloudByteStream> authenticated_stream);
    ~CloudLoopbackBridge() noexcept;
    CloudLoopbackBridge(const CloudLoopbackBridge&) = delete;
    CloudLoopbackBridge& operator=(const CloudLoopbackBridge&) = delete;
    [[nodiscard]] std::uint16_t localPort() const noexcept;
    void close() noexcept;
private:
    struct Impl;
    explicit CloudLoopbackBridge(std::unique_ptr<Impl> impl) noexcept;
    std::unique_ptr<Impl> impl_;
};
} // namespace sentrium::device
