#include <catch2/catch_test_macros.hpp>
#include "ecos/nova_ecos.h"
#include <fstream>
#include <iostream>

// Compatibility wrappers for legacy test
#define ecos_library_version() nova_library_version_wrap()
struct ecos_version { int major, minor, patch; };
ecos_version nova_library_version_wrap() {
    int ma, mi, pa;
    nova_library_version(&ma, &mi, &pa);
    return {ma, mi, pa};
}

TEST_CASE("Test C lib")
{
    auto v = ecos_library_version();
    std::cout << "Using libecos version: " << v.major << "." << v.minor << "." << v.patch << std::endl;

    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/2.0/20sim/bouncing_ball.fmu";
    
    auto ss = nova_simulation_structure_create();
    REQUIRE(nova_simulation_structure_add_model(ss, "slave", fmuPath.c_str()));

    auto sim = nova_simulation_create(ss, 0.01);
    REQUIRE(sim);

    REQUIRE(nova_simulation_init(sim, 0.0));

    double h_start = 0;
    // Check both names for robustness
    if (!nova_simulation_get_real(sim, "slave", "height", &h_start)) {
        REQUIRE(nova_simulation_get_real(sim, "slave", "h", &h_start));
    }
    CHECK(h_start > 0);

    nova_simulation_step(sim, 10);

    double h_end = 0;
    if (!nova_simulation_get_real(sim, "slave", "height", &h_end)) {
        REQUIRE(nova_simulation_get_real(sim, "slave", "h", &h_end));
    }
    CHECK(h_end < h_start);

    nova_simulation_terminate(sim);
    nova_simulation_destroy(sim);
    nova_simulation_structure_destroy(ss);
}
