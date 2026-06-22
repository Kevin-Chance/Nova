#include <catch2/catch_test_macros.hpp>
#include "nova_fmi/fmu.hpp"

using namespace nova_fmi;

namespace
{

void test(nova_fmi::fmu& fmu)
{
    const auto d = fmu.get_model_description();
    CHECK(d.modelName == "no.viproma.demo.identity");

    auto slave = fmu.new_instance("instance");
    REQUIRE(slave != nullptr);
    REQUIRE(slave->enter_initialization_mode());
    REQUIRE(slave->exit_initialization_mode());

    std::vector<value_ref> vr{0}; // Often 0 for input/output in this FMU
    
    // We should look up the variable by name "input" to be robust
    auto var_opt = d.get_variable("input");
    if (var_opt) vr[0] = var_opt->vr;

    std::vector<double> realVal{123.45};
    std::vector<int> integerVal{0};
    std::vector<bool> booleanVal{false};
    std::vector<std::string> stringVal{""};

    std::vector<double> realRef(1);
    std::vector<int> integerRef(1);
    std::vector<bool> booleanRef(1);
    std::vector<std::string> stringRef(1);

    double t = 0.0;
    double tEnd = 1.0;
    double dt = 0.1;

    while (t <= tEnd) {
        slave->get_real(vr, realRef);
        slave->get_integer(vr, integerRef);
        slave->get_boolean(vr, booleanRef);
        slave->get_string(vr, stringRef);

        REQUIRE(slave->step(t, dt));

        realVal[0] += 1.0;
        integerVal[0] += 1;
        booleanVal[0] = !booleanVal[0];
        stringVal[0] += 'a';

        slave->set_real(vr, realVal);
        slave->set_integer(vr, integerVal);
        slave->set_boolean(vr, booleanVal);
        slave->set_string(vr, stringVal);

        t += dt;
    }

    REQUIRE(slave->terminate());
}

} // namespace

TEST_CASE("fmi_test_identity")
{
    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/1.0/identity.fmu";
    auto fmu = loadFmu(fmuPath);
    REQUIRE(fmu != nullptr);
    test(*fmu);
}
