#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <nova/engine/nova_engine.hpp>
#include <nova/components/algorithm/fixed_step_algorithm.hpp>

using namespace nova_sim;

class mock_slave : public model_instance {
public:
    mock_slave(const std::string& name) : model_instance(name) {
        realVal = 0.0;
        properties_.add_real_property("realOut", property_t<double>::create(variable_identifier(name, "realOut"), [&]{ return realVal; }, [&](double v){ realVal = v; }));
        properties_.add_real_property("realIn", property_t<double>::create(variable_identifier(name, "realIn"), [&]{ return realVal; }, [&](double v){ realVal = v; }));
    }
    
    void enter_initialization_mode(double start = 0) override {}
    void exit_initialization_mode() override {}
    void step(double currentTime, double stepSize) override {
        if (instanceName_ == "source") {
            realVal += 1.0;
        }
    }
    void terminate() override {}
    void reset() override {
        realVal = 0.0;
    }

    double realVal;
};

TEST_CASE("test_nova_engine_connections")
{
    auto algo = std::make_unique<fixed_step_algorithm>(0.1, false);
    nova_engine sim(std::move(algo));

    auto src = std::make_unique<mock_slave>("source");
    auto dst = std::make_unique<mock_slave>("sink");
    auto src_ptr = src.get();
    auto dst_ptr = dst.get();

    sim.add_slave(std::move(src));
    sim.add_slave(std::move(dst));
    
    NovaDataLink link;
    link.src_instance = "source";
    link.src_variable = "realOut";
    link.dst_instance = "sink";
    link.dst_variable = "realIn";
    link.type = "real";
    sim.add_link(link);

    sim.init();

    sim.step();
    CHECK_THAT(src_ptr->realVal, Catch::Matchers::WithinRel(1.0));
    CHECK_THAT(dst_ptr->realVal, Catch::Matchers::WithinRel(1.0));

    sim.step();
    CHECK_THAT(src_ptr->realVal, Catch::Matchers::WithinRel(2.0));
    CHECK_THAT(dst_ptr->realVal, Catch::Matchers::WithinRel(2.0));

    auto fetched = sim.get_instance("sink");
    CHECK(fetched != nullptr);
    CHECK(fetched->instanceName() == "sink");
    
    auto invalid = sim.get_instance("not_exist");
    CHECK(invalid == nullptr);

    sim.reset();
    CHECK(src_ptr->realVal == 0.0);
    CHECK(sim.time() == 0.0);
}

class mock_listener : public engine_observer {
public:
    int step_calls = 0;
    void post_step(nova_engine& engine) override {
        step_calls++;
    }
};

TEST_CASE("test_nova_engine_listeners")
{
    auto algo = std::make_unique<fixed_step_algorithm>(0.1, false);
    nova_engine sim(std::move(algo));

    auto listener = std::make_shared<mock_listener>();
    sim.add_listener("mock_lis", listener);

    sim.init();
    sim.step(10); 

    CHECK(listener->step_calls == 10);
}

TEST_CASE("test_nova_engine_negative_paths")
{
    auto algo = std::make_unique<fixed_step_algorithm>(0.1, false);
    nova_engine sim(std::move(algo));

    // 1. Missing slaves but added links
    NovaDataLink link1;
    link1.src_instance = "ghost_src";
    link1.src_variable = "var";
    link1.dst_instance = "ghost_dst";
    link1.dst_variable = "var";
    link1.type = "real";
    sim.add_link(link1);
    
    // Engine should initialize cleanly without crash (logs a warning internally)
    sim.init();
    CHECK(sim.initialized() == true);
    sim.reset();

    // 2. Add real slaves but wrong variable mapping
    sim.add_slave(std::make_unique<mock_slave>("source"));
    sim.add_slave(std::make_unique<mock_slave>("sink"));

    NovaDataLink link_wrong_var;
    link_wrong_var.src_instance = "source";
    link_wrong_var.src_variable = "ghost_var"; // Does not exist
    link_wrong_var.dst_instance = "sink";
    link_wrong_var.dst_variable = "realIn";
    link_wrong_var.type = "real";
    sim.add_link(link_wrong_var);

    sim.init();
    CHECK(sim.initialized() == true);
    
    // 3. Test null slave boundary condition
    CHECK_THROWS_AS(sim.add_slave(nullptr), std::invalid_argument);

    // 4. Test step_until with past time (should log warning and not advance)
    sim.step_until(-1.0);
    CHECK(sim.time() == 0.0); // Time shouldn't move backward
}
