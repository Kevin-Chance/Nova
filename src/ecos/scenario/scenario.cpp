#include "ecos/scenario/scenario.hpp"
#include <pugixml.hpp>
#include <iostream>

namespace nova_sim {

void scenario::runInitActions() {
    for (auto& f : initActions) f();
    initActions.clear();
}

void scenario::apply(double t) {
    // Process timed actions
    while (!timedActionsQueue_.empty() && timedActionsQueue_.back().time_point() <= t + timedActionsQueue_.back().eps()) {
        timedActionsQueue_.back().invoke();
        discardedTimedActions.push_back(timedActionsQueue_.back());
        timedActionsQueue_.pop_back();
    }
    // Process predicate actions
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
    // Re-fill if needed from original lists
}

void scenario::load(const std::filesystem::path& config, std::vector<std::unique_ptr<model_instance>>& instances) {
    pugi::xml_document doc;
    if (!doc.load_file(config.c_str())) return;

    for (auto action : doc.child("ScenarioConfig").children("Action")) {
        double t = action.attribute("t").as_double();
        for (auto var : action.children("Variable")) {
            std::string instName = var.attribute("instance").as_string();
            std::string varName = var.attribute("name").as_string();
            double value = var.attribute("value").as_double();

            // Week 6: Linear search for instance intervention
            invoke_at(timed_action(t, [instName, varName, value, &instances]() {
                for (auto& inst : instances) {
                    if (inst->instanceName() == instName) {
                        auto prop = inst->get_properties().get_real_property(varName);
                        if (prop) prop->set_value(value);
                        break;
                    }
                }
            }));
        }
    }
}

} // namespace nova_sim
