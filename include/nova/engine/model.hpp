#ifndef NOVA_MODEL_HPP
#define NOVA_MODEL_HPP

#include "nova/engine/model_instance.hpp"
#include "../../src/nova_fmi/model_description.hpp"
#include <memory>
#include <optional>

namespace nova_sim
{

class model
{
public:
    virtual std::unique_ptr<model_instance> instantiate(const std::string& instanceName, std::optional<double> stepSizeHint = std::nullopt) = 0;
    virtual const nova_fmi::model_description& get_model_description() const = 0;
    virtual ~model() = default;
};

} // namespace nova_sim

#endif // NOVA_MODEL_HPP
