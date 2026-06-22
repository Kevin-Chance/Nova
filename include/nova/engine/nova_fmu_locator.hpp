#ifndef NOVA_FMU_LOCATOR_HPP
#define NOVA_FMU_LOCATOR_HPP

#include "nova/engine/model.hpp"
#include <filesystem>
#include <memory>
#include <string>

namespace nova_sim
{

class NovaFmuLocator
{
public:
    static std::shared_ptr<model> resolve(const std::filesystem::path& base, const std::string& uri);
    static std::shared_ptr<model> resolve(const std::string& uri);
};

} // namespace nova_sim

#endif // NOVA_FMU_LOCATOR_HPP
