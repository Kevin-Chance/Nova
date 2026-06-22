#ifndef NOVA_FMI_NOVA_FMU_HPP
#define NOVA_FMI_NOVA_FMU_HPP

#include "model_description.hpp"
#include "nova_slave.hpp"
#include <filesystem>
#include <memory>

namespace nova_fmi {

class fmu {
public:
    virtual const model_description& get_model_description() const = 0;
    virtual std::unique_ptr<NovaSlave> new_instance(const std::string& instanceName) = 0;
    virtual ~fmu() = default;
};

std::unique_ptr<fmu> loadFmu(const std::filesystem::path& fmuPath, bool fmiLogging = true);

} // namespace nova_fmi

#endif // NOVA_FMI_NOVA_FMU_HPP
