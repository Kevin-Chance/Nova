#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <nova/components/algorithm/fixed_step_algorithm.hpp>
#include <nova/engine/nova_engine.hpp>
#include <nova/engine/model_instance.hpp>

using namespace nova_sim;

class step_counter_instance : public model_instance {
public:
    step_counter_instance(const std::string& name, std::optional<double> stepHint = std::nullopt)
        : model_instance(name, stepHint) {}

    void enter_initialization_mode(double start = 0) override {}
    void exit_initialization_mode() override {}
    void step(double currentTime, double stepSize) override {
        step_count++;
        last_step_size = stepSize;
    }
    void terminate() override {}
    void reset() override { step_count = 0; }

    int step_count = 0;
    double last_step_size = 0.0;
};

TEST_CASE("test_fixed_step_algorithm_basic")
{
    auto algo = std::make_unique<fixed_step_algorithm>(0.1, false);
    nova_engine sim(std::move(algo));

    auto inst = std::make_unique<step_counter_instance>("inst1");
    auto ptr = inst.get();
    sim.add_slave(std::move(inst));

    sim.init();
    sim.step(10);

    CHECK(ptr->step_count == 10);
    CHECK_THAT(ptr->last_step_size, Catch::Matchers::WithinRel(0.1));
}

TEST_CASE("test_fixed_step_algorithm_macro_steps")
{
    auto algo = std::make_unique<fixed_step_algorithm>(0.05, false);
    nova_engine sim(std::move(algo));

    auto inst = std::make_unique<step_counter_instance>("inst_macro", 0.1);
    auto ptr = inst.get();
    sim.add_slave(std::move(inst));

    sim.init();
    sim.step(20); // engine steps 20 times * 0.05 = 1.0s

    CHECK(ptr->step_count == 10); // slave steps 10 times with 0.1s step size
    CHECK_THAT(ptr->last_step_size, Catch::Matchers::WithinRel(0.05));
}
