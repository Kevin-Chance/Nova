
#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/ssp/ssp_loader.hpp"
#include "ecos/util/plotter.hpp"

#include <spdlog/stopwatch.h>

using namespace nova_sim;

int main()
{
    set_logging_level(log::level::debug);

    const std::filesystem::path sspFolder = std::string(DATA_FOLDER) + "/ssp/1.0/quarter_truck";

    try {
        const auto ss = load_ssp(sspFolder);
        const auto sim = ss->load(std::make_unique<fixed_step_algorithm>(1.0 / 100));

        std::filesystem::create_directories(RESULT_FOLDER);
        auto csvWriter = std::make_unique<csv_writer>(std::string(RESULT_FOLDER) + "/nova_quarter_truck_ssp.csv");
        
        csv_config& config = csvWriter->config();
        config.register_variable("chassis::zChassis");
        config.register_variable("wheel::zWheel");
        config.register_variable("ground::zGround");
        sim->add_listener("csv_writer", std::move(csvWriter));

        spdlog::stopwatch sw;
        sim->init("initialValues");
        sim->step_until(5);
        log::info("Nova SSP Quarter-Truck finished. Elapsed {:.4f}s", sw);

        sim->terminate();

        // plot_csv(outputPath, sspFolder / "ChartConfig.xml");
    } catch (const std::exception& ex) {

        log::err(ex.what());
    }
}
