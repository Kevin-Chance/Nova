#define NOVA_EXPORT_DLL
#include "ecos/nova_ecos.h"
#include "ecos/structure/simulation_structure.hpp"
#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/lib_info.hpp"
#include <memory>
#include <map>
#include <string>

using namespace nova_sim;

struct nova_simulation_structure_t {
    simulation_structure ss;
};

struct nova_simulation_t {
    std::unique_ptr<simulation> sim;
};

nova_simulation_structure_t* nova_simulation_structure_create() {
    return new nova_simulation_structure_t();
}

void nova_simulation_structure_destroy(nova_simulation_structure_t* ss) {
    delete ss;
}

bool nova_simulation_structure_add_model(nova_simulation_structure_t* ss, const char* name, const char* uri) {
    try {
        ss->ss.add_model(name, std::string(uri), std::nullopt);
        return true;
    } catch (...) {
        return false;
    }
}

void nova_simulation_structure_make_connection(nova_simulation_structure_t* ss, const char* src_inst, const char* src_var, const char* dst_inst, const char* dst_var, const char* type) {
    if (std::string(type) == "real")
        ss->ss.make_connection<double>({src_inst, src_var}, {dst_inst, dst_var});
    else if (std::string(type) == "int")
        ss->ss.make_connection<int>({src_inst, src_var}, {dst_inst, dst_var});
    else if (std::string(type) == "bool")
        ss->ss.make_connection<bool>({src_inst, src_var}, {dst_inst, dst_var});
}

nova_simulation_t* nova_simulation_create(nova_simulation_structure_t* ss, double step_size) {
    auto algo = std::make_unique<fixed_step_algorithm>(step_size);
    auto sim = new nova_simulation_t();
    sim->sim = ss->ss.load(std::move(algo));
    return sim;
}

void nova_simulation_destroy(nova_simulation_t* sim) {
    delete sim;
}

bool nova_simulation_init(nova_simulation_t* sim, double start_time) {
    try {
        sim->sim->init(start_time);
        return true;
    } catch (...) {
        return false;
    }
}

double nova_simulation_step(nova_simulation_t* sim, unsigned int num_steps) {
    return sim->sim->step(num_steps);
}

void nova_simulation_step_until(nova_simulation_t* sim, double time_point) {
    sim->sim->step_until(time_point);
}

void nova_simulation_step_for(nova_simulation_t* sim, double duration) {
    sim->sim->step_for(duration);
}

void nova_simulation_terminate(nova_simulation_t* sim) {
    sim->sim->terminate();
}

bool nova_simulation_get_real(nova_simulation_t* sim, const char* instance, const char* variable, double* value) {
    auto prop = sim->sim->get_real_property({instance, variable});
    if (prop) {
        *value = prop->get_value();
        return true;
    }
    return false;
}

bool nova_simulation_set_real(nova_simulation_t* sim, const char* instance, const char* variable, double value) {
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

// Version info
extern "C" NOVA_API void nova_library_version(int* major, int* minor, int* patch) {
    auto v = library_version();
    *major = v.major;
    *minor = v.minor;
    *patch = v.patch;
}
