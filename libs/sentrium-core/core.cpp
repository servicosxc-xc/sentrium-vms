#include <sentrium/core.hpp>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace sentrium::core {
std::string_view name(State s) noexcept {
    switch (s) {
    case State::Starting: return "Starting";
    case State::Ready: return "Ready";
    case State::Stopping: return "Stopping";
    case State::Stopped: return "Stopped";
    case State::Failed: return "Failed";
    }
    return "Failed";
}
void Config::validate() const {
    if (log_bytes < 1024 || log_bytes > 1048576 || log_files < 1 || log_files > 4)
        throw Failure(Error::configuration, "Invalid log size or rotation count");
}
std::chrono::steady_clock::time_point SystemClock::monotonic() const { return std::chrono::steady_clock::now(); }
std::chrono::system_clock::time_point SystemClock::utc() const { return std::chrono::system_clock::now(); }
bool Health::ready() const noexcept { return state == State::Ready && dataroot_ready && resources_ready && !last_error; }
static double elapsed(std::chrono::steady_clock::time_point a, std::chrono::steady_clock::time_point b) {
    return std::chrono::duration<double, std::milli>(b-a).count();
}
Lifecycle::Lifecycle(const Clock& clock) : clock_(clock), start_(clock.monotonic()), stop_(start_) {}
void Lifecycle::ready() {
    if (health_.state != State::Starting) throw Failure(Error::internal, "Invalid ready transition");
    health_.state = State::Ready;
    health_.dataroot_ready = health_.resources_ready = true;
    health_.telemetry.workers = 1;
    health_.telemetry.startup_ms = elapsed(start_, clock_.monotonic());
}
void Lifecycle::stopping() {
    if (health_.state != State::Ready) throw Failure(Error::internal, "Invalid stopping transition");
    health_.state = State::Stopping;
    stop_ = clock_.monotonic();
}
void Lifecycle::stopped() {
    if (health_.state != State::Stopping) throw Failure(Error::internal, "Invalid stopped transition");
    health_.state = State::Stopped;
    health_.resources_ready = false;
    health_.telemetry.workers = 0;
    health_.telemetry.shutdown_ms = elapsed(stop_, clock_.monotonic());
}
void Lifecycle::fail(Error e) {
    if (health_.state == State::Starting) ++health_.telemetry.bootstrap_failures;
    health_.state = State::Failed;
    health_.last_error = e;
    health_.resources_ready = false;
    health_.telemetry.workers = 0;
}
Health Lifecycle::health() const {
    auto result = health_;
    result.telemetry.uptime_ms = elapsed(start_, clock_.monotonic());
    return result;
}
Activity::Activity() : thread_([this](std::stop_token token) {
    std::unique_lock lock(mutex_);
    started_.count_down();
    wake_.wait(lock, token, [] { return false; });
    completed_ = token.stop_requested();
}) { started_.wait(); }
Activity::~Activity() { stop(); }
void Activity::stop() noexcept {
    if (thread_.joinable()) { thread_.request_stop(); thread_.join(); }
}
std::string json_string(std::string_view s) {
    static constexpr char digits[] = "0123456789abcdef";
    std::string out = "\"";
    for (unsigned char c : s) {
        if (c == '"' || c == '\\') { out += '\\'; out += static_cast<char>(c); }
        else if (c < 32 || c >= 127) {
            out += "\\u00"; out += digits[c >> 4]; out += digits[c & 15];
        } else out += static_cast<char>(c);
    }
    return out + '"';
}
std::string event_json(const Clock& clock, std::string_view level, std::string_view event,
                       std::string_view component, std::string_view message, std::string_view run) {
    const auto time = std::chrono::system_clock::to_time_t(clock.utc());
    std::tm utc{};
    if (gmtime_s(&utc, &time) != 0) throw Failure(Error::internal, "UTC conversion failed");
    std::ostringstream stamp;
    stamp << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return "{\"Timestamp\":" + json_string(stamp.str()) + ",\"Level\":" + json_string(level)
        + ",\"EventId\":" + json_string(event) + ",\"Component\":" + json_string(component)
        + ",\"Message\":" + json_string(message) + ",\"Version\":" + json_string(version)
        + ",\"BuildId\":" + json_string(build_id) + ",\"RunId\":" + json_string(run) + "}\n";
}
std::string health_json(const Health& h) {
    const auto optional_number = [](const std::optional<double>& v) { return v ? std::to_string(*v) : "null"; };
    return "{\"Product\":" + json_string(product) + ",\"Version\":" + json_string(version)
        + ",\"BuildId\":" + json_string(build_id) + ",\"Configuration\":" + json_string(build_configuration)
        + ",\"Platform\":\"x64\",\"State\":" + json_string(name(h.state))
        + ",\"DataRootReady\":" + (h.dataroot_ready ? "true" : "false")
        + ",\"ResourcesReady\":" + (h.resources_ready ? "true" : "false")
        + ",\"UptimeMs\":" + std::to_string(h.telemetry.uptime_ms)
        + ",\"StartupMs\":" + optional_number(h.telemetry.startup_ms)
        + ",\"ShutdownMs\":" + optional_number(h.telemetry.shutdown_ms)
        + ",\"Workers\":" + std::to_string(h.telemetry.workers)
        + ",\"BootstrapFailures\":" + std::to_string(h.telemetry.bootstrap_failures)
        + ",\"LastError\":" + (h.last_error ? std::to_string(static_cast<int>(*h.last_error)) : "null") + "}\n";
}
}
