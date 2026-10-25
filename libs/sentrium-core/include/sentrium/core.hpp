#pragma once
#include <sentrium/build.hpp>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <latch>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

namespace sentrium::core {
enum class State { Starting, Ready, Stopping, Stopped, Failed };
std::string_view name(State state) noexcept;
enum class Error { configuration = 2, filesystem = 3, resource = 4, internal = 5 };
class Failure final : public std::runtime_error {
public:
    Failure(Error code, const std::string& message) : std::runtime_error(message), code_(code) {}
    Error code() const noexcept { return code_; }
private:
    Error code_;
};
struct Config {
    std::size_t log_bytes = 16384;
    std::size_t log_files = 3;
    void validate() const;
};
class Clock {
public:
    virtual ~Clock() = default;
    virtual std::chrono::steady_clock::time_point monotonic() const = 0;
    virtual std::chrono::system_clock::time_point utc() const = 0;
};
class SystemClock final : public Clock {
public:
    std::chrono::steady_clock::time_point monotonic() const override;
    std::chrono::system_clock::time_point utc() const override;
};
struct Telemetry {
    double uptime_ms = 0;
    std::optional<double> startup_ms;
    std::optional<double> shutdown_ms;
    unsigned workers = 0;
    unsigned bootstrap_failures = 0;
};
struct Health {
    State state = State::Starting;
    bool dataroot_ready = false;
    bool resources_ready = false;
    Telemetry telemetry;
    std::optional<Error> last_error;
    bool ready() const noexcept;
};
class Lifecycle final {
public:
    explicit Lifecycle(const Clock& clock);
    void ready();
    void stopping();
    void stopped();
    void fail(Error error);
    Health health() const;
private:
    const Clock& clock_;
    std::chrono::steady_clock::time_point start_;
    std::chrono::steady_clock::time_point stop_;
    Health health_;
};
// One owned cooperative activity. The latch proves initialization, and join
// proves cancellation observation and completion without timing assumptions.
class Activity final {
public:
    Activity();
    ~Activity();
    Activity(const Activity&) = delete;
    Activity& operator=(const Activity&) = delete;
    void stop() noexcept;
    bool completed() const noexcept { return completed_; } // only after join
private:
    std::mutex mutex_;
    std::condition_variable_any wake_;
    std::latch started_{1};
    bool completed_ = false;
    std::jthread thread_;
};
std::string json_string(std::string_view value);
std::string event_json(const Clock& clock, std::string_view level, std::string_view event,
                       std::string_view component, std::string_view message, std::string_view run);
std::string health_json(const Health& health);
class LogSink {
public:
    virtual ~LogSink() = default;
    virtual void write(std::string_view record) = 0;
    virtual void finish() = 0;
};
}
