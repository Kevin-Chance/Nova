#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/model_instance.hpp"
#include "ecos/simulation.hpp"

namespace nova_sim {

fixed_step_algorithm::fixed_step_algorithm(double baseStepSize)
    : baseStepSize_(baseStepSize) {}

void fixed_step_algorithm::initialize(double startTime) {}

double fixed_step_algorithm::step(double currentTime, simulation& sim) {
    for (auto& wrapper : instances_) {
        // 1. 同步输入到 FMU
        wrapper.instance->get_properties().apply_sets();
        
        // 2. 步进
        wrapper.instance->step(currentTime, baseStepSize_);
        
        // 3. 从 FMU 同步输出
        wrapper.instance->get_properties().apply_gets();
    }

    // Sync links after all slave steps to ensure Jacobi data propagation
    // This matches the original Ecos behavior and eliminates the 1-step lag in CSV logging
    sim.sync_links();

    // Push synced inputs to FMUs so they can be logged correctly and are ready for next step
    for (auto& wrapper : instances_) {
        wrapper.instance->get_properties().apply_sets();
    }

    return currentTime + baseStepSize_;
}

void fixed_step_algorithm::model_instance_added(model_instance* instance) {
    instances_.push_back({instance, 1});
}

} // namespace nova_sim
