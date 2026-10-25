#include <sentrium/media.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

using namespace sentrium::media;

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

Packet packet(std::uint64_t sequence, std::size_t size) {
    Packet value;
    value.sequence = sequence;
    value.data_type = 1;
    value.arrival = std::chrono::steady_clock::now();
    value.bytes.resize(size, std::byte{0x2a});
    return value;
}
}

int main(int argc, char** argv) {
    try {
        require(argc == 2, "test id required");
        const int id = std::stoi(argv[1]);

        switch (id) {
        case 1: {
            PacketQueue queue(2);
            require(queue.capacity() == 2, "capacity");
            require(!queue.try_pop().has_value(), "empty queue");
            break;
        }
        case 2: {
            bool rejected = false;
            try { PacketQueue queue(0); } catch (const std::invalid_argument&) { rejected = true; }
            require(rejected, "zero capacity rejected");
            break;
        }
        case 3: {
            PacketQueue queue(2);
            queue.push(packet(1, 10));
            queue.push(packet(2, 20));
            queue.push(packet(3, 30));
            const auto metrics = queue.metrics();
            require(metrics.accepted == 3, "accepted");
            require(metrics.dropped == 1, "drop oldest");
            require(metrics.high_water_mark == 2, "hwm");
            require(metrics.depth == 2, "depth");
            require(metrics.bytes == 60, "bytes");
            auto first = queue.try_pop();
            require(first && first->sequence == 2, "oldest dropped");
            break;
        }
        case 4: {
            PacketQueue queue(4);
            queue.push(packet(7, 32));
            auto value = queue.try_pop();
            require(value && value->bytes.size() == 32, "packet lifetime");
            require(queue.metrics().depth == 0, "depth after pop");
            break;
        }
        case 5: {
            require(StreamKind::Principal != StreamKind::Extra, "stream kinds distinct");
            break;
        }
        case 6: {
            PacketQueue queue(1);
            for (std::uint64_t i = 0; i < 100; ++i) {
                queue.push(packet(i, 1));
            }
            const auto value = queue.try_pop();
            require(value && value->sequence == 99, "latest survives");
            const auto metrics = queue.metrics();
            require(metrics.dropped == 99 && metrics.high_water_mark == 1, "bounded under load");
            break;
        }
        default:
            throw std::runtime_error("unknown test");
        }

        std::cout << "PASS M-" << id << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL " << error.what() << '\n';
        return 1;
    }
}
