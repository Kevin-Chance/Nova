#ifndef NOVA_CSV_RECORDER_HPP
#define NOVA_CSV_RECORDER_HPP

#include "nova/components/recorder/engine_observer.hpp"
#include "nova/engine/variable_identifier.hpp"
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

namespace nova_sim
{

class csv_config
{
public:
    void register_variable(const std::string& instance, const std::string& variable);
    void register_variable(const variable_identifier& id);
    void load(const std::filesystem::path& configPath);
    
    bool is_empty() const { return variable_register.empty(); }
    bool should_log(const std::string& inst, const std::string& var) const;

    size_t decimation_factor = 1;

private:
    std::vector<variable_identifier> variable_register;
};

class csv_recorder : public engine_observer
{
public:
    explicit csv_recorder(const std::string& filename);
    csv_recorder(const std::string& filename, const std::string& configPath);

    csv_config& config() { return config_; }
    std::string output_path() const { return filename_; }

    void post_init(nova_engine& sim) override;
    void post_step(nova_engine& sim) override;
    void post_terminate(nova_engine& sim) override;

private:
    void write_header(const nova_engine& sim);
    void write_row(const nova_engine& sim);

    std::string filename_;
    std::ofstream file_;
    csv_config config_;
    bool header_written_ = false;
};

} // namespace nova_sim

#endif // NOVA_CSV_RECORDER_HPP
