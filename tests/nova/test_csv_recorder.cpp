#include <catch2/catch_test_macros.hpp>
#include <nova/components/recorder/csv_recorder.hpp>
#include <nova/components/algorithm/fixed_step_algorithm.hpp>
#include <nova/engine/nova_engine.hpp>
#include <nova/engine/model_instance.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace nova_sim;

class dummy_csv_slave : public model_instance {
public:
    dummy_csv_slave(const std::string& name) : model_instance(name) {
        val1 = 10.0;
        val2 = 20.0;
        int_val = 42;
        bool_val = true;
        str_val = "hello";
        properties_.add_real_property("val1", property_t<double>::create(variable_identifier(name, "val1"), [&]{ return val1; }, [&](double v){ val1 = v; }));
        properties_.add_real_property("val2", property_t<double>::create(variable_identifier(name, "val2"), [&]{ return val2; }, [&](double v){ val2 = v; }));
        properties_.add_int_property("int_val", property_t<int>::create(variable_identifier(name, "int_val"), [&]{ return int_val; }, [&](int v){ int_val = v; }));
        properties_.add_bool_property("bool_val", property_t<bool>::create(variable_identifier(name, "bool_val"), [&]{ return bool_val; }, [&](bool v){ bool_val = v; }));
        properties_.add_string_property("str_val", property_t<std::string>::create(variable_identifier(name, "str_val"), [&]{ return str_val; }, [&](std::string v){ str_val = v; }));
    }

    void enter_initialization_mode(double start = 0) override {}
    void exit_initialization_mode() override {}
    void step(double currentTime, double stepSize) override {
        val1 += 1.0;
        val2 += 2.0;
        int_val += 1;
        bool_val = !bool_val;
        str_val = "world";
    }
    void terminate() override {}
    void reset() override {}

    double val1;
    double val2;
    int int_val;
    bool bool_val;
    std::string str_val;
};

TEST_CASE("test_csv_recorder_logging", "[csv_recorder]") {
    std::filesystem::path temp_file = std::filesystem::temp_directory_path() / "test_nova_recorder.csv";
    if (std::filesystem::exists(temp_file)) std::filesystem::remove(temp_file);

    auto algo = std::make_unique<fixed_step_algorithm>(0.1, false);
    nova_engine sim(std::move(algo));
    sim.add_slave(std::make_unique<dummy_csv_slave>("dummy"));

    auto recorder = std::make_shared<csv_recorder>(temp_file.string());
    sim.add_listener("csv", recorder);

    sim.init();
    sim.step(2); // Steps: t=0.0 -> t=0.1 (Step 1) -> t=0.2 (Step 2)
    sim.terminate();

    // Verify file exists
    CHECK(std::filesystem::exists(temp_file));

    // Read content
    std::ifstream ifs(temp_file);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(ifs, line)) {
        lines.push_back(line);
    }
    ifs.close();

    // The header and exactly 3 data rows (t=0.0 init, t=0.1 step1, t=0.2 step2)
    CHECK(lines.size() >= 4);

    // Verify header format (should contain dummy::val1 and dummy::val2)
    CHECK(lines[0].find("time") != std::string::npos);
    CHECK(lines[0].find("dummy::val1") != std::string::npos);
    CHECK(lines[0].find("dummy::val2") != std::string::npos);
    CHECK(lines[0].find("dummy::int_val[INT]") != std::string::npos);
    CHECK(lines[0].find("dummy::bool_val[BOOL]") != std::string::npos);
    CHECK(lines[0].find("dummy::str_val[STR]") != std::string::npos);

    // Verify data logic
    // t=0
    CHECK(lines[1].find("0.0") != std::string::npos);
    CHECK(lines[1].find("42") != std::string::npos);
    CHECK(lines[1].find("1") != std::string::npos); // bool true
    CHECK(lines[1].find("hello") != std::string::npos);

    // t=0.1 (val1 becomes 11, val2 becomes 22)
    CHECK(lines[2].find("0.1") != std::string::npos);
    CHECK(lines[2].find("11") != std::string::npos);
    CHECK(lines[2].find("22") != std::string::npos);
    CHECK(lines[2].find("43") != std::string::npos);
    CHECK(lines[2].find("0") != std::string::npos); // bool false
    CHECK(lines[2].find("world") != std::string::npos);
    
    // Cleanup
    std::filesystem::remove(temp_file);
}

TEST_CASE("test_csv_recorder_config_filter", "[csv_recorder]") {
    csv_config config;
    CHECK(config.is_empty());
    CHECK(config.should_log("inst", "var")); // By default, logs everything

    config.register_variable("inst1", "var1");
    CHECK_FALSE(config.is_empty());

    CHECK(config.should_log("inst1", "var1"));
    CHECK_FALSE(config.should_log("inst1", "var2"));
    CHECK_FALSE(config.should_log("inst2", "var1"));
}

TEST_CASE("test_csv_recorder_config_load_errors", "[csv_recorder]") {
    csv_config config;
    
    // 1. Missing file
    std::filesystem::path missing_file = std::filesystem::temp_directory_path() / "missing_config.xml";
    CHECK_THROWS_AS(config.load(missing_file), std::runtime_error);

    // 2. Wrong extension
    std::filesystem::path wrong_ext_file = std::filesystem::temp_directory_path() / "config.txt";
    std::ofstream ofs_txt(wrong_ext_file);
    ofs_txt << "<test/>";
    ofs_txt.close();
    CHECK_THROWS_AS(config.load(wrong_ext_file), std::runtime_error);
    std::filesystem::remove(wrong_ext_file);

    // 3. Malformed XML
    std::filesystem::path malformed_xml = std::filesystem::temp_directory_path() / "malformed.xml";
    std::ofstream ofs_xml(malformed_xml);
    ofs_xml << "<nova:LogConfig><unclosed>";
    ofs_xml.close();
    CHECK_THROWS_AS(config.load(malformed_xml), std::runtime_error);
    std::filesystem::remove(malformed_xml);
}

TEST_CASE("test_csv_recorder_decimation", "[csv_recorder]") {
    std::filesystem::path temp_file = std::filesystem::temp_directory_path() / "test_nova_recorder_decimation.csv";
    if (std::filesystem::exists(temp_file)) std::filesystem::remove(temp_file);

    std::filesystem::path config_file = std::filesystem::temp_directory_path() / "config.xml";
    std::ofstream ofs(config_file);
    ofs << "<nova:LogConfig decimationFactor=\"2\">\n"
        << "  <nova:components name=\"dummy\">\n"
        << "    <nova:variable name=\"val1\"/>\n"
        << "  </nova:components>\n"
        << "</nova:LogConfig>\n";
    ofs.close();

    auto algo = std::make_unique<fixed_step_algorithm>(0.1, false);
    nova_engine sim(std::move(algo));
    sim.add_slave(std::make_unique<dummy_csv_slave>("dummy"));

    auto recorder = std::make_shared<csv_recorder>(temp_file.string(), config_file.string());
    sim.add_listener("csv", recorder);

    sim.init();
    sim.step(4); // t=0.0 -> t=0.4 (4 steps)
    sim.terminate();

    std::ifstream ifs(temp_file);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(ifs, line)) {
        lines.push_back(line);
    }
    ifs.close();

    // decimation=2 means row 0 (init), row 2 (step 2), row 4 (step 4) are logged.
    // That means header + 3 rows = 4 lines total.
    CHECK(lines.size() == 4);

    std::filesystem::remove(temp_file);
    std::filesystem::remove(config_file);
}

