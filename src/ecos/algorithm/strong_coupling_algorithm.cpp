#include "ecos/algorithm/strong_coupling_algorithm.hpp"
#include "ecos/simulation.hpp"
#include <map>
#include <string>

namespace nova_sim {

struct strong_coupling_algorithm::impl {
    double baseStepSize;
    simulation& sim;
    std::vector<model_instance*> instances;
    int maxIterations = 5; // 默认 5 次迭代

    explicit impl(double dt, simulation& s) : baseStepSize(dt), sim(s) {}
};

strong_coupling_algorithm::strong_coupling_algorithm(double baseStepSize, simulation& sim)
    : pimpl_(std::make_unique<impl>(baseStepSize, sim)) {}

void strong_coupling_algorithm::initialize(double startTime) {}

double strong_coupling_algorithm::step(double currentTime) {
    // 强耦合 Gauss-Seidel 迭代逻辑
    for (int i = 0; i < pimpl_->maxIterations; ++i) {
        for (auto* inst : pimpl_->instances) {
            // 1. 将上一次同步的输入应用到 FMU
            inst->get_properties().apply_sets();
            
            // 2. 执行步进 (FMI 3.0 或 FMI 2.0)
            // 注意：强耦合通常需要 FMU 支持 rollback 或能够多次在同一时间步执行
            // 这里我们采用最基础的 Gauss-Seidel 同步
            inst->step(currentTime, pimpl_->baseStepSize);
            
            // 3. 读取 FMU 最新输出
            inst->get_properties().apply_gets();
            
            // 4. 执行跨 Slave 数据链路同步
            pimpl_->sim.sync_links();
        }
    }
    return currentTime + pimpl_->baseStepSize;
}

void strong_coupling_algorithm::model_instance_added(model_instance* instance) {
    pimpl_->instances.push_back(instance);
}

strong_coupling_algorithm::~strong_coupling_algorithm() = default;

} // namespace nova_sim
