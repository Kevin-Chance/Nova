#pragma once

#include <ecos/algorithm/algorithm.hpp>
#include <memory>
#include <vector>

namespace ecos
{

/**
 * @brief 工程级强耦合联合仿真算法（支持残差与松弛）
 */
class strong_coupling_algorithm : public algorithm
{
public:
    strong_coupling_algorithm(
        double stepSize,
        std::size_t maxIterations,
        bool parallel = false, 
        int outputHolderType = 0,
        int inputHolderType = 0);

    void model_instance_added(model_instance* instance) override;
    double step(double currentTime) override;

    ~strong_coupling_algorithm() override;

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};


} // namespace ecos
