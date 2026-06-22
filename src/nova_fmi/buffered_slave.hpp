#ifndef NOVA_FMI_BUFFERED_SLAVE_HPP
#define NOVA_FMI_BUFFERED_SLAVE_HPP

#include "nova_slave.hpp"
#include <map>

namespace nova_fmi
{

class buffered_slave : public NovaSlave
{
public:
    explicit buffered_slave(std::unique_ptr<NovaSlave> slave)
        : NovaSlave(slave->instanceName, slave->md, slave->lib_), slave_(std::move(slave))
    {
        this->fmi = slave_->fmi;
        this->component_ = slave_->component_;
    }

    void receiveCachedGets() {}
    void transferCachedSets() {}

private:
    std::unique_ptr<NovaSlave> slave_;
};

} // namespace nova_fmi

#endif // NOVA_FMI_BUFFERED_SLAVE_HPP
