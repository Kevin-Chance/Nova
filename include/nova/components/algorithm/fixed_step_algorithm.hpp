#ifndef NOVA_FIXED_STEP_ALGORITHM_HPP
#define NOVA_FIXED_STEP_ALGORITHM_HPP

#include "nova/components/algorithm/algorithm.hpp"
#include "nova/engine/model_instance.hpp"
#include <vector>
#include <memory>

namespace nova_sim
{

class fixed_step_algorithm : public algorithm
{
public:
    explicit fixed_step_algorithm(double baseStepSize, bool parallel = true);
    ~fixed_step_algorithm() override;

    void initialize(double startTime) override;
    double step(double currentTime, nova_engine& sim) override;

    void model_instance_added(model_instance* instance);

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

} // namespace nova_sim

#endif // NOVA_FIXED_STEP_ALGORITHM_HPP
