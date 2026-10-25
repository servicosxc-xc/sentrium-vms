#include <sentrium/media.hpp>

#include <algorithm>
#include <stdexcept>

namespace sentrium::media {

PacketQueue::PacketQueue(std::size_t capacity) : capacity_(capacity) {
    if (capacity_ == 0 || capacity_ > 65536) {
        throw std::invalid_argument("packet queue capacity out of range");
    }
}

void PacketQueue::push(Packet packet) {
    std::lock_guard lock(mutex_);
    ++metrics_.accepted;
    metrics_.bytes += static_cast<std::uint64_t>(packet.bytes.size());

    if (queue_.size() == capacity_) {
        queue_.pop_front();
        ++metrics_.dropped;
    }

    queue_.push_back(std::move(packet));
    metrics_.depth = queue_.size();
    metrics_.high_water_mark = std::max(metrics_.high_water_mark, queue_.size());
}

std::optional<Packet> PacketQueue::try_pop() {
    std::lock_guard lock(mutex_);
    if (queue_.empty()) {
        metrics_.depth = 0;
        return std::nullopt;
    }

    Packet packet = std::move(queue_.front());
    queue_.pop_front();
    metrics_.depth = queue_.size();
    return packet;
}

QueueMetrics PacketQueue::metrics() const {
    std::lock_guard lock(mutex_);
    QueueMetrics copy = metrics_;
    copy.depth = queue_.size();
    return copy;
}

} // namespace sentrium::media
