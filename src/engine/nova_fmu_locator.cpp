#include "nova/engine/nova_fmu_locator.hpp"
#include "fmi_model.hpp"
#include <filesystem>
#include <memory>

namespace nova_sim {

std::shared_ptr<model> NovaFmuLocator::resolve(const std::string& uri) {
    return resolve(std::filesystem::current_path(), uri);
}

std::shared_ptr<model> NovaFmuLocator::resolve(const std::filesystem::path& base, const std::string& uri) {
    std::filesystem::path fmuFile = uri;
    if (!fmuFile.is_absolute()) fmuFile = base / uri;
    
    if (std::filesystem::exists(fmuFile)) {
        try {
            return std::make_shared<fmi_model>(fmuFile);
        } catch (...) {
            return nullptr;
        }
    }
    return nullptr;
}

} // namespace nova_sim
