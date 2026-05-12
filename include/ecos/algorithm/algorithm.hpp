#ifndef ECOS_ALGORITHM_HPP
#define ECOS_ALGORITHM_HPP

#include <memory>

namespace nova_sim
{

class simulation;

class algorithm
{
public:
    virtual void initialize(double startTime) = 0;
    virtual double step(double currentTime, simulation& sim) = 0;
    virtual ~algorithm() = default;
};

} // namespace nova_sim

#endif // ECOS_ALGORITHM_HPP
