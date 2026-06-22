#include "nova/engine/nova_engine.hpp"
#include "nova/components/structure/simulation_structure.hpp"
#include <nova/components/algorithm/fixed_step_algorithm.hpp>
#include <nova/components/recorder/csv_recorder.hpp>
#include <nova/components/logger/logger.hpp>
#include <nova/components/util/chart_plotter.hpp>
#include <filesystem>

using namespace nova_sim;

int main()
{
    set_logging_level(log::level::debug);

    std::filesystem::create_directories(RESULT_FOLDER);
    std::string resultFile{std::string(RESULT_FOLDER) + "/nova_bouncing_ball_cpp.csv"};
    const std::string fmuPath{std::string(DATA_FOLDER) + "/fmus/3.0/ref/BouncingBall.fmu"};

    simulation_structure ss;
    ss.add_model("ball", fmuPath);

    double stepSize = 1.0 / 100.0;
    auto algo = std::make_unique<fixed_step_algorithm>(stepSize);
    
    const auto sim = ss.load(std::move(algo));

    auto csvWriter = std::make_unique<csv_recorder>(resultFile);
    const auto outputPath = csvWriter->output_path();
    csv_config& config = csvWriter->config();
    config.register_variable("ball::h");

    sim->add_listener("csv_recorder", std::move(csvWriter));

    sim->init();
    sim->step(1000); // 10 seconds with 1/100 step size
    sim->terminate();

    log::info("Nova C++ BouncingBall finished.");

    TChartConfig plotConfig;
    plotConfig.addChart(TTimeSeriesChart{
        "bouncing_ball",
        "Time[s]",
        {{{"ball", {{"h"}}}}}});

    plot_csv(outputPath, plotConfig);

    return 0;
}
