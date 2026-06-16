#ifndef NOVA_FMI_FMI2_FMU_HPP
#define NOVA_FMI_FMI2_FMU_HPP

#include "fmilibcpp/nova_fmi_library.hpp"
#include "fmilibcpp/fmu.hpp"
#include "fmilibcpp/nova_slave.hpp"
#include "util/temp_dir.hpp"

#include <memory>

namespace nova_fmi
{

class fmi2_fmu : public fmu
{

public:
    fmi2_fmu(std::shared_ptr<NovaFmiLibrary> lib, std::unique_ptr<nova_sim::temp_dir> temp, model_description md, bool fmiLogging = true);

    [[nodiscard]] const model_description& get_model_description() const override;

    [[nodiscard]] std::unique_ptr<NovaSlave> new_instance(const std::string& instanceName) override;

private:
    std::shared_ptr<NovaFmiLibrary> lib_;
    std::unique_ptr<nova_sim::temp_dir> temp_;
    bool fmiLogging_;
    model_description md_;
};

} // namespace nova_fmi

#endif // NOVA_FMI_FMI2_FMU_HPP
