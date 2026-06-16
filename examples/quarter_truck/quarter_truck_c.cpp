
#include "nova/nova.h"

#include <filesystem>

// just to check if it works
double modifier(double val)
{
    return val;
}

int main()
{
    nova_set_log_level("debug");

    const std::filesystem::path fmuDir = std::string(DATA_FOLDER) + "/ssp/1.0/quarter_truck/resources";
//D:
//    \project\nova - nuaa\data\ssp\1.0\quarter_truck\resources
    const auto ss = nova_simulation_structure_create();
    nova_simulation_structure_add_model(ss, "chassis", (fmuDir / "chassis.fmu").string().c_str());
    nova_simulation_structure_add_model(ss, "ground", (fmuDir / "ground.fmu").string().c_str());
    nova_simulation_structure_add_model(ss, "wheel", (fmuDir / "wheel.fmu").string().c_str());

    nova_simulation_structure_make_real_connection_mod(ss, "chassis::p.e", "wheel::p1.e", modifier);
    nova_simulation_structure_make_real_connection(ss, "wheel::p1.f", "chassis::p.f");
    nova_simulation_structure_make_real_connection(ss, "wheel::p.e", "ground::p.e");
    nova_simulation_structure_make_real_connection(ss, "ground::p.f", "wheel::p.f");

    const auto pps = nova_parameter_set_create();
    nova_parameter_set_add_real(pps, "chassis::C.mChassis", 400.);
    nova_simulation_structure_add_parameter_set(ss, "initialValues", pps);

    const auto sim = nova_simulation_create_from_structure(ss, 1.0 / 100);

    nova_simulation_structure_destroy(ss);
    nova_parameter_set_destroy(pps);

    const auto csvConfig = std::string(DATA_FOLDER) + "/ssp/1.0/quarter_truck/CsvConfig.xml";
    const auto plotConfig = std::string(DATA_FOLDER) + "/ssp/1.0/quarter_truck/ChartConfig.xml";
    const auto resultFile = std::string{"results/quarter_truck_c_with_config.csv"};
    const auto csvWriter = nova_csv_writer_create(resultFile.c_str(), csvConfig.c_str());

    nova_simulation_add_listener(sim, "CSV Writer", csvWriter);

    nova_simulation_init(sim, 0, "initialValues");
    nova_simulation_step_until(sim, 10);
    nova_simulation_terminate(sim);

    nova_plot_csv(resultFile.c_str(), plotConfig.c_str());

    nova_simulation_destroy(sim);
}
