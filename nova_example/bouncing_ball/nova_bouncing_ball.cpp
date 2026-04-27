#include "ecos/simulation.hpp"
#include "ecos/structure/simulation_structure.hpp"
#include <ecos/algorithm/fixed_step_algorithm.hpp>
#include <ecos/listeners/csv_writer.hpp>
#include <ecos/logger/logger.hpp>
#include <filesystem>

using namespace nova_sim;

int main()
{
    set_logging_level(log::level::debug);

    std::filesystem::create_directories("results");
    std::string resultFile{"results/nova_bouncing_ball_cpp.csv"};
    const std::string fmuPath{std::string(DATA_FOLDER) + "/fmus/3.0/ref/BouncingBall.fmu"};

    simulation_structure ss;
    ss.add_model("ball", fmuPath);

    double stepSize = 1.0 / 100.0;
    auto algo = std::make_unique<fixed_step_algorithm>(stepSize);
    
    const auto sim = ss.load(std::move(algo));

    auto csvWriter = std::make_unique<csv_writer>(resultFile);
    csv_config& config = csvWriter->config();
    config.register_variable("ball::h");

    sim->add_listener("csv_writer", std::move(csvWriter));

    sim->init();
    sim->step(1000); // 10 seconds with 1/100 step size
    sim->terminate();

    log::info("Nova C++ BouncingBall finished.");

    return 0;
}
