#ifndef NOVA_FMI_MODEL_DESCRIPTION_HPP
#define NOVA_FMI_MODEL_DESCRIPTION_HPP

#include "scalar_variable.hpp"
#include <string>
#include <vector>
#include <optional>

namespace nova_fmi
{

/**
 * @brief 默认仿真实验配置
 * 取自 modelDescription.xml 中的 <DefaultExperiment> 节点
 */
struct default_experiment
{
    std::optional<double> startTime;
    std::optional<double> stopTime;
    std::optional<double> stepSize;
    std::optional<double> tolerance;
};

/**
 * @brief 完整的 FMU 模型描述元数据
 * 用于存储从 modelDescription.xml 中解析出的所有关键信息。
 */
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

    /** @brief 变量描述列表 */
    model_variables modelVariables;

    /**
     * @brief 根据变量名称查找具体的标量变量定义
     * @param name 要查找的变量名称
     * @return 查找到的标量变量指针，如果未找到则返回 nullptr
     */
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
