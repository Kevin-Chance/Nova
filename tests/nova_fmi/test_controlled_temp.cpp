#include "nova_fmi/fmu.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <iostream>

using namespace nova_fmi;

namespace
{

void test(fmu& fmu)
{
    const auto d = fmu.get_model_description();
    CHECK(d.modelName == "ControlledTemperature");

    auto slave = fmu.new_instance("instance");
    REQUIRE(slave != nullptr);
    REQUIRE(slave->enter_initialization_mode());
    REQUIRE(slave->exit_initialization_mode());

    // Original test used vr=47, which maps to Temperature_Room
    auto t_var = d.get_variable("Temperature_Room");
    REQUIRE(t_var != nullptr);
    uint32_t vr = t_var->vr;

    double t_initial = slave->get_real(vr);
    CHECK(t_initial > 0);

    REQUIRE(slave->step(0.0, 0.1));

    double t_later = slave->get_real(vr);
    CHECK(t_later != t_initial);

    REQUIRE(slave->terminate());
}

} // namespace

TEST_CASE("fmi_test_controlled_temp")
{
    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/2.0/20sim/ControlledTemperature.fmu";
    auto fmu = loadFmu(fmuPath);
    REQUIRE(fmu != nullptr);
    test(*fmu);
}
