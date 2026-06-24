#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <nova/engine/model_instance.hpp>

using namespace nova_sim;

class mock_model_instance : public model_instance {
public:
    mock_model_instance(const std::string& name) : model_instance(name) {
        doubleValue = 10.0;
        properties_.add_real_property("param1", property_t<double>::create(variable_identifier(name, "param1"), [&] { return doubleValue; }, [&](double v) { doubleValue = v; }));
        
        intValue = 5;
        properties_.add_int_property("param2", property_t<int>::create(variable_identifier(name, "param2"), [&] { return intValue; }, [&](int v) { intValue = v; }));
    }
    
    void enter_initialization_mode(double start = 0) override {}
    void exit_initialization_mode() override {}
    void step(double currentTime, double stepSize) override {}
    void terminate() override {}

    double doubleValue;
    int intValue;
};

TEST_CASE("test_model_instance_parameter_sets")
{
    mock_model_instance inst("test_inst");

    SECTION("test add_parameter_set full map") {
        std::unordered_map<std::string, scalar_value> pset;
        pset["param1"] = 25.0;
        pset["param2"] = 42;
        
        inst.add_parameter_set("set1", pset);
        
        bool applied = inst.apply_parameter_set("set1");
        CHECK(applied == true);
        
        inst.get_properties().apply_sets();
        
        CHECK_THAT(inst.doubleValue, Catch::Matchers::WithinRel(25.0));
        CHECK(inst.intValue == 42);
    }
    
    SECTION("test add_parameterset_entry individually") {
        inst.add_parameterset_entry("set2", "param1", 99.0);
        inst.add_parameterset_entry("set2", "param2", -10);
        
        bool applied = inst.apply_parameter_set("set2");
        CHECK(applied == true);
        
        inst.get_properties().apply_sets();
        
        CHECK_THAT(inst.doubleValue, Catch::Matchers::WithinRel(99.0));
        CHECK(inst.intValue == -10);
    }

    SECTION("test apply non-existent parameter set") {
        bool applied = inst.apply_parameter_set("non_existent");
        CHECK(applied == false);
        
        CHECK_THAT(inst.doubleValue, Catch::Matchers::WithinRel(10.0));
        CHECK(inst.intValue == 5);
    }
}
