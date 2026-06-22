#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "nova/components/algorithm/fixed_step_algorithm.hpp"
#include "nova/engine/nova_fmu_locator.hpp"
#include "nova/engine/engine_scheduler.hpp"
#include "nova/components/logger/logger.hpp"

using namespace nova_sim;

TEST_CASE("test nova_engine runner")
{
    set_logging_level(nova_sim::log::level::debug);

    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/2.0/20sim/ControlledTemperature.fmu";
    
    // Intentionally leak the model to prevent FreeLibrary crashes on teardown from this specific 20-sim FMU
    auto* fmuModel = new auto(NovaFmuLocator::resolve(fmuPath));
    REQUIRE((*fmuModel) != nullptr);

    // Also leak the sim to prevent teardown crashes
    auto* sim = new nova_engine(std::make_unique<fixed_step_algorithm>(1.0 / 100));
    auto inst = (*fmuModel)->instantiate("slave", std::nullopt);
    REQUIRE(inst != nullptr);
    sim->add_slave(std::move(inst));
    sim->init();

    auto runner = engine_scheduler(*sim);
    runner.set_real_time_factor(1);
    auto future = runner.run_while([&sim] {
        return sim->time() < 0.1;
    });

    future.get();

    log::debug("Simulated {:.3f}s in {:.4f}s, RTF={:.3f}", sim->time(), runner.wall_clock(), runner.real_time_factor());

    CHECK_THAT(runner.real_time_factor(), Catch::Matchers::WithinAbs(runner.target_real_time_factor(), 0.1));

    sim->terminate();
}
