#include "fmilibcpp/fmu.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <nova/model_instance.hpp>

using namespace nova_fmi;

namespace
{

void test(fmu& fmu)
{
    const auto md = fmu.get_model_description();
    auto instance = fmu.new_instance("instance");
    REQUIRE(instance != nullptr);
    REQUIRE(instance->enter_initialization_mode());
    REQUIRE(instance->exit_initialization_mode());

    auto h_var = md.get_variable("h");
    REQUIRE(h_var != nullptr);
    uint32_t vr = h_var->vr;

    double h_initial = instance->get_real(vr);
    REQUIRE(instance->step(0.0, 0.1));
    double h_after = instance->get_real(vr);
    
    // FMI 3.0 Reference FMU BouncingBall supports state
    void* state = instance->get_state();
    REQUIRE(state != nullptr);
    
    REQUIRE(instance->step(0.1, 0.1));
    double h_later = instance->get_real(vr);
    CHECK(h_later != Catch::Approx(h_after));

    REQUIRE(instance->set_state(state));
    CHECK(instance->get_real(vr) == Catch::Approx(h_after));

    instance->free_state(state);
    REQUIRE(instance->terminate());
}

} // namespace

TEST_CASE("fmi_test_state")
{
    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/3.0/ref/BouncingBall.fmu";
    auto fmu = loadFmu(fmuPath);
    REQUIRE(fmu != nullptr);
    test(*fmu);
}
