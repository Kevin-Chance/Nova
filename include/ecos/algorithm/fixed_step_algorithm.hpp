#ifndef ECOS_FIXED_STEP_ALGORITHM_HPP
#define ECOS_FIXED_STEP_ALGORITHM_HPP

#include "ecos/algorithm/algorithm.hpp"
#include "ecos/model_instance.hpp"
#include <vector>

namespace nova_sim
{

class fixed_step_algorithm : public algorithm
{
public:
    explicit fixed_step_algorithm(double baseStepSize);

    void initialize(double startTime) override;
    double step(double currentTime) override;

    void model_instance_added(model_instance* instance);

private:
    double baseStepSize_;
    struct instance_wrapper {
        model_instance* instance;
        int decimationFactor;
    };
    std::vector<instance_wrapper> instances_;
};

} // namespace nova_sim

#endif // ECOS_FIXED_STEP_ALGORITHM_HPP
