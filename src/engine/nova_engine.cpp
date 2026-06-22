#include "nova/engine/nova_engine.hpp"
#include "nova/components/recorder/engine_observer.hpp"
#include "nova/components/logger/logger.hpp"
#include "nova/engine/property.hpp"
#include "scenario.hpp"
#include "nova/components/algorithm/fixed_step_algorithm.hpp"

#include <execution>
#include <ranges>

namespace nova_sim {

struct nova_engine::Impl
{
    double lastDelta_{};
    double currentTime_{0};
    bool initialized_{false};
    bool terminated_{false};
    unsigned long num_iterations_{0};

    scenario scenario_;
    std::unique_ptr<algorithm> algorithm_;
    std::vector<std::unique_ptr<model_instance>> instances_;
    std::unordered_map<std::string, std::shared_ptr<engine_observer>> listeners_;

    nova_engine& sim_;

    explicit Impl(nova_engine& sim, std::unique_ptr<algorithm> algorithm)
        : algorithm_(std::move(algorithm))
        , sim_(sim)
    { }

    void transfer_data()
    {
        for (const auto& link : sim_.links_) {
            auto src_inst = sim_.get_instance(link.src_instance);
            auto dst_inst = sim_.get_instance(link.dst_instance);
            if (!src_inst || !dst_inst) continue;

            auto& src_props = src_inst->get_properties();
            auto& dst_props = dst_inst->get_properties();

            if (link.type == "real") {
                auto sp = src_props.get_real_property(link.src_variable);
                auto dp = dst_props.get_real_property(link.dst_variable);
                if (sp && dp) {
                    double val = sp->get_value();
                    if (link.real_modifier) val = link.real_modifier(val);
                    dp->set_value(val);
                }
            }
        }
    }

    double step(unsigned int numStep)
    {
        if (!initialized_) throw std::runtime_error("init() has not been invoked!");
        double newT{};
        for (unsigned i = 0; i < numStep; ++i) {
            for (auto& listener : listeners_ | std::views::values) listener->pre_step(sim_);
            scenario_.apply(currentTime_);
            
            // Sync links is now handled inside algorithm_->step to allow sequential execution
            newT = algorithm_->step(currentTime_, sim_);

            lastDelta_ = newT - currentTime_;
            currentTime_ = newT;
            ++num_iterations_;
            for (auto& listener : listeners_ | std::views::values) listener->post_step(sim_);
        }
        return currentTime_;
    }

    void init(std::optional<double> startTime, const std::optional<std::string>& parameterSet)
    {
        if (initialized_) return;
        currentTime_ = startTime.value_or(0);
        
        auto fixed_algo = dynamic_cast<fixed_step_algorithm*>(algorithm_.get());
        for (auto& instance : instances_) {
            if (fixed_algo) fixed_algo->model_instance_added(instance.get());
        }

        // 1. Enter initialization
        for (auto& instance : instances_) {
            instance->enter_initialization_mode(currentTime_);
        }

        // 2. Apply parameter set during initialization (FMI 2.0) 
        // or immediately after initializeSlave (FMI 1.0) to match Nova behavior
        if (parameterSet) {
            for (auto& instance : instances_) {
                instance->apply_parameter_set(*parameterSet);
            }
        }

        // 3. Propagate initial values and parameters
        for (int i = 0; i < 3; ++i) {
            for (auto& instance : instances_) {
                instance->get_properties().apply_sets();
                instance->get_properties().apply_gets();
            }
            sim_.sync_links();
        }

        // 4. Exit initialization
        for (auto& instance : instances_) {
            instance->exit_initialization_mode();
        }

        // 5. Final sync after initialization to ensure all outputs are read and propagated
        for (int i = 0; i < 3; ++i) {
            for (auto& instance : instances_) {
                instance->get_properties().apply_gets();
            }
            sim_.sync_links();
            for (auto& instance : instances_) {
                instance->get_properties().apply_sets();
            }
        }

        algorithm_->initialize(currentTime_);
        initialized_ = true;

        // 触发初始化后监听器通知
        for (auto& listener : listeners_ | std::views::values) {
            listener->post_init(sim_);
        }
    }

    void terminate()
    {
        if (terminated_) return;
        for (auto& instance : instances_) { try { instance->terminate(); } catch(...) {} }
        terminated_ = true;
    }

