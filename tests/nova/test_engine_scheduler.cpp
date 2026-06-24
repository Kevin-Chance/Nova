#include <catch2/catch_test_macros.hpp>
#include <nova/engine/engine_scheduler.hpp>
#include <nova/engine/nova_engine.hpp>
#include <nova/components/algorithm/fixed_step_algorithm.hpp>

using namespace nova_sim;

TEST_CASE("test_engine_scheduler_pause_toggle")
{
    nova_engine sim(std::make_unique<fixed_step_algorithm>(0.1));
    engine_scheduler scheduler(sim);

    bool paused = scheduler.toggle_pause();
    CHECK(paused == true);

    paused = scheduler.toggle_pause();
    CHECK(paused == false);
}

TEST_CASE("test_engine_scheduler_rtf")
{
    nova_engine sim(std::make_unique<fixed_step_algorithm>(0.1));
    engine_scheduler scheduler(sim);

    CHECK(scheduler.target_real_time_factor() == 1.0);

    scheduler.set_real_time_factor(0.5);
    CHECK(scheduler.target_real_time_factor() == 0.5);

    scheduler.set_real_time_factor(2.0);
    CHECK(scheduler.target_real_time_factor() == 2.0);
}
