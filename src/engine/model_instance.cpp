
#include "nova/engine/model_instance.hpp"

#include "nova/components/logger/logger.hpp"

using namespace nova_sim;

/**
 * @brief 应用指定的参数集到当前模型实例
 * 
 * 在参数集字典中查找目标参数集，并遍历其中的所有变量覆盖其初始值。
 * 针对 Real, Integer, Boolean, String 和 Vector 等不同类型的变量进行类型匹配和写入。
 * 如果发生类型不匹配或变量不存在，会通过日志系统发出警告。
 * 
 * @param name 要应用的参数集名称
 * @return 如果成功找到并应用该参数集返回 true，如果参数集不存在则返回 false
 */
bool model_instance::apply_parameter_set(const std::string& name)
{
    if (parameterSets_.contains(name)) {

        const auto& parameters = parameterSets_.at(name);
        for (const auto& [variableName, value] : parameters) {

            switch (value.index()) {
                case 0: {
                    if (const auto p = properties_.get_real_property(variableName); !p) {
                        log::warn("No variable named '{}' of type real registered for instance '{}'", variableName, instanceName_);
                    } else {
                        p->set_value(std::get<double>(value));
                    }
                    break;
                }
                case 1: {
                    if (const auto p = properties_.get_int_property(variableName); p) {
                        p->set_value(std::get<int>(value));
                    } else {
                        if (const auto pr = properties_.get_real_property(variableName); pr) {
                            pr->set_value(std::get<int>(value));
                        } else {
                            log::warn("No variable named '{}' of type int registered for instance '{}'", variableName, instanceName_);
                        }
                    }
                    break;
                }
                case 2: {
                    if (const auto p = properties_.get_bool_property(variableName); !p) {
                        log::warn("No variable named '{}' of type bool registered for instance '{}'", variableName, instanceName_);
                    } else {
                        p->set_value(std::get<bool>(value));
                    }
                    break;
                }
                case 3: {
                    if (const auto p = properties_.get_string_property(variableName); !p) {
                        log::warn("No variable named '{}' of type string registered for instance '{}'", variableName, instanceName_);
                    } else {
                        p->set_value(std::get<std::string>(value));
                    }
                    break;
                }
                case 4: {
                    if (const auto p = properties_.get_vector_property(variableName); !p) {
                        log::warn("No variable named '{}' of type vector registered for instance '{}'", variableName, instanceName_);
                    } else {
                        p->set_value(std::get<std::vector<double>>(value));
                    }
                    break;
                }
            }
        }
        return true;
    }
    return false;
}

/**
 * @brief 添加一个新的参数集配置
 * @param name 参数集名称
 * @param parameterSet 变量名到标量值的键值对映射
 */
void model_instance::add_parameter_set(const std::string& name, const std::unordered_map<std::string, scalar_value>& parameterSet)
{
    parameterSets_.emplace(name, parameterSet);
}

void model_instance::add_parameterset_entry(const std::string& parameterSetName, const std::string& variableName, const scalar_value& value)
{
    parameterSets_[parameterSetName][variableName] = value;
}