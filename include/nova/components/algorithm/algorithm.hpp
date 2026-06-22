#ifndef NOVA_ALGORITHM_HPP
#define NOVA_ALGORITHM_HPP

#include <memory>

namespace nova_sim
{

class nova_engine;

class algorithm
{
public:
    virtual void initialize(double startTime) = 0;
    virtual double step(double currentTime, nova_engine& sim) = 0;
    virtual ~algorithm() = default;
};

} // namespace nova_sim

#endif // NOVA_ALGORITHM_HPP
