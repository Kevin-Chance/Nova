#ifndef NOVA_FMI_MODEL_HPP
#define NOVA_FMI_MODEL_HPP

#include "engine/fmi_model_instance.hpp"
#include "nova/engine/model.hpp"
#include "nova_fmi/fmu.hpp"
#include <filesystem>
#include <stdexcept>

namespace nova_sim
{

class fmi_model : public model
{
public:
    explicit fmi_model(const std::filesystem::path& fmuPath, bool fmiLogging = true)
        : fmu_(nova_fmi::loadFmu(fmuPath, fmiLogging))
    { 
        if (!fmu_) {
            throw std::runtime_error("Failed to load FMU at: " + fmuPath.string());
        }
    }

    [[nodiscard]] const nova_fmi::model_description& get_model_description() const override
    {
        return fmu_->get_model_description();
    }

    std::unique_ptr<model_instance> instantiate(const std::string& instanceName, std::optional<double> stepSizeHint) override
    {
        auto slave = fmu_->new_instance(instanceName);
        if (!slave) return nullptr;
        return std::make_unique<fmi_model_instance>(std::move(slave), stepSizeHint);
    }

private:
    std::unique_ptr<nova_fmi::fmu> fmu_;
};

} // namespace nova_sim

#endif // NOVA_FMI_MODEL_HPP
