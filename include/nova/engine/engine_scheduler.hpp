
#ifndef LIBNOVA_ENGINE_SCHEDULER_HPP
#define LIBNOVA_ENGINE_SCHEDULER_HPP

#include "nova/engine/nova_engine.hpp"

#include <atomic>
#include <functional>
#include <future>
#include <limits>
#include <optional>
#include <thread>

namespace nova_sim
{

class engine_scheduler
{

public:
    explicit engine_scheduler(nova_engine& sim);
    ~engine_scheduler() { stop(); }

    engine_scheduler(const engine_scheduler&) = delete;
    engine_scheduler(engine_scheduler&&) = delete;
    engine_scheduler& operator=(engine_scheduler&&) = delete;
    engine_scheduler& operator=(const engine_scheduler&) = delete;

    [[nodiscard]] double real_time_factor() const;

    [[nodiscard]] double target_real_time_factor() const;

    [[nodiscard]] double wall_clock() const;

    engine_scheduler& set_real_time_factor(double target);

    engine_scheduler& set_callback(const std::optional<std::function<void()>>& callback);

    std::future<void> run_while(std::function<bool()> predicate);

    void start();

    void stop();

    bool toggle_pause()
    {
        paused_ = !paused_;
        return paused_;
    }


private:
    nova_engine& sim_;

    std::thread t_;
    std::atomic<bool> stop_{false};
    std::atomic<bool> paused_{false};

    double wallClock_{};
    double targetRtf_{1.0};
    double rtf_ = -std::numeric_limits<double>::infinity();

    std::optional<std::function<void()>> callback_;
    std::function<bool()> predicate_;

    std::mutex m_;

    void run();
};

} // namespace nova_sim

#endif // LIBNOVA_ENGINE_SCHEDULER_HPP
