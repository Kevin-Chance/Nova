#ifndef NOVA_SIM_CT_COMMON_HPP
#define NOVA_SIM_CT_COMMON_HPP

#include "nova/simulation.hpp"
#include "nova/structure/simulation_structure.hpp"
#include "nova/algorithm/fixed_step_algorithm.hpp"
#include "nova/listeners/csv_writer.hpp"
#include "nova/logger/logger.hpp"
#include "nova/util/plotter.hpp"
#include <filesystem>

using namespace nova_sim;

inline double kelvi2deg(double k)
{
    return k - 273.15;
}

inline int run(const std::filesystem::path& fmuPath, bool remoting)
{
    // 1. 设置日志级别为 Debug (严格对齐)
    set_logging_level(log::level::debug);

    try {
        // 2. 构建仿真结构并添加模型
        simulation_structure ss;
        
        // 处理 remoting 逻辑 (严格对齐原版 URI 构造方式)
        std::string uri = remoting ? ("proxyfmu://localhost?file=" + fmuPath.string()) : fmuPath.string();
        ss.add_model("slave", uri);

        // 3. 配置算法 (1/50s 步长，严格对齐)
        auto algo = std::make_unique<fixed_step_algorithm>(1.0 / 50.0);
        auto sim = ss.load(std::move(algo));

        if (!sim) {
            return 1;
        }

        // 4. 设置 Output Modifiers (严格对齐功能覆盖)
        auto t_room = sim->get_real_property("slave::Temperature_Room");
        if (t_room) {
            t_room->set_output_modifier(kelvi2deg);
        }

        auto t_ref = sim->get_real_property("slave::Temperature_Reference");
        if (t_ref) {
            t_ref->set_output_modifier(kelvi2deg);
        }
// 5. 配置 CSV 监听器 (现在支持自动全量记录)
std::filesystem::create_directories(RESULT_FOLDER);
auto csvWriter = std::make_unique<csv_writer>(std::string(RESULT_FOLDER) + "/nova_controlled_temperature.csv");
const auto outputPath = csvWriter->output_path();
sim->add_listener("csv_writer", std::move(csvWriter));

        // 6. 执行仿真 (10s, 使用新实现的 step_for)
        sim->init();
        sim->step_for(10.0); 

        sim->terminate();

        log::info("Nova simulation finished. Output: {}/nova_controlled_temperature.csv", RESULT_FOLDER);

        plot_csv(outputPath, std::string(DATA_FOLDER) + "/fmus/2.0/20sim/ChartConfig.xml");

    } catch (const std::exception& ex) {
        log::err(ex.what());
        return 1;
    }

    return 0;
}

#endif // NOVA_SIM_CT_COMMON_HPP
