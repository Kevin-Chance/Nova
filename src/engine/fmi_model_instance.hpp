#ifndef NOVA_FMI_MODEL_INSTANCE_HPP
#define NOVA_FMI_MODEL_INSTANCE_HPP

#include "nova_fmi/nova_slave.hpp"
#include "nova/engine/model_instance.hpp"
#include "nova/components/logger/logger.hpp"
#include <iostream>

namespace nova_sim
{

class fmi_model_instance : public model_instance
{
public:
    explicit fmi_model_instance(std::unique_ptr<nova_fmi::NovaSlave> slave, std::optional<double> stepSizeHint)
        : model_instance(slave->instanceName, stepSizeHint)
        , slave_(std::move(slave))
    {
        // Fix WEEK 2: Pull variables from move-aware source
        auto& vars = slave_->md.modelVariables;
        for (const auto& v : vars) {
            std::string propertyName = v.name;
            unsigned int vr = v.vr;
            
            if (v.is_real()) {
                properties_.add_real_property(propertyName, property_t<double>::create(
                    {slave_->instanceName, propertyName},
                    [vr, this] { return slave_->get_real(vr); },
                    [vr, this](double value) { slave_->set_real(vr, value); }));
            } else if (v.is_integer()) {
                properties_.add_int_property(propertyName, property_t<int>::create(
                    {slave_->instanceName, propertyName},
                    [vr, this] { return slave_->get_integer(vr); },
                    [vr, this](int value) { slave_->set_integer(vr, value); }));
            }
        }
    }

    void enter_initialization_mode(double start = 0) override { slave_->enter_initialization_mode(start); }
    void exit_initialization_mode() override { slave_->exit_initialization_mode(); }
    void step(double currentTime, double stepSize) override { slave_->step(currentTime, stepSize); }
    void terminate() override { slave_->terminate(); }
    void reset() override { slave_->reset(); }

private:
    std::unique_ptr<nova_fmi::NovaSlave> slave_;
};

} // namespace nova_sim

#endif // NOVA_FMI_MODEL_INSTANCE_HPP
