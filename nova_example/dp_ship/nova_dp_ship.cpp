#include "nova/algorithm/fixed_step_algorithm.hpp"
#include "nova/listeners/csv_writer.hpp"
#include "nova/logger/logger.hpp"
#include "nova/ssp/ssp_loader.hpp"
#include "nova/util/plotter.hpp"

#include <filesystem>

using namespace nova_sim;

int main()
{
    set_logging_level(log::level::debug);

    std::filesystem::path sspDir = std::string(DATA_FOLDER) + "/ssp/1.0/dp_ship";

    try {
        const auto ss = load_ssp(sspDir);
        const auto sim = ss->load(std::make_unique<fixed_step_algorithm>(0.04));

        //sim->load_scenario(sspDir / "waypoints_scenario.xml");

        std::filesystem::create_directories(RESULT_FOLDER);
        const auto outputPath = std::string(RESULT_FOLDER) + "/nova_dp_ship_cpp.csv";
        auto writer = std::make_unique<csv_writer>(outputPath);
        writer->config().load(sspDir / "CsvConfig.xml");
        sim->add_listener("writer", std::move(writer));

        sim->init();
        sim->step_until(1500);

        sim->terminate();

        plot_csv(outputPath, sspDir / "ChartConfig.xml");
        log::info("Nova C++ dp_ship finished. Results: {}", outputPath);
    } catch (const std::exception& ex) {
        log::err(ex.what());
    }
    return 0;
}
