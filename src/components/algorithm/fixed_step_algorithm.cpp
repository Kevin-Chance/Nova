#include "nova/components/algorithm/fixed_step_algorithm.hpp"
#include "nova/engine/model_instance.hpp"
#include "nova/engine/nova_engine.hpp"
#include "nova/components/logger/logger.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

#if __has_include(<execution>)
#include <execution>
#define HAS_EXECUTION
#endif

namespace nova_sim {

namespace {

struct instance_wrapper {
    int decimationFactor;
    model_instance* instance;
};

int calculateDecimationFactor(const model_instance& m, double baseStepSize) {
    constexpr double EPS = 1e-3;
    const auto& stepSizeHint = m.stepSizeHint();
    if (!stepSizeHint) return 1;

    const int decimationFactor = std::max(1, static_cast<int>(std::ceil(*stepSizeHint / baseStepSize)));
    const double actualStepSize = baseStepSize * decimationFactor;
    const double diff = std::fabs(actualStepSize - *stepSizeHint);
    if (diff >= EPS) {
        // 使用 nova_sim 的 logger
        log::warn("Actual stepSize for {} will be {} rather than requested value {}", m.instanceName(), actualStepSize, *stepSizeHint);
    }
    return decimationFactor;
}

bool should_step(size_t step, int factor) {
    return step % factor == 0;
}

} // namespace

class fixed_step_algorithm::impl {
public:
    impl(double stepSize, bool parallel)
        : parallel_(parallel), stepSize_(stepSize), stepNumber_(0) {}

    void model_instance_added(model_instance* instance) {
        const int decimationFactor = calculateDecimationFactor(*instance, stepSize_);
        instances_.emplace_back(instance_wrapper{decimationFactor, instance});
    }

    double step(double currentTime, nova_engine& sim) {
        auto f = [currentTime, this](instance_wrapper& wrapper) {
            if (should_step(stepNumber_, wrapper.decimationFactor)) {
                // 1. 同步输入到 FMU
                wrapper.instance->get_properties().apply_sets();
                // 2. 步进
                wrapper.instance->step(currentTime, stepSize_);
                // 3. 从 FMU 同步输出
                wrapper.instance->get_properties().apply_gets();
            }
        };

#if defined(HAS_EXECUTION)
        if (parallel_) {
            std::for_each(std::execution::par, instances_.begin(), instances_.end(), f);
        } else {
            std::for_each(instances_.begin(), instances_.end(), f);
        }
#else
        for (auto& wrapper : instances_) {
            f(wrapper);
        }
#endif

        // 解决 1-step 延迟：在所有实例步进后同步 link
        sim.sync_links();

        // 将同步后的结果推送到受影响的 FMU，确保日志和下一步的初始值正确
        auto post_sync = [this](instance_wrapper& wrapper) {
            if (should_step(stepNumber_, wrapper.decimationFactor)) {
                wrapper.instance->get_properties().apply_sets();
            }
        };

#if defined(HAS_EXECUTION)
        if (parallel_) {
            std::for_each(std::execution::par, instances_.begin(), instances_.end(), post_sync);
        } else {
            std::for_each(instances_.begin(), instances_.end(), post_sync);
        }
#else
        for (auto& wrapper : instances_) {
            post_sync(wrapper);
        }
#endif

        ++stepNumber_;
        return currentTime + stepSize_;
    }

private:
    bool parallel_;
    double stepSize_;
    size_t stepNumber_;
    std::vector<instance_wrapper> instances_;
};

fixed_step_algorithm::fixed_step_algorithm(double baseStepSize, bool parallel)
    : pimpl_(std::make_unique<impl>(baseStepSize, parallel)) {}

fixed_step_algorithm::~fixed_step_algorithm() = default;

void fixed_step_algorithm::initialize(double startTime) {}

double fixed_step_algorithm::step(double currentTime, nova_engine& sim) {
    return pimpl_->step(currentTime, sim);
}

void fixed_step_algorithm::model_instance_added(model_instance* instance) {
    pimpl_->model_instance_added(instance);
}

} // namespace nova_sim
