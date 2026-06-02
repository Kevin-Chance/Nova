#define NOVA_EXPORT_DLL
#include "ecos/nova_ecos.h"
#include "ecos/structure/simulation_structure.hpp"
#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/lib_info.hpp"
#include "ecos/util/plotter.hpp"
#include "ecos/ssp/ssp_loader.hpp"
#include "ecos/simulation_runner.hpp"
#include <memory>
#include <map>
#include <string>

using namespace nova_sim;

struct nova_simulation_structure_t {
    std::unique_ptr<simulation_structure> ss;
};

struct nova_simulation_t {
    std::unique_ptr<simulation> sim;
};

struct nova_parameter_set_t {
    std::map<variable_identifier, scalar_value> params;
};

struct nova_csv_writer_t {
    std::shared_ptr<csv_writer> writer;
};

struct nova_simulation_runner_t {
    std::unique_ptr<simulation_runner> runner;
};

extern "C" NOVA_API nova_simulation_structure_t* nova_simulation_structure_create() {
    auto ss = new nova_simulation_structure_t();
    ss->ss = std::make_unique<simulation_structure>();
    return ss;
}

extern "C" NOVA_API nova_simulation_structure_t* nova_simulation_structure_load_ssp(const char* ssp_path) {
    try {
        auto ss_ptr = load_ssp(std::filesystem::path(ssp_path));
        if (!ss_ptr) return nullptr;
        auto ss = new nova_simulation_structure_t();
        ss->ss = std::move(ss_ptr);
        return ss;
    } catch (...) {
        return nullptr;
    }
}

extern "C" NOVA_API void nova_simulation_structure_destroy(nova_simulation_structure_t* ss) {
    delete ss;
}

extern "C" NOVA_API bool nova_simulation_structure_add_model(nova_simulation_structure_t* ss, const char* name, const char* uri) {
    try {
        ss->ss->add_model(name, std::string(uri), std::nullopt);
        return true;
    } catch (...) {
        return false;
    }
}

extern "C" NOVA_API void nova_simulation_structure_make_connection(nova_simulation_structure_t* ss, const char* src_inst, const char* src_var, const char* dst_inst, const char* dst_var, const char* type) {
    if (std::string(type) == "real")
        ss->ss->make_connection<double>(variable_identifier(src_inst, src_var), variable_identifier(dst_inst, dst_var));
    else if (std::string(type) == "int")
        ss->ss->make_connection<int>(variable_identifier(src_inst, src_var), variable_identifier(dst_inst, dst_var));
    else if (std::string(type) == "bool")
        ss->ss->make_connection<bool>(variable_identifier(src_inst, src_var), variable_identifier(dst_inst, dst_var));
}

extern "C" NOVA_API void nova_simulation_structure_make_real_connection_mod(nova_simulation_structure_t* ss, const char* src, const char* dst, nova_real_modifier_t modifier) {
    std::function<double(const double&)> mod_fn;
    if (modifier) mod_fn = modifier;
    ss->ss->make_connection<double>(src, dst, modifier ? std::make_optional(mod_fn) : std::nullopt);
}

extern "C" NOVA_API void nova_simulation_structure_make_real_connection(nova_simulation_structure_t* ss, const char* src, const char* dst) {
    nova_simulation_structure_make_real_connection_mod(ss, src, dst, nullptr);
}

extern "C" NOVA_API nova_parameter_set_t* nova_parameter_set_create() {
    return new nova_parameter_set_t();
}

extern "C" NOVA_API void nova_parameter_set_add_real(nova_parameter_set_t* pps, const char* name, double value) {
    pps->params[variable_identifier(name)] = value;
}

extern "C" NOVA_API void nova_parameter_set_add_int(nova_parameter_set_t* pps, const char* name, int value) {
    pps->params[variable_identifier(name)] = value;
}

extern "C" NOVA_API void nova_parameter_set_add_bool(nova_parameter_set_t* pps, const char* name, bool value) {
    pps->params[variable_identifier(name)] = value;
}

extern "C" NOVA_API void nova_parameter_set_add_string(nova_parameter_set_t* pps, const char* name, const char* value) {
    pps->params[variable_identifier(name)] = std::string(value);
}

extern "C" NOVA_API void nova_parameter_set_destroy(nova_parameter_set_t* pps) {
    delete pps;
}

extern "C" NOVA_API void nova_simulation_structure_add_parameter_set(nova_simulation_structure_t* ss, const char* name, nova_parameter_set_t* pps) {
    ss->ss->add_parameter_set(std::string(name), pps->params);
}

extern "C" NOVA_API nova_simulation_t* nova_simulation_create(nova_simulation_structure_t* ss, double step_size) {
    auto algo = std::make_unique<fixed_step_algorithm>(step_size);
    auto sim = new nova_simulation_t();
    sim->sim = ss->ss->load(std::move(algo));
    return sim;
}

