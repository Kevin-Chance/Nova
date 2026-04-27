#include "ecos/listeners/csv_writer.hpp"
#include "ecos/simulation.hpp"
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>
#include <map>

namespace nova_sim {

namespace {
    const char* separator = ", ";
}

void csv_config::register_variable(const std::string& instance, const std::string& variable) {
    variable_register.push_back({instance, variable});
}

void csv_config::register_variable(const variable_identifier& id) {
    variable_register.push_back(id);
}

bool csv_config::should_log(const std::string& inst, const std::string& var) const {
    if (variable_register.empty()) return true;
    for (const auto& id : variable_register) {
        if (id.instanceName == inst && id.variableName == var) return true;
    }
    return false;
}

csv_writer::csv_writer(const std::string& filename) {
    std::filesystem::path p(filename);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }
    file_.open(filename, std::ios::out | std::ios::trunc);
}

void csv_writer::post_init(simulation& sim) {
    if (!header_written_) {
        write_header(sim);
        header_written_ = true;
    }
    write_row(sim);
}

void csv_writer::post_step(simulation& sim) {
    if (sim.iterations() % config_.decimation_factor == 0) {
        write_row(sim);
    }
}

void csv_writer::post_terminate(simulation& sim) {
    file_.flush();
    file_.close();
}

void csv_writer::write_header(const simulation& sim) {
    file_ << "time";
    
    for (const auto& inst : sim.get_instances()) {
        const auto& name = inst->instanceName();
        const auto& props = inst->get_properties();
        
        // 1. Reals
        for (const auto& pair : props.get_reals()) {
            if (config_.should_log(name, pair.first)) 
                file_ << separator << name << "::" << pair.first << "[REAL]";
        }
        // 2. Integers
        for (const auto& pair : props.get_integers()) {
            if (config_.should_log(name, pair.first)) 
                file_ << separator << name << "::" << pair.first << "[INT]";
        }
        // 3. Booleans
        for (const auto& pair : props.get_booleans()) {
            if (config_.should_log(name, pair.first)) 
                file_ << separator << name << "::" << pair.first << "[BOOL]";
        }
        // 4. Strings
        for (const auto& pair : props.get_strings()) {
            if (config_.should_log(name, pair.first)) 
                file_ << separator << name << "::" << pair.first << "[STR]";
        }
    }
    file_ << "\n";
    file_.flush();
}

void csv_writer::write_row(const simulation& sim) {
    file_ << sim.time();
    
    for (const auto& inst : sim.get_instances()) {
        const auto& name = inst->instanceName();
        const auto& props = inst->get_properties();
        
        // 1. Reals
        for (const auto& pair : props.get_reals()) {
            if (config_.should_log(name, pair.first))
                file_ << separator << std::to_string(pair.second->get_value());
        }
        // 2. Integers
        for (const auto& pair : props.get_integers()) {
            if (config_.should_log(name, pair.first))
                file_ << separator << std::to_string(pair.second->get_value());
        }
        // 3. Booleans
        for (const auto& pair : props.get_booleans()) {
            if (config_.should_log(name, pair.first))
                file_ << separator << (pair.second->get_value() ? "1" : "0");
        }
        // 4. Strings
        for (const auto& pair : props.get_strings()) {
            if (config_.should_log(name, pair.first))
                file_ << separator << pair.second->get_value();
        }
    }
    file_ << "\n";
}

} // namespace nova_sim
