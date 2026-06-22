#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "nova/components/structure/simulation_structure.hpp"
#include "nova/components/algorithm/fixed_step_algorithm.hpp"
#include "nova/components/logger/logger.hpp"
#include <filesystem>

using namespace nova_sim;

TEST_CASE("Nova System Integration: Controlled Temperature Flow")
{
    set_logging_level(log::level::debug);
    
    // 1. 构建仿真结构
    simulation_structure ss;
    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/2.0/20sim/ControlledTemperature.fmu";
    
    // 2. 添加模型实例
    REQUIRE(ss.add_model("room_heater", fmuPath));
    
    // 3. 配置初始参数 (验证重构后的 parameter_set 和 vector 存储)
    parameter_set params;
    // 设置初始参考温度为 298.15K (25 degC)
    params[{"room_heater", "Ramp.offset"}] = 298.15; 
    ss.add_parameter_set("initial_config", params);
    
    // 4. 加载算法与生成仿真对象
    // 使用 0.01s 的固定步进
    auto algo = std::make_unique<fixed_step_algorithm>(0.01);
    auto sim = ss.load(std::move(algo));
    REQUIRE(sim != nullptr);
    
    // 5. 初始化仿真 (验证参数应用逻辑)
    REQUIRE(sim->init("initial_config"));
    
    // 6. 验证初始状态获取
    auto t_ref_prop = sim->get_real_property("room_heater::Temperature_Reference");
    REQUIRE(t_ref_prop != nullptr);
    // 初始时刻参考温度应等于 offset
    CHECK_THAT(t_ref_prop->get_value(), Catch::Matchers::WithinRel(298.15));
    
    // 7. 执行仿真步进
    // 步进 100 步，总时长应为 1.0s
    sim->step(100);
    CHECK_THAT(sim->time(), Catch::Matchers::WithinRel(1.0));
    
    // 8. 验证仿真后的物理变化
    // 在 ControlledTemperature 中，随时间增加，Temperature_Room 应该发生变化
    auto t_room_prop = sim->get_real_property("room_heater::Temperature_Room");
    REQUIRE(t_room_prop != nullptr);
    double t_room_after = t_room_prop->get_value();
    
    // 逻辑验证：经过 1s 仿真，房间温度不应保持在初始绝对零度或未定义状态
    CHECK(t_room_after > 0.0);
    
    // 9. 终止仿真
    sim->terminate();
}
