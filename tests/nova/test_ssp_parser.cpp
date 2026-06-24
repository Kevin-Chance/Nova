#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "ssp/ssp.hpp"
#include <iostream>

using namespace nova_sim;

namespace
{

void checkSystemStructure(const ssp::SystemStructureDescription& ssd)
{
    CHECK(ssd.name == "QuarterTruck");

    const auto system = ssd.system;
    CHECK(system.name == "QuarterTruckSystem");

    CHECK(system.connections.size() == 4);

    const auto components = system.elements.components;
    CHECK(components.size() == 3);

    REQUIRE(components.contains("chassis"));
    REQUIRE(components.contains("wheel"));
    REQUIRE(components.contains("ground"));

    const ssp::Component& chassis = components.at("chassis");
    CHECK(chassis.source == "resources/chassis.fmu");
    REQUIRE(chassis.connectors.size() == 2);
    CHECK(chassis.connectors.at("p.e").name == "p.e");
    CHECK(chassis.connectors.at("p.e").kind == "output");
    CHECK(!chassis.connectors.at("p.e").type.unit.has_value());
    CHECK(chassis.connectors.at("p.e").type.typeName() == "Real");
    CHECK(chassis.connectors.at("p.f").name == "p.f");
    CHECK(chassis.connectors.at("p.f").kind == "input");
    CHECK(!chassis.connectors.at("p.f").type.unit.has_value());
    CHECK(chassis.connectors.at("p.f").type.typeName() == "Real");

    const ssp::Component& wheel = components.at("wheel");
    CHECK(wheel.source == "resources/wheel.fmu");
    REQUIRE(wheel.connectors.size() == 4);
    CHECK(wheel.connectors.at("p.f").name == "p.f");
    CHECK(wheel.connectors.at("p1.e").name == "p1.e");
    CHECK(wheel.connectors.at("p.e").name == "p.e");
    CHECK(wheel.connectors.at("p1.f").name == "p1.f");

    CHECK(wheel.connectors.at("p.e").type == wheel.connectors.at("p1.f").type);

    const auto& wheelParameters = wheel.parameterSets;
    std::cout << "Wheel parameter set keys: ";
    for (const auto& kv : wheelParameters) {
        std::cout << "'" << kv.first << "' ";
    }
    std::cout << std::endl;
    REQUIRE(wheelParameters.at("initialValues").parameters.size() == 3);

    const ssp::Component& ground = components.at("ground");
    CHECK(ground.source == "resources/ground.fmu");
    CHECK(ground.connectors.size() == 2);

    const auto& groundParameters = ground.parameterSets;
    REQUIRE(groundParameters.empty());

    REQUIRE(system.elements.parameterSets.size() == 1);
    REQUIRE(system.elements.parameterSets.contains("initialValues"));
    const auto& initialValues = system.elements.parameterSets.at("initialValues");
    REQUIRE(initialValues.size() == 2);
    REQUIRE(initialValues.contains(chassis));
    REQUIRE(initialValues.at(chassis).size() == 3);
    REQUIRE(initialValues.contains(wheel));
    REQUIRE(initialValues.at(wheel).size() == 3);
}

} // namespace

TEST_CASE("test_ssp_parser_archive")
{
    std::filesystem::path temporal;

    {
        const auto quarterTruckArchive = std::string(DATA_FOLDER) + "/ssp/1.0/quarter_truck/quarter-truck.ssp";
        ssp::SystemStructureDescription desc(quarterTruckArchive);
        checkSystemStructure(desc);
        temporal = desc.dir();

        const auto& groundComponent = desc.system.elements.components.at("ground");
        const auto& groundFmu = desc.file(groundComponent.source);
        REQUIRE(std::filesystem::exists(groundFmu));
        REQUIRE(groundFmu.extension().string() == ".fmu");
    }

    REQUIRE(!std::filesystem::exists(temporal));
}

TEST_CASE("test_ssp_parser_folder")
{
    const auto quarterTruckFolder = std::string(DATA_FOLDER) + "/ssp/1.0/quarter_truck";
    ssp::SystemStructureDescription desc(quarterTruckFolder);
    checkSystemStructure(desc);

    const auto& ex = desc.defaultExperiment;
    REQUIRE(ex);
    CHECK_THAT(*ex->start, Catch::Matchers::WithinRel(10.));

    const auto& annotations = ex->annotations;
    REQUIRE(annotations.size() == 1);
    REQUIRE(annotations.front().type == "com.opensimulationplatform");
    const auto& annotationNode = annotations.front().node;
    const auto algorithmNode = annotationNode.child("osp:Algorithm");
    REQUIRE(algorithmNode);
    const auto fixedStepNode = algorithmNode.child("osp:FixedStepAlgorithm");
    REQUIRE(fixedStepNode);
    CHECK_THAT(fixedStepNode.attribute("baseStepSize").as_double(), Catch::Matchers::WithinRel(1e-4));
}

#include <fstream>
TEST_CASE("test_ssp_parser_negative_paths")
{
    std::filesystem::path bad_folder = std::filesystem::temp_directory_path() / "bad_ssp_test";
    std::filesystem::create_directories(bad_folder);
    std::filesystem::path bad_ssd = bad_folder / "SystemStructure.ssd";
    std::ofstream ofs(bad_ssd);
    ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<ssd:SystemStructureDescription xmlns:ssd=\"http://ssp-standard.org/SSP1/SystemStructureDescription\" name=\"BadSystem\">\n"
        << "  <ssd:System name=\"Sys\">\n"
        << "    <ssd:Elements>\n"
        << "      <ssd:Component name=\"comp\" source=\"fake.fmu\">\n"
        << "        <ssd:ParameterBindings>\n"
        << "          <ssd:ParameterBinding>\n"
        << "            <!-- Missing ssv:ParameterSet to trigger the runtime_error we patched earlier -->\n"
        << "          </ssd:ParameterBinding>\n"
        << "        </ssd:ParameterBindings>\n"
        << "      </ssd:Component>\n"
        << "    </ssd:Elements>\n"
        << "  </ssd:System>\n"
        << "</ssd:SystemStructureDescription>\n";
    ofs.close();

    CHECK_THROWS_AS(ssp::SystemStructureDescription(bad_folder.string()), std::runtime_error);

    std::filesystem::remove_all(bad_folder);
}
