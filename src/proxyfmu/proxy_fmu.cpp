
#include "proxy_fmu.hpp"

#include "proxy_slave.hpp"

#include "fmilibcpp/fmu.hpp"

#include <memory>
#include <utility>


namespace nova_sim::proxy
{

proxy_fmu::proxy_fmu(const std::filesystem::path& fmuPath, std::optional<remote_info> remote)
    : fmuPath_(fmuPath)
    , modelDescription_(nova_fmi::loadFmu(fmuPath)->get_model_description())
    , remote_(std::move(remote))
{
    if (!exists(fmuPath)) {
        throw std::runtime_error("No such file: " + absolute(fmuPath).string() + "!");
    }
}

const nova_fmi::model_description& proxy_fmu::get_model_description() const
{
    return modelDescription_;
}

std::unique_ptr<nova_fmi::slave> proxy_fmu::new_instance(const std::string& instanceName)
{
    return std::make_unique<proxy_slave>(fmuPath_, instanceName, modelDescription_, remote_);
}

} // namespace proxyfmu
