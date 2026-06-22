#include "nova/components/structure/simulation_structure.hpp"
#include "nova/engine/variable_identifier.hpp"
#include <ranges>
#include <utility>
#include <stdexcept>

namespace nova_sim {

simulation_structure::simulation_structure() {}

void simulation_structure::add_model(const std::string& instanceName, const std::string& uri, std::optional<double> stepSizeHint)
{
    const auto model = NovaFmuLocator::resolve(uri);
    if (!model) throw std::runtime_error("Unable to resolve model: " + uri);
    add_model(instanceName, model, stepSizeHint);
}

void simulation_structure::add_model(const std::string& instanceName, const std::filesystem::path& path, std::optional<double> stepSizeHint)
{
    add_model(instanceName, path.string(), stepSizeHint);
}

void simulation_structure::add_model(const std::string& instanceName, std::shared_ptr<model> model, std::optional<double> stepSizeHint)
{
    if (!model) throw std::runtime_error("Attempting to pass nullptr as model!");
    if (models_.contains(instanceName)) throw std::runtime_error("A model named " + instanceName + " has already been added!");
    models_[instanceName] = {std::move(model), stepSizeHint};
}

std::unique_ptr<nova_engine> simulation_structure::load(std::unique_ptr<algorithm> algorithm)
{
    auto sim = std::make_unique<nova_engine>(std::move(algorithm));
    
    // Week 5: Clear and populate linear variable table
    variables_.clear();

    for (const auto& [name, m_pair] : models_) {
        auto inst = m_pair.first->instantiate(name, m_pair.second);
        
        // Week 5: Harvest variables from instantiated model into linear vector
        const auto& props = inst->get_properties();
        for (const auto& pair : props.get_reals()) {
            variables_.push_back({pair.first, name, 0, "real"});
        }
        for (const auto& pair : props.get_integers()) {
            variables_.push_back({pair.first, name, 0, "int"});
        }
        for (const auto& pair : props.get_booleans()) {
            variables_.push_back({pair.first, name, 0, "bool"});
        }
        for (const auto& pair : props.get_strings()) {
            variables_.push_back({pair.first, name, 0, "string"});
        }
        
        sim->add_slave(std::move(inst));
    }

    // Transfer parameter sets to instantiated models
    for (const auto& [setName, paramMap] : parameterSets_) {
        for (const auto& [identifier, value] : paramMap) {
            auto inst = sim->get_instance(identifier.instanceName);
            if (inst) {
                inst->add_parameterset_entry(setName, identifier.variableName, value);
            }
        }
    }

    // Week 4: Populate links in nova_engine
    for (const auto& link : links_) {
        NovaDataLink nl;
        nl.src_instance = link.src_instance;
        nl.src_variable = link.src_variable;
        nl.dst_instance = link.dst_instance;
        nl.dst_variable = link.dst_variable;
        nl.type = link.type;
        nl.real_modifier = link.real_modifier;
        sim->add_link(nl);
    }

    return sim;
}

} // namespace nova_sim
