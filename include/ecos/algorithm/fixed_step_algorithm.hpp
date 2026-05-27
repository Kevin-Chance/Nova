#ifndef ECOS_FIXED_STEP_ALGORITHM_HPP
#define ECOS_FIXED_STEP_ALGORITHM_HPP

#include "ecos/algorithm/algorithm.hpp"
#include "ecos/model_instance.hpp"
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
    double step(double currentTime, simulation& sim) override;

    void model_instance_added(model_instance* instance);

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

} // namespace nova_sim

#endif // ECOS_FIXED_STEP_ALGORITHM_HPP
