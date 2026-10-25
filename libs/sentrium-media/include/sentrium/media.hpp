#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
#include <vector>

namespace sentrium::media {

enum class StreamKind { Principal, Extra };

struct Packet {
    std::uint32_t data_type = 0;
    std::uint64_t sequence = 0;
    std::chrono::steady_clock::time_point arrival{};
    std::vector<std::byte> bytes;
};

struct QueueMetrics {
    std::uint64_t accepted = 0;
    std::uint64_t dropped = 0;
    std::uint64_t bytes = 0;
    std::size_t depth = 0;
    std::size_t high_water_mark = 0;
};

class PacketQueue final {
public:
    explicit PacketQueue(std::size_t capacity);

    PacketQueue(const PacketQueue&) = delete;
    PacketQueue& operator=(const PacketQueue&) = delete;

    void push(Packet packet);
    std::optional<Packet> try_pop();
    QueueMetrics metrics() const;
    std::size_t capacity() const noexcept { return capacity_; }

private:
    const std::size_t capacity_;
    mutable std::mutex mutex_;
    std::deque<Packet> queue_;
    QueueMetrics metrics_;
};

} // namespace sentrium::media
