#include "nova_fmi/buffered_slave.hpp"
#include "nova_fmi/fmu.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace nova_fmi;

namespace
{

void test(fmu& fmu)
{
    const auto d = fmu.get_model_description();
    CHECK(d.modelName == "no.viproma.demo.identity");

    auto slave = std::make_unique<buffered_slave>(fmu.new_instance("instance"));
    REQUIRE(slave != nullptr);
    REQUIRE(slave->enter_initialization_mode());
    REQUIRE(slave->exit_initialization_mode());

    std::vector<value_ref> vr{0}; // Usually 0
    auto var_opt = d.get_variable("input");
    if (var_opt) vr[0] = var_opt->vr;

    std::vector<double> realVal{0.0};
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
        slave->receiveCachedGets();

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

        slave->transferCachedSets();

        t += dt;
    }

    REQUIRE(slave->terminate());
}

} // namespace

TEST_CASE("test_buffered_identity")
{
    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/1.0/identity.fmu";
    auto fmu = loadFmu(fmuPath);
    REQUIRE(fmu != nullptr);
    test(*fmu);
}