extern "C" NOVA_API void nova_simulation_destroy(nova_simulation_t* sim) {
    delete sim;
}

extern "C" NOVA_API bool nova_simulation_load_scenario(nova_simulation_t* sim, const char* scenario_file) {
    try {
        sim->sim->load_scenario(std::filesystem::path(scenario_file));
        return true;
    } catch (...) {
        return false;
    }
}

extern "C" NOVA_API bool nova_simulation_init(nova_simulation_t* sim, double start_time, const char* parameter_set) {
    try {
        if (parameter_set && std::string(parameter_set) != "") {
            sim->sim->init(start_time, std::string(parameter_set));
        } else {
            sim->sim->init(start_time, std::nullopt);
        }
        return true;
    } catch (...) {
        return false;
    }
}

extern "C" NOVA_API double nova_simulation_step(nova_simulation_t* sim, unsigned int num_steps) {
    return sim->sim->step(num_steps);
}

extern "C" NOVA_API void nova_simulation_step_until(nova_simulation_t* sim, double time_point) {
    sim->sim->step_until(time_point);
}

extern "C" NOVA_API void nova_simulation_step_for(nova_simulation_t* sim, double duration) {
    sim->sim->step_for(duration);
}

extern "C" NOVA_API void nova_simulation_terminate(nova_simulation_t* sim) {
    sim->sim->terminate();
}

extern "C" NOVA_API bool nova_simulation_get_real(nova_simulation_t* sim, const char* instance, const char* variable, double* value) {
    auto prop = sim->sim->get_real_property({instance, variable});
    if (prop) {
        *value = prop->get_value();
        return true;
    }
    return false;
}

extern "C" NOVA_API bool nova_simulation_set_real(nova_simulation_t* sim, const char* instance, const char* variable, double value) {
    auto prop = sim->sim->get_real_property({instance, variable});
    if (prop) {
        prop->set_value(value);
        return true;
    }
    return false;
}

extern "C" NOVA_API void nova_simulation_add_csv_writer(nova_simulation_t* sim, const char* filename) {
    sim->sim->add_listener("csv_writer", std::make_unique<nova_sim::csv_writer>(filename));
}

extern "C" NOVA_API nova_csv_writer_t* nova_csv_writer_create(const char* filename, const char* config_path) {
    auto w = new nova_csv_writer_t();
    w->writer = std::make_shared<csv_writer>(std::string(filename));
    if (config_path && std::string(config_path) != "") {
        try {
            w->writer->config().load(std::string(config_path));
        } catch(const std::exception& e) {
            log::err("Failed to load CSV config: {}", e.what());
        }
    }
    return w;
}

extern "C" NOVA_API void nova_simulation_add_listener(nova_simulation_t* sim, const char* name, nova_csv_writer_t* writer) {
    sim->sim->add_listener(std::string(name), writer->writer);
    delete writer;
}

extern "C" NOVA_API void nova_plot_csv(const char* csv_path, const char* config_path) {
    try {
        nova_sim::plot_csv(std::string(csv_path), std::string(config_path));
    } catch(const std::exception& e) {
        log::err("Failed to plot: {}", e.what());
    }
}

// Version info
extern "C" NOVA_API void nova_library_version(int* major, int* minor, int* patch) {
    auto v = library_version();
    *major = v.major;
    *minor = v.minor;
    *patch = v.patch;
}

extern "C" NOVA_API void nova_set_log_level(const char* level) {
    std::string l(level);
    log::level lvl = log::level::info;
    if (l == "trace") lvl = log::level::trace;
    else if (l == "debug") lvl = log::level::debug;
    else if (l == "info") lvl = log::level::info;
    else if (l == "warn") lvl = log::level::warn;
    else if (l == "err" || l == "error") lvl = log::level::err;
    else if (l == "off") lvl = log::level::off;
    
    log::set_logging_level(lvl);
}

extern "C" NOVA_API nova_simulation_runner_t* nova_simulation_runner_create(nova_simulation_t* sim) {
    auto runner = new nova_simulation_runner_t();
    runner->runner = std::make_unique<simulation_runner>(*sim->sim);
    return runner;
}

extern "C" NOVA_API void nova_simulation_runner_start(nova_simulation_runner_t* runner) {
    runner->runner->start();
}

extern "C" NOVA_API void nova_simulation_runner_stop(nova_simulation_runner_t* runner) {
    runner->runner->stop();
}

extern "C" NOVA_API void nova_simulation_runner_set_real_time_factor(nova_simulation_runner_t* runner, double factor) {
    runner->runner->set_real_time_factor(factor);
}

extern "C" NOVA_API void nova_simulation_runner_destroy(nova_simulation_runner_t* runner) {
    delete runner;
}
