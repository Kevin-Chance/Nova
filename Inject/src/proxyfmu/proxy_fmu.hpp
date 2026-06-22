
#ifndef PROXY_PROXY_FMU_FMU_HPP
#define PROXY_PROXY_FMU_FMU_HPP

#include "remote_info.hpp"

#include "nova_fmi/fmu.hpp"
#include "nova_fmi/model_description.hpp"
#include "nova_fmi/slave.hpp"

#include <optional>

namespace ecos::proxy
{

class proxy_fmu : public nova_fmi::fmu
{

public:
    explicit proxy_fmu(const std::filesystem::path& fmuPath, std::optional<remote_info> remote = std::nullopt);

    [[nodiscard]] const nova_fmi::model_description& get_model_description() const override;

    std::unique_ptr<nova_fmi::slave> new_instance(const std::string& instanceName) override;

    ~proxy_fmu() override = default;


private:
    const std::filesystem::path fmuPath_;
    const nova_fmi::model_description modelDescription_;

    const std::optional<remote_info> remote_;
};

} // namespace ecos::proxy


#endif // PROXY_PROXY_FMU_FMU_HPP
