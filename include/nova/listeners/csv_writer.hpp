#ifndef NOVA_CSV_WRITER_HPP
#define NOVA_CSV_WRITER_HPP

#include "nova/listeners/simulation_listener.hpp"
#include "nova/variable_identifier.hpp"
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

class csv_writer : public simulation_listener
{
public:
    explicit csv_writer(const std::string& filename);
    csv_writer(const std::string& filename, const std::string& configPath);

    csv_config& config() { return config_; }
    std::string output_path() const { return filename_; }

    void post_init(simulation& sim) override;
    void post_step(simulation& sim) override;
    void post_terminate(simulation& sim) override;

private:
    void write_header(const simulation& sim);
    void write_row(const simulation& sim);

    std::string filename_;
    std::ofstream file_;
    csv_config config_;
    bool header_written_ = false;
};

} // namespace nova_sim

#endif // NOVA_CSV_WRITER_HPP
