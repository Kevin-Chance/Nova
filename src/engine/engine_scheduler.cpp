
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

/**
 * @brief 异步运行调度器，直到满足给定条件
 * 启动一个后台线程执行调度循环（run()），当 predicate 返回 false 时停止。
 * @param predicate 返回 bool 值的判断函数，用于控制循环何时结束
 * @return 返回 std::future 对象，以便调用者可以等待异步执行结束
 */
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

/**
 * @brief 内部执行的核心调度循环
 * 
 * 此方法在一个独立线程中运行。它会负责：
 * 1. 检查暂停标志，并在暂停时进行休眠
 * 2. 测量每次仿真的经过时间，更新 wall clock
 * 3. 比较当前的实时因子 (RTF) 和目标 RTF，如果执行太快则挂起线程等待
 * 4. 驱动底层 nova_engine 执行步进 (step)
 */
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

/**
 * @brief 启动调度器
 * 如果引擎尚未初始化，则先初始化引擎，然后进入调度循环
 */
void engine_scheduler::start()
{
    if (!sim_.initialized()) {
        sim_.init();
    }
    run();
}

/**
 * @brief 停止调度器
 * 标志调度循环结束，并阻塞当前线程等待调度线程安全退出
 */
void engine_scheduler::stop()
{
    stop_ = true;
    std::lock_guard lck(m_);
    if (t_.joinable()) {
        t_.join();
    }
}
