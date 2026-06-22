#ifndef NOVA_FMI_MODEL_DESCRIPTION_HPP
#define NOVA_FMI_MODEL_DESCRIPTION_HPP

#include "scalar_variable.hpp"
#include <string>
#include <vector>
#include <optional>

namespace nova_fmi
{

struct default_experiment
{
    std::optional<double> startTime;
    std::optional<double> stopTime;
    std::optional<double> stepSize;
    std::optional<double> tolerance;
};

struct model_description
{
    std::string guid;
    std::string modelName;
    std::string modelIdentifier;
    std::string fmiVersion;
    std::string author;
    std::optional<std::string> description;
    std::optional<std::string> generationTool;
    std::string generationDateAndTime;
    bool canGetAndSetState = false;

    // 改为普通结构体以兼容旧代码 md.defaultExperiment.xxx 的写法
    default_experiment defaultExperiment;

    model_variables modelVariables;

    [[nodiscard]] const scalar_variable* get_variable(const std::string& name) const
    {
        for (const auto& var : modelVariables) {
            if (var.name == name) return &var;
        }
        return nullptr;
    }
};

} // namespace nova_fmi

#endif // NOVA_FMI_MODEL_DESCRIPTION_HPP