    void load_scenario(const std::filesystem::path& config) { scenario_.load(config, instances_); }
};

nova_engine::nova_engine(std::unique_ptr<algorithm> algorithm)
    : pimpl_(std::make_unique<Impl>(*this, std::move(algorithm))) {}

nova_engine::~nova_engine() = default;

double nova_engine::time() const { return pimpl_->currentTime_; }
unsigned long nova_engine::iterations() const { return pimpl_->num_iterations_; }
bool nova_engine::initialized() const { return pimpl_->initialized_; }
bool nova_engine::terminated() const { return pimpl_->terminated_; }

void nova_engine::init(std::optional<double> startTime, const std::optional<std::string>& parameterSet)
{ pimpl_->init(startTime, parameterSet); }

double nova_engine::step(unsigned int numStep) { return pimpl_->step(numStep); }

void nova_engine::step_until(double t)
{
    if (t <= pimpl_->currentTime_) {
        log::warn("Input time {} is not greater than the current nova_engine time {}. Simulation will not progress.", t, pimpl_->currentTime_);
    } else {
        // 步进直到达到或超过目标时间
        while (pimpl_->currentTime_ + pimpl_->lastDelta_ < t + 1e-9) { // 考虑浮点数精度
            step();
        }
    }
}

void nova_engine::step_for(double dt)
{
    step_until(pimpl_->currentTime_ + dt);
}

void nova_engine::terminate() { pimpl_->terminate(); }

void nova_engine::reset()
{
    pimpl_->initialized_ = false;
    pimpl_->terminated_ = false;
    pimpl_->currentTime_ = 0;
    pimpl_->num_iterations_ = 0;
    for (auto& instance : pimpl_->instances_) instance->reset();
}

void nova_engine::add_slave(std::unique_ptr<model_instance> slave)
{ 
    if (!slave) throw std::invalid_argument("Null slave");
    pimpl_->instances_.push_back(std::move(slave)); 
}

void nova_engine::add_listener(const std::string& name, std::shared_ptr<engine_observer> listener)
{ pimpl_->listeners_[name] = listener; }

void nova_engine::remove_listener(const std::string& name)
{ pimpl_->listeners_.erase(name); }

model_instance* nova_engine::get_instance(const std::string& name) const
{
    for (auto& instance : pimpl_->instances_) {
        if (instance->instanceName() == name) return instance.get();
    }
    return nullptr;
}

const std::vector<std::unique_ptr<model_instance>>& nova_engine::get_instances() const
{ return pimpl_->instances_; }

std::vector<variable_identifier> nova_engine::identifiers() const
{
    std::vector<variable_identifier> ids;
    for (const auto& inst : pimpl_->instances_) {
        for (const auto& name : inst->get_properties().get_property_names()) {
            ids.push_back({inst->instanceName(), name});
        }
    }
    return ids;
}

property_t<double>* nova_engine::get_real_property(const variable_identifier& identifier) const
{
    auto inst = get_instance(identifier.instanceName);
    return inst ? inst->get_properties().get_real_property(identifier.variableName) : nullptr;
}

property_t<int>* nova_engine::get_int_property(const variable_identifier& identifier) const
{
    auto inst = get_instance(identifier.instanceName);
    return inst ? inst->get_properties().get_int_property(identifier.variableName) : nullptr;
}

property_t<bool>* nova_engine::get_bool_property(const variable_identifier& identifier) const
{
    auto inst = get_instance(identifier.instanceName);
    return inst ? inst->get_properties().get_bool_property(identifier.variableName) : nullptr;
}

property_t<std::string>* nova_engine::get_string_property(const variable_identifier& identifier) const
{
    auto inst = get_instance(identifier.instanceName);
    return inst ? inst->get_properties().get_string_property(identifier.variableName) : nullptr;
}

property_t<std::vector<double>>* nova_engine::get_vector_property(const variable_identifier& identifier) const
{
    auto inst = get_instance(identifier.instanceName);
    return inst ? inst->get_properties().get_vector_property(identifier.variableName) : nullptr;
}

void nova_engine::sync_links()
{
    pimpl_->transfer_data();
}

void nova_engine::load_scenario(const std::filesystem::path& config) { pimpl_->load_scenario(config); }

} // namespace nova_sim
