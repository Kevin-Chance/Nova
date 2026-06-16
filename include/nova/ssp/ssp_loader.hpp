
#ifndef NOVA_SSP_LOADER_HPP
#define NOVA_SSP_LOADER_HPP

#include "nova/structure/simulation_structure.hpp"

#include <filesystem>
#include <memory>

namespace nova_sim
{

[[nodiscard]] std::unique_ptr<simulation_structure> load_ssp(const std::filesystem::path& path);

} // namespace nova_sim

#endif // NOVA_SSP_LOADER_HPP
