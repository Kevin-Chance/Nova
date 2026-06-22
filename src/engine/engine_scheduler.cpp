
#include "nova/engine/engine_scheduler.hpp"
#include "nova/components/logger/logger.hpp"

using namespace nova_sim;


engine_scheduler::engine_scheduler(nova_engine& sim)
    : sim_(sim)
{ }

double engine_scheduler::real_time_factor() const
{
    return rtf_;
}

double engine_scheduler::target_real_time_factor() const
{
    return targetRtf_;
}

double engine_scheduler::wall_clock() const
{
    return wallClock_;
}

engine_scheduler& engine_scheduler::set_real_time_factor(double target)
{
    targetRtf_ = target > 0 ? target : std::numeric_limits<double>::infinity();
    return *this;
}

engine_scheduler& engine_scheduler::set_callback(const std::optional<std::function<void()>>& callback)
{
    callback_ = callback;
    return *this;
}

std::future<void> engine_scheduler::run_while(std::function<bool()> predicate)
{
    predicate_ = std::move(predicate);

    return std::async(std::launch::async, [this] {
        run();

        std::lock_guard lck(m_);
        if (t_.joinable()) {
            t_.join();
        }
    });
}

void engine_scheduler::run()
{
    t_ = std::thread([this] {
        while (!stop_) {

            if (paused_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            log::Stopwatch sw;
            if (predicate_ && !predicate_()) {
                stop_ = true;
            } else {

                if (rtf_ < targetRtf_) {
                    sim_.step();

                    if (callback_) {
                        (*callback_)();
                    }
                } else {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
                }
            }
            const double elapsed = sw.elapsed().count();

            const double t = sim_.time();
            wallClock_ += elapsed;
            rtf_ = t / wallClock_;
        }
    });
}

void engine_scheduler::start()
{
    if (!sim_.initialized()) {
        sim_.init();
    }
    run();
}

void engine_scheduler::stop()
{
    stop_ = true;
    std::lock_guard lck(m_);
    if (t_.joinable()) {
        t_.join();
    }
}
