#include "nova/components/recorder/csv_recorder.hpp"
#include "nova/engine/nova_engine.hpp"
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include "nova/components/util/nova_xml.hpp"

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

void csv_config::load(const std::filesystem::path& configPath) {
    if (!std::filesystem::exists(configPath)) {
        throw std::runtime_error("No such file: '" + std::filesystem::absolute(configPath).string() + "'");
    }
    if (const auto ext = configPath.extension().string(); ext != ".xml") {
        throw std::runtime_error("Wrong config extension. Was " + ext + ", expected " + ".xml");
    }
    xml::XmlDocument doc;
    if (!doc.load_file(configPath.string().c_str())) {
        throw std::runtime_error(
            "Unable to parse '" + std::filesystem::absolute(configPath).string() + "'");
    }

    const auto root = doc.child("nova:LogConfig");
    if (const auto dec = root.attribute("decimationFactor")) {
        decimation_factor = dec.as_uint();
    }

    const auto components = root.child("nova:components");
    for (const auto& instances : components) {
        const auto instanceName = instances.attribute("name").as_string();
        for (const auto& variable : instances) {
            const auto variableName = variable.attribute("name").as_string();
            register_variable({instanceName, variableName});
        }
    }
}

bool csv_config::should_log(const std::string& inst, const std::string& var) const {
    if (variable_register.empty()) return true;
    for (const auto& id : variable_register) {
        if (id.instanceName == inst && id.variableName == var) return true;
    }
    return false;
}

csv_recorder::csv_recorder(const std::string& filename) : filename_(filename) {
    std::filesystem::path p(filename);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }
    file_.open(filename, std::ios::out | std::ios::trunc);
}

csv_recorder::csv_recorder(const std::string& filename, const std::string& configPath) : csv_recorder(filename) {
    config_.load(configPath);
}

void csv_recorder::post_init(nova_engine& sim) {
    if (!header_written_) {
        write_header(sim);
        header_written_ = true;
    }
    write_row(sim);
}

void csv_recorder::post_step(nova_engine& sim) {
    if (sim.iterations() % config_.decimation_factor == 0) {
        write_row(sim);
    }
}

void csv_recorder::post_terminate(nova_engine& sim) {
    file_.flush();
    file_.close();
}

void csv_recorder::write_header(const nova_engine& sim) {
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

void csv_recorder::write_row(const nova_engine& sim) {
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
