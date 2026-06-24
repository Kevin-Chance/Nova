#include <catch2/catch_test_macros.hpp>
#include "../../../src/engine/scenario.hpp"
#include <nova/engine/model_instance.hpp>
#include <nova/engine/nova_engine.hpp>

using namespace nova_sim;

#include <iostream>

TEST_CASE("test_scenario_timed_actions", "[scenario]") {
    scenario sc;
    int execution_count = 0;

    sc.invoke_at(timed_action(0.5, [&]() {
        execution_count++;
    }, 1e-6));

    sc.invoke_at(timed_action(1.0, [&]() {
        execution_count += 2;
    }, 1e-6));

    // Initially zero
    CHECK(execution_count == 0);

    // Apply at t=0.2 (Nothing should happen)
    sc.apply(0.2);
    CHECK(execution_count == 0);

    // Apply at t=0.5 (First action triggers)
    sc.apply(0.5);
    CHECK(execution_count == 1);

    // Apply at t=0.5 again (Should not trigger again because it was discarded)
    sc.apply(0.5);
    CHECK(execution_count == 1);

    // Apply at t=1.0 (Second action triggers)
    sc.apply(1.0);
    CHECK(execution_count == 3);
}

TEST_CASE("test_scenario_predicate_actions", "[scenario]") {
    scenario sc;
    int value = 0;
    bool triggered = false;

    sc.invoke_when(predicate_action(
        [&]() { return value > 5; },
        [&]() { triggered = true; }
    ));

    sc.apply(0.0);
    CHECK_FALSE(triggered);

    value = 4;
    sc.apply(0.1);
    CHECK_FALSE(triggered);

    value = 6;
    sc.apply(0.2);
    CHECK(triggered);

    // Reset trigger to verify it only runs once and is then discarded
    triggered = false;
    sc.apply(0.3);
    CHECK_FALSE(triggered);
}

TEST_CASE("test_scenario_init_actions", "[scenario]") {
    scenario sc;
    bool init_called = false;

    sc.on_init([&]() { init_called = true; });

    CHECK_FALSE(init_called);
    sc.runInitActions();
    CHECK(init_called);
    
    // Check that it's cleared after running
    init_called = false;
    sc.runInitActions();
    CHECK_FALSE(init_called);
}

#include <fstream>
#include <filesystem>

class dummy_scenario_slave : public model_instance {
public:
    dummy_scenario_slave(const std::string& name) : model_instance(name) {
        val1 = 0.0;
        int_val = 0;
        bool_val = false;
        str_val = "init";
        properties_.add_real_property("val1", property_t<double>::create(variable_identifier(name, "val1"), [&]{ return val1; }, [&](double v){ val1 = v; }));
        properties_.add_int_property("int_val", property_t<int>::create(variable_identifier(name, "int_val"), [&]{ return int_val; }, [&](int v){ int_val = v; }));
        properties_.add_bool_property("bool_val", property_t<bool>::create(variable_identifier(name, "bool_val"), [&]{ return bool_val; }, [&](bool v){ bool_val = v; }));
        properties_.add_string_property("str_val", property_t<std::string>::create(variable_identifier(name, "str_val"), [&]{ return str_val; }, [&](std::string v){ str_val = v; }));
    }

    void enter_initialization_mode(double start = 0) override {}
    void exit_initialization_mode() override {}
    void step(double currentTime, double stepSize) override {}
    void terminate() override {}
    void reset() override {}

    double val1;
    int int_val;
    bool bool_val;
    std::string str_val;
};

TEST_CASE("test_scenario_load_xml", "[scenario]") {
    std::filesystem::path config_file = std::filesystem::temp_directory_path() / "test_scenario.xml";
    std::ofstream ofs(config_file);
    ofs << "<nova:Scenario>\n"
        << "  <nova:action t=\"1.0\">\n"
        << "    <nova:variable id=\"dummy::val1\">\n"
        << "      <nova:real value=\"42.5\"/>\n"
        << "    </nova:variable>\n"
        << "    <nova:variable id=\"dummy::int_val\">\n"
        << "      <nova:integer value=\"99\"/>\n"
        << "    </nova:variable>\n"
        << "  </nova:action>\n"
        << "  <nova:action t=\"2.0\" eps=\"1e-4\">\n"
        << "    <nova:variable id=\"dummy::bool_val\">\n"
        << "      <nova:boolean value=\"1\"/>\n"
        << "    </nova:variable>\n"
        << "    <nova:variable id=\"dummy::str_val\">\n"
        << "      <nova:string value=\"success\"/>\n"
        << "    </nova:variable>\n"
        << "  </nova:action>\n"
        << "</nova:Scenario>\n";
    ofs.close();

    std::vector<std::unique_ptr<model_instance>> instances;
    instances.push_back(std::make_unique<dummy_scenario_slave>("dummy"));

    scenario sc;
    sc.load(config_file, instances);
    
    auto* slave = static_cast<dummy_scenario_slave*>(instances[0].get());

    CHECK(slave->val1 == 0.0);
    CHECK(slave->int_val == 0);
    CHECK(slave->bool_val == false);
    CHECK(slave->str_val == "init");

    // Apply before any action triggers
    sc.apply(0.5);
    slave->get_properties().apply_sets();
    CHECK(slave->val1 == 0.0);

    // Apply at t=1.0 triggers real and integer modifications
    sc.apply(1.0);
    slave->get_properties().apply_sets();
    CHECK(slave->val1 == 42.5);
    CHECK(slave->int_val == 99);
    CHECK(slave->bool_val == false);

    // Apply at t=2.0 triggers bool and string modifications
    sc.apply(2.0);
    slave->get_properties().apply_sets();
    CHECK(slave->bool_val == true);
    CHECK(slave->str_val == "success");

    std::filesystem::remove(config_file);
}

TEST_CASE("test_scenario_load_errors", "[scenario]") {
    scenario sc;
    std::vector<std::unique_ptr<model_instance>> instances;

    std::filesystem::path missing_file = std::filesystem::temp_directory_path() / "missing_scenario.xml";
    CHECK_THROWS_AS(sc.load(missing_file, instances), std::runtime_error);

    std::filesystem::path wrong_ext_file = std::filesystem::temp_directory_path() / "scenario.txt";
    std::ofstream ofs_txt(wrong_ext_file);
    ofs_txt << "<test/>";
    ofs_txt.close();
    CHECK_THROWS_AS(sc.load(wrong_ext_file, instances), std::runtime_error);
    std::filesystem::remove(wrong_ext_file);

    std::filesystem::path malformed_xml = std::filesystem::temp_directory_path() / "malformed_scen.xml";
    std::ofstream ofs_xml(malformed_xml);
    ofs_xml << "<nova:Scenario><unclosed>";
    ofs_xml.close();
    CHECK_THROWS_AS(sc.load(malformed_xml, instances), std::runtime_error);
    std::filesystem::remove(malformed_xml);
}
