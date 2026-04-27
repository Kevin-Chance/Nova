#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/model_resolver.hpp"
#include "ecos/simulation.hpp"
#include "ecos/structure/simulation_structure.hpp"
#include <ecos/listeners/csv_writer.hpp>

#include <iostream>

using namespace ecos;
//  用于存放配置的结构体
struct SimConfig
{
    std::string fmbs_path;
    std::string mworks_path;
    double time;
    double dt;
};

void addConnections(simulation_structure& ss);
void addParmeterSets(simulation_structure& ss);
void run(simulation_structure& ss, double time, double dt);
// 从.txt文件读取并解析配置
bool readConfig(const std::string& cfg_file, SimConfig& cfg)
{
    std::ifstream infile(cfg_file);
    if (!infile.is_open()) {
        std::cerr << "无法打开配置文件：" << cfg_file << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(infile, line)) {
        // 去除注释和空行
        auto pos = line.find('#');
        if (pos != std::string::npos)
            line = line.substr(0, pos);
        if (line.empty())
            continue;

        // 去除前后空格
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // 查找等号的位置
        pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // 去除前后空格
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            // 解析键值对
            if (key == "FMBS_PATH")
                cfg.fmbs_path = value;
            else if (key == "MWorks_PATH")
                cfg.mworks_path = value;
            else if (key == "TIME")
                cfg.time = std::stod(value);
            else if (key == "DT")
                cfg.dt = std::stod(value);
            else
                std::cerr << "未知配置项：" << key << std::endl;

            //}
        }
    }

    std::cout << "path1 " << cfg.fmbs_path << std::endl;
    std::cout << "path2 " << cfg.mworks_path << std::endl;
    std::cout << "time " << cfg.time << std::endl;
    std::cout << "dt " << cfg.dt << std::endl;
    infile.close();
    return true;
}


int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file_path>" << std::endl;
        return 1;
    }
    std::string filePath = argv[1];
    std ::cout << "filePath" << filePath << std::endl;

    set_logging_level(log::level::debug);


    SimConfig cfg;
    // std::string cfg_file = "config.txt";
    std::string cfg_file = filePath;
    if (!readConfig(cfg_file, cfg)) return -1;
    simulation_structure ss;
    std::string fmbsPath = cfg.fmbs_path;
    fmbsPath.erase(std::remove(fmbsPath.begin(), fmbsPath.end(), '\"'), fmbsPath.end());

    std::filesystem::path fmuDir1 = fmbsPath;

    std::string fmuPath = fmuDir1.generic_string();

    std::string mworksPath = cfg.mworks_path;
    mworksPath.erase(std::remove(mworksPath.begin(), mworksPath.end(), '\"'), mworksPath.end());


    std::filesystem::path fmuDir2 = mworksPath;

    std::string fmuPath2 = fmuDir2.generic_string(); // 使用 '/' 分隔


    ss.add_model("FMBS", fmuPath);
    ss.add_model("MWorks", fmuPath2);

    addParmeterSets(ss);
    addConnections(ss);

    run(ss, cfg.time, cfg.dt);
    return 0;
}


void addConnections(simulation_structure& ss)
{
    ss.make_connection<double>("MWorks::cm2_output", "FMBS::cm_er_pos");
    ss.make_connection<double>("MWorks::cm1_output", "FMBS::cm_yi_pos");
    ss.make_connection<double>("MWorks::tx_output", "FMBS::tx_pos");
    ss.make_connection<double>("MWorks::db_output", "FMBS::db_pos");
}

void addParmeterSets(simulation_structure& ss)
{
    std::map<variable_identifier, scalar_value> map;
    ss.add_parameter_set("initialValues", map);
}

