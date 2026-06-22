
#ifndef ECOS_FMI_MODEL_HPP
#define ECOS_FMI_MODEL_HPP

#include "fmi_model_instance.hpp"

#include "ecos/model.hpp"

#include "nova_fmi/fmu.hpp"

#include <filesystem>

namespace ecos
{

class fmi_model : public model
{

public:
    explicit fmi_model(const std::filesystem::path& fmuPath, bool fmiLogging = true)
        : fmu_(nova_fmi::loadFmu(fmuPath, fmiLogging))
    { }

    [[nodiscard]] nova_fmi::model_description get_model_description() const
    {
        return fmu_->get_model_description();
    }

    std::unique_ptr<model_instance> instantiate(const std::string& instanceName, std::optional<double> stepSizeHint) override
    {
        return std::make_unique<fmi_model_instance>(fmu_->new_instance(instanceName), stepSizeHint);
    }

private:
    std::unique_ptr<nova_fmi::fmu> fmu_;
};

} // namespace ecos

#endif // ECOS_FMI_MODEL_HPP
