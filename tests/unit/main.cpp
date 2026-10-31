#include <sentrium/core.hpp>
#include <iostream>
#include <memory>
#include <type_traits>

using namespace sentrium::core;
static void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
template<class F> static void rejects(F action, Error expected) {
    try { action(); } catch (const Failure& e) { require(e.code() == expected, "wrong error code"); return; }
    throw std::runtime_error("expected rejection");
}
class FakeClock final : public Clock {
public:
    std::chrono::milliseconds ticks{0};
    std::chrono::steady_clock::time_point monotonic() const override { return std::chrono::steady_clock::time_point(ticks); }
    std::chrono::system_clock::time_point utc() const override { return std::chrono::system_clock::time_point(ticks); }
};
int main(int argc, char** argv) {
    try {
        require(argc == 2, "test id required");
        const auto id = std::stoi(argv[1]);
        FakeClock clock;
        Lifecycle lifecycle(clock);
        switch (id) {
        case 1:
            require(lifecycle.health().state == State::Starting && !lifecycle.health().ready(), "initial health");
            rejects([&] { lifecycle.stopped(); }, Error::internal);
            lifecycle.ready(); require(lifecycle.health().ready(), "ready health");
            rejects([&] { lifecycle.ready(); }, Error::internal);
            lifecycle.stopping(); require(!lifecycle.health().ready(), "admission closed");
            lifecycle.stopped(); require(lifecycle.health().state == State::Stopped && lifecycle.health().telemetry.workers == 0, "stopped health");
            break;
        case 2:
            Config{}.validate();
            for (auto config : {Config{0,3}, Config{1048577,3}, Config{1024,0}, Config{1024,5}})
                rejects([&] { config.validate(); }, Error::configuration);
            break;
        case 3:
            clock.ticks = std::chrono::milliseconds(12); lifecycle.ready();
            require(lifecycle.health().telemetry.startup_ms == 12, "startup time");
            lifecycle.stopping(); clock.ticks += std::chrono::milliseconds(7); lifecycle.stopped();
            require(lifecycle.health().telemetry.shutdown_ms == 7 && lifecycle.health().telemetry.uptime_ms == 19, "deterministic clock");
            break;
        case 4: {
            struct Resource { int& count; ~Resource() { ++count; } };
            int count = 0;
            try { auto owner = std::make_unique<Resource>(count); auto moved = std::move(owner); require(!owner && moved != nullptr, "exclusive owner"); throw Failure(Error::resource,"injected init failure"); }
            catch (const Failure&) {}
            require(count == 1, "exactly once unwinding");
            static_assert(!std::is_copy_constructible_v<Activity>);
            { Activity owned; } // destructor must cancel and join
            break;
        }
        case 5:
            for (int i = 0; i < 100; ++i) { Activity activity; activity.stop(); require(activity.completed(), "stop token observed"); activity.stop(); }
            break;
        case 6:
            lifecycle.fail(Error::resource);
            require(lifecycle.health().last_error == Error::resource && lifecycle.health().state == State::Failed
                    && lifecycle.health().telemetry.bootstrap_failures == 1, "error propagation");
            rejects([] { throw Failure(Error::filesystem,"synthetic"); }, Error::filesystem);
            break;
        case 7: {
            const auto log = event_json(clock,"Info","synthetic","sentrium-unit-tests","quote\"\n\\\t","run");
            require(log.find("1970-01-01T00:00:00Z") != std::string::npos, "UTC");
            require(log.find("quote\\\"\\u000a\\\\\\u0009") != std::string::npos, "JSON injection escaped");
            require(log.find('\n') == log.size()-1, "single log record");
            for (auto field : {"Timestamp", "Level", "EventId", "Component", "Message", "Version", "BuildId", "RunId"})
                require(log.find(field) != std::string::npos, "log schema");
            break;
        }
        case 8:
            require(product == "SENTRIUM VMS" && foundation == "Foundation-005" && version == "0.0.0-foundation005" && !build_id.empty(), "build identity");
            require(health_json(lifecycle.health()).find("\"StartupMs\":null") != std::string::npos, "unavailable metric");
            require(build_configuration == "Debug" || build_configuration == "Release" || build_configuration == "RelWithDebInfo", "configuration");
            break;
        default: throw std::runtime_error("unknown test");
        }
        std::cout << "PASS U-" << id << ' ' << version << ' ' << build_id << '\n';
        return 0;
    } catch (const std::exception& e) { std::cerr << "FAIL " << e.what() << '\n'; return 1; }
}
