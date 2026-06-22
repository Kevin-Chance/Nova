#include "nova_fmi/fmu.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace nova_fmi;

namespace
{

void test(fmu& fmu)
{
    const auto d = fmu.get_model_description();
    CHECK(d.modelName == "BouncingBall");
    CHECK(d.description ==
        "This model calculates the trajectory, over time, of a ball dropped from a height of 1 m");

    auto slave = fmu.new_instance("instance");
    REQUIRE(slave);
    REQUIRE(slave->enter_initialization_mode());
    REQUIRE(slave->exit_initialization_mode());

    // VR for h is typically 0 or 1. Let's use dynamic lookup
    auto var_opt = d.get_variable("h");
    REQUIRE(var_opt != nullptr);
    uint32_t vr = var_opt->vr;

    std::vector<double> realRef(1);

    double t = 0.0;
    double tEnd = 1.0;
    double dt = 0.1;

    double h;
    while (t <= tEnd) {
        REQUIRE(slave->step(t, dt));
        h = slave->get_real(vr);
        t += dt;
    }

    CHECK(h == Catch::Approx(0.0235492));

    REQUIRE(slave->terminate());
}

} // namespace

TEST_CASE("fmi_test_bouncingball")
{
    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/3.0/ref/BouncingBall.fmu";
    const auto fmu = loadFmu(fmuPath);
    REQUIRE(fmu);
    test(*fmu);
}
