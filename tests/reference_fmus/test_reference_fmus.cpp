#include <catch2/catch_test_macros.hpp>

#include "nova/engine/nova_fmu_locator.hpp"
#include "nova/engine/nova_engine.hpp"
#include <nova/components/algorithm/fixed_step_algorithm.hpp>
#include <nova/components/structure/simulation_structure.hpp>

using namespace nova_sim;

std::vector<std::filesystem::path> collectFMus(const std::filesystem::path& basePath)
{
    std::vector<std::filesystem::path> paths;
    if (!std::filesystem::exists(basePath)) return paths;
    for (const auto& entry : std::filesystem::directory_iterator(basePath)) {
        if (entry.is_regular_file()) {
            if (entry.path().extension() == ".fmu") {
                paths.emplace_back(entry.path());
            }
        }
    }
    return paths;
}

std::vector<std::filesystem::path> v1FMus()
{
    return collectFMus(std::string(REF_FMU_FOLDER) + "1.0/cs");
}

std::vector<std::filesystem::path> v2FMus()
{
    return collectFMus(std::string(REF_FMU_FOLDER) + "2.0");
}

std::vector<std::filesystem::path> v3FMus()
{
    return collectFMus(std::string(REF_FMU_FOLDER) + "3.0");
}

void run(const std::vector<std::filesystem::path>& paths)
{
    for (const auto& path : paths) {
        auto fmuModel = NovaFmuLocator::resolve(path.string());
        if (!fmuModel) continue;

        nova_engine sim(std::make_unique<fixed_step_algorithm>(1.0 / 100));
        auto slave = fmuModel->instantiate(path.stem().string(), std::nullopt);
        if (!slave) continue;
        
        sim.add_slave(std::move(slave));
        try {
            sim.init();
            sim.step(5);
            sim.terminate();
        } catch(...) {}
    }
}

TEST_CASE("Run reference fmus")
{
    run(v1FMus());
    run(v2FMus());
    run(v3FMus());
    CHECK(true);
}
