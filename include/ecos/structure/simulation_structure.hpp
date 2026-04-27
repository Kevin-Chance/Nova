#ifndef NOVA_SIMULATION_STRUCTURE_HPP
#define NOVA_SIMULATION_STRUCTURE_HPP

#include "ecos/model.hpp"
#include "ecos/nova_fmu_locator.hpp"
#include "ecos/scalar.hpp"
#include "ecos/simulation.hpp"
#include "ecos/variable_identifier.hpp"

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <functional>
#include <map>

namespace nova_sim
{

// Week 4: Lightweight DataLink structure
struct DataLink {
    std::string src_instance;
    std::string src_variable;
    std::string dst_instance;
    std::string dst_variable;
    std::string type;
};

// Week 5: VariableEntry for linear search
struct VariableEntry {
    std::string name;
    std::string instance_name;
    unsigned int vr;
    std::string type;
};

class simulation_structure
{
public:
    simulation_structure();

    void add_model(const std::string& instanceName, const std::string& uri, std::optional<double> stepSizeHint = std::nullopt);
    void add_model(const std::string& instanceName, const std::filesystem::path& path, std::optional<double> stepSizeHint = std::nullopt);
    void add_model(const std::string& instanceName, std::shared_ptr<model> model, std::optional<double> stepSizeHint = std::nullopt);

    // Week 4: API maintained for compatibility but implementation using DataLink
    template<class T>
    void make_connection(variable_identifier source, variable_identifier sink, const std::optional<std::function<T(const T&)>>& modifier = std::nullopt)
    {
        std::string type;
        if constexpr (std::is_same_v<T, double>) type = "real";
        else if constexpr (std::is_same_v<T, int>) type = "int";
        else if constexpr (std::is_same_v<T, bool>) type = "bool";
        else if constexpr (std::is_same_v<T, std::string>) type = "string";
        else if constexpr (std::is_same_v<T, std::vector<double>>) type = "vector";
        
        links_.push_back({source.instanceName, source.variableName, sink.instanceName, sink.variableName, type});
    }

    void add_parameter_set(const std::string& name, const std::map<variable_identifier, scalar_value>& paramMap) {
        // Implementation for compatibility
    }

    std::unique_ptr<simulation> load(std::unique_ptr<algorithm> algorithm);

private:
    std::vector<DataLink> links_;
    std::vector<VariableEntry> variables_;
    std::unordered_map<std::string, std::pair<std::shared_ptr<model>, std::optional<double>>> models_;
};

} // namespace nova_sim

#endif // NOVA_SIMULATION_STRUCTURE_HPP
