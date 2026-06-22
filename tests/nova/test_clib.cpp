#include <catch2/catch_test_macros.hpp>
#include "nova/api/nova.h"
#include <fstream>
#include <iostream>

// Compatibility wrappers for legacy test
#define nova_library_version() nova_library_version_wrap()
struct nova_version { int major, minor, patch; };
nova_version nova_library_version_wrap() {
    int ma, mi, pa;
    nova_library_version(&ma, &mi, &pa);
    return {ma, mi, pa};
}

TEST_CASE("Test C lib")
{
    auto v = nova_library_version();
    std::cout << "Using libnova version: " << v.major << "." << v.minor << "." << v.patch << std::endl;

    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/2.0/20sim/bouncing_ball.fmu";
    
    auto ss = nova_simulation_structure_create();
    REQUIRE(nova_simulation_structure_add_model(ss, "slave", fmuPath.c_str()));

    auto sim = nova_engine_create(ss, 0.01);
    REQUIRE(sim);

    REQUIRE(nova_engine_init(sim, 0.0, nullptr));

    double h_start = 0;
    // Check both names for robustness
    if (!nova_engine_get_real(sim, "slave", "height", &h_start)) {
        REQUIRE(nova_engine_get_real(sim, "slave", "h", &h_start));
    }
    CHECK(h_start > 0);

    nova_engine_step(sim, 10);

    double h_end = 0;
    if (!nova_engine_get_real(sim, "slave", "height", &h_end)) {
        REQUIRE(nova_engine_get_real(sim, "slave", "h", &h_end));
    }
    CHECK(h_end < h_start);

    nova_engine_terminate(sim);
    nova_engine_destroy(sim);
    nova_simulation_structure_destroy(ss);
}
