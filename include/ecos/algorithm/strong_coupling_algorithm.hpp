#ifndef ECOS_STRONG_COUPLING_ALGORITHM_HPP
#define ECOS_STRONG_COUPLING_ALGORITHM_HPP

#include "ecos/algorithm/algorithm.hpp"
#include "ecos/model_instance.hpp"
#include <vector>

namespace nova_sim
{

class simulation;

class strong_coupling_algorithm : public algorithm
{
public:
    explicit strong_coupling_algorithm(double baseStepSize, simulation& sim);
    void initialize(double startTime) override;
    double step(double currentTime) override;
    void model_instance_added(model_instance* instance);
    ~strong_coupling_algorithm() override;

private:
    struct impl;
    std::unique_ptr<impl> pimpl_;
};

} // namespace nova_sim

#endif // ECOS_STRONG_COUPLING_ALGORITHM_HPP