void run(simulation_structure& ss, double time, double dt)
{
    set_logging_level(log::level::debug);

    try {
        // double stepsize = 0.2;
        auto sim = ss.load(std::make_unique<fixed_step_algorithm>(dt));

        auto curve_cm2_output = std::make_unique<csv_writer>("results/cm2_output.curve");
        auto curve_cm1_output = std::make_unique<csv_writer>("results/cm1_output.curve");
        auto curve_tx_output = std::make_unique<csv_writer>("results/tx_output.curve");
        auto curve_db_output = std::make_unique<csv_writer>("results/db_output.curve");
        auto curve_sc_time = std::make_unique<csv_writer>("results/sc_time.curve");
        auto curve_sh_time = std::make_unique<csv_writer>("results/sh_time.curve");

        auto curve_cm_er_pos = std::make_unique<csv_writer>("results/cm_er_pos.curve");
        auto curve_cm_yi_pos = std::make_unique<csv_writer>("results/cm_yi_pos.curve");
        auto curve_db_pos = std::make_unique<csv_writer>("results/db_pos.curve");
        auto curve_tx_pos = std::make_unique<csv_writer>("results/tx_pos.curve");
        auto curve_cm_er_force = std::make_unique<csv_writer>("results/cm_er_force.curve");
        auto curve_cm_yi_force = std::make_unique<csv_writer>("results/cm_yi_force.curve");
        auto curve_db_force = std::make_unique<csv_writer>("results/db_force.curve");
        auto curve_tx_force = std::make_unique<csv_writer>("results/tx_force.curve");


        csv_config& config_cm2_output = curve_cm2_output->config();
        config_cm2_output.register_variable("Mworks::cm2_output");
        sim->add_listener("csv_writer1", std::move(curve_cm2_output));

        csv_config& config_cm1_output = curve_cm1_output->config();
        config_cm1_output.register_variable("Mworks::cm1_output");
        sim->add_listener("csv_writer2", std::move(curve_cm1_output));

        csv_config& config_tx_output = curve_tx_output->config();
        config_tx_output.register_variable("Mworks::tx_output");
        sim->add_listener("csv_writer3", std::move(curve_tx_output));

        csv_config& config_db_output = curve_db_output->config();
        config_db_output.register_variable("Mworks::db_output");
        sim->add_listener("csv_writer4", std::move(curve_db_output));

        csv_config& config_sc_time = curve_sc_time->config();
        config_sc_time.register_variable("Mworks::sc_time");
        sim->add_listener("csv_writer5", std::move(curve_sc_time));

        csv_config& config_sh_time = curve_sh_time->config();
        config_sh_time.register_variable("Mworks::sh_time");
        sim->add_listener("csv_writer6", std::move(curve_sh_time));


        csv_config& config_cm_er_pos = curve_cm_er_pos->config();
        config_cm_er_pos.register_variable("FMBS::cm_er_pos");
        sim->add_listener("csv_writer7", std::move(curve_cm_er_pos));

        csv_config& config_cm_yi_pos = curve_cm_yi_pos->config();
        config_cm_yi_pos.register_variable("FMBS::cm_yi_pos");
        sim->add_listener("csv_writer8", std::move(curve_cm_yi_pos));

        csv_config& config_db_pos = curve_db_pos->config();
        config_db_pos.register_variable("FMBS::db_pos");
        sim->add_listener("csv_writer9", std::move(curve_db_pos));

        csv_config& config_tx_pos = curve_tx_pos->config();
        config_tx_pos.register_variable("FMBS::tx_pos");
        sim->add_listener("csv_writer10", std::move(curve_tx_pos));

        csv_config& config_cm_er_force = curve_cm_er_force->config();
        config_cm_er_force.register_variable("FMBS::cm_er_force");
        sim->add_listener("csv_writer11", std::move(curve_cm_er_force));

        csv_config& config_cm_yi_force = curve_cm_yi_force->config();
        config_cm_yi_force.register_variable("FMBS::cm_yi_force");
        sim->add_listener("csv_writer12", std::move(curve_cm_yi_force));

        csv_config& config_db_force = curve_db_force->config();
        config_db_force.register_variable("FMBS::db_force");
        sim->add_listener("csv_writer13", std::move(curve_db_force));

        csv_config& config_tx_force = curve_tx_force->config();
        config_tx_force.register_variable("FMBS::tx_force");
        sim->add_listener("csv_writer14", std::move(curve_tx_force));

        sim->init("initialValues");
        sim->step_until(time);
        sim->terminate();
    } catch (const std::exception& ex) {

        log::err(ex.what());
    }
}
