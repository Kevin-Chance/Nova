#include "scenario.hpp"
#include "nova/components/util/nova_xml.hpp"
#include <iostream>

namespace nova_sim {

/**
 * @brief 执行所有初始化动作
 * 遍历并执行注册在场景中的初始化回调函数，然后清空列表
 */
void scenario::runInitActions() {
    for (auto& f : initActions) f();
    initActions.clear();
}

/**
 * @brief 在给定时间点应用场景动作
 * 处理到达触发时间的定时动作，并检查和执行满足条件的谓词动作
 * @param t 当前仿真时间
 */
void scenario::apply(double t) {
    // 处理定时动作 (Timed actions)
    while (!timedActionsQueue_.empty() && timedActionsQueue_.back().time_point() <= t + timedActionsQueue_.back().eps()) {
        timedActionsQueue_.back().invoke();
        discardedTimedActions.push_back(timedActionsQueue_.back());
        timedActionsQueue_.pop_back();
    }
    // 处理谓词动作 (Predicate actions)
    auto it = predicateActions.begin();
    while (it != predicateActions.end()) {
        if (it->invoke()) {
            discardedPredicateActions.push_back(*it);
            it = predicateActions.erase(it);
        } else {
            ++it;
        }
    }
}

void scenario::on_init(std::function<void()> f) { initActions.push_back(std::move(f)); }
void scenario::invoke_when(predicate_action pa) { predicateActions.push_back(std::move(pa)); }
void scenario::invoke_at(timed_action ta) { 
    timedActionsQueue_.push_back(std::move(ta));
    std::sort(timedActionsQueue_.begin(), timedActionsQueue_.end());
}

void scenario::reset() {
    timedActionsQueue_.clear();
    predicateActions.clear();
    // 如果需要，从原始列表中重新填充
}

/**
 * @brief 从XML配置文件加载场景
 * 解析给定的XML文件，并将其中定义的定时或条件动作添加到场景队列中
 * @param config XML配置文件的路径
 * @param instances 模型实例的列表，用于在动作触发时修改对应实例的变量
 */
void scenario::load(const std::filesystem::path& config, std::vector<std::unique_ptr<model_instance>>& instances) {
    if (!std::filesystem::exists(config)) {
        throw std::runtime_error("No such file: " + std::filesystem::absolute(config).string());
    }

    if (const auto ext = config.extension().string(); ext != ".xml") {
        throw std::runtime_error("Wrong config extension. Was " + ext + ", expected " + ".xml");
    }

    xml::XmlDocument doc;
    if (!doc.load_file(config.string().c_str())) {
        throw std::runtime_error("Unable to parse '" + std::filesystem::absolute(config).string() + "'");
    }

    const auto root = doc.child("nova:Scenario");
    for (const auto& action : root) {
        const auto t = action.attribute("t").as_double();
        const auto epsAttr = action.attribute("eps");
        std::optional<double> eps;
        if (epsAttr) eps = epsAttr.as_double();

        for (const auto& variable : action) {
            variable_identifier id = variable.attribute("id").as_string();

            xml::XmlNode var;
            if ((var = variable.child("nova:real"))) {
                const double value = var.attribute("value").as_double();
                invoke_at(timed_action(t, [id, value, &instances]() {
                    for (auto& inst : instances) {
                        if (inst->instanceName() == id.instanceName) {
                            auto prop = inst->get_properties().get_real_property(id.variableName);
                            if (prop) prop->set_value(value);
                            break;
                        }
                    }
                }, eps));
            } else if ((var = variable.child("nova:integer"))) {
                const int value = var.attribute("value").as_int();
                invoke_at(timed_action(t, [id, value, &instances]() {
                    for (auto& inst : instances) {
                        if (inst->instanceName() == id.instanceName) {
                            auto prop = inst->get_properties().get_int_property(id.variableName);
                            if (prop) prop->set_value(value);
                            break;
                        }
                    }
                }, eps));
            } else if ((var = variable.child("nova:boolean"))) {
                const bool value = var.attribute("value").as_bool();
                invoke_at(timed_action(t, [id, value, &instances]() {
                    for (auto& inst : instances) {
                        if (inst->instanceName() == id.instanceName) {
                            auto prop = inst->get_properties().get_bool_property(id.variableName);
                            if (prop) prop->set_value(value);
                            break;
                        }
                    }
                }, eps));
            } else if ((var = variable.child("nova:string"))) {
                const std::string value = var.attribute("value").as_string();
                invoke_at(timed_action(t, [id, value, &instances]() {
                    for (auto& inst : instances) {
                        if (inst->instanceName() == id.instanceName) {
                            auto prop = inst->get_properties().get_string_property(id.variableName);
                            if (prop) prop->set_value(value);
                            break;
                        }
                    }
                }, eps));
            }
        }
    }
}

} // namespace nova_sim
