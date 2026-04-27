#include "ecos/simulation.hpp"
#include "ecos/listeners/simulation_listener.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/property.hpp"
#include "ecos/scenario/scenario.hpp"
#include "ecos/algorithm/fixed_step_algorithm.hpp"

#include <execution>
#include <ranges>

namespace nova_sim {

struct simulation::Impl
{
    double lastDelta_{};
    double currentTime_{0};
    bool initialized_{false};
    bool terminated_{false};
    unsigned long num_iterations_{0};

    scenario scenario_;
    std::unique_ptr<algorithm> algorithm_;
    std::vector<std::unique_ptr<model_instance>> instances_;
    std::unordered_map<std::string, std::shared_ptr<simulation_listener>> listeners_;

    simulation& sim_;

    explicit Impl(simulation& sim, std::unique_ptr<algorithm> algorithm)
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
                if (sp && dp) dp->set_value(sp->get_value());
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
            
            for (auto& inst : instances_) inst->get_properties().apply_sets();
            newT = algorithm_->step(currentTime_);
            for (auto& inst : instances_) inst->get_properties().apply_gets();

            transfer_data();
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
            if (parameterSet) instance->apply_parameter_set(*parameterSet);
            instance->enter_initialization_mode(currentTime_);
        }
        for (auto& instance : instances_) {
            instance->get_properties().apply_sets();
            instance->exit_initialization_mode();
            instance->get_properties().apply_gets();
        }
        algorithm_->initialize(currentTime_);
        initialized_ = true;

        // 关键点：触发初始化后监听器通知
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

simulation::simulation(std::unique_ptr<algorithm> algorithm)
    : pimpl_(std::make_unique<Impl>(*this, std::move(algorithm))) {}

simulation::~simulation() = default;

double simulation::time() const { return pimpl_->currentTime_; }
unsigned long simulation::iterations() const { return pimpl_->num_iterations_; }
bool simulation::initialized() const { return pimpl_->initialized_; }
bool simulation::terminated() const { return pimpl_->terminated_; }

void simulation::init(std::optional<double> startTime, const std::optional<std::string>& parameterSet)
{ pimpl_->init(startTime, parameterSet); }

double simulation::step(unsigned int numStep) { return pimpl_->step(numStep); }

void simulation::step_until(double t)
{
    if (t <= pimpl_->currentTime_) {
        log::warn("Input time {} is not greater than the current simulation time {}. Simulation will not progress.", t, pimpl_->currentTime_);
    } else {
        // 步进直到达到或超过目标时间
        while (pimpl_->currentTime_ + pimpl_->lastDelta_ < t + 1e-9) { // 考虑浮点数精度
            step();
        }
    }
}

void simulation::step_for(double dt)
{
    step_until(pimpl_->currentTime_ + dt);
}

void simulation::terminate() { pimpl_->terminate(); }

void simulation::reset()
{
    pimpl_->initialized_ = false;
    pimpl_->terminated_ = false;
    pimpl_->currentTime_ = 0;
    pimpl_->num_iterations_ = 0;
    for (auto& instance : pimpl_->instances_) instance->reset();
}

void simulation::add_slave(std::unique_ptr<model_instance> slave)
{ 
    if (!slave) throw std::invalid_argument("Null slave");
    pimpl_->instances_.push_back(std::move(slave)); 
}

void simulation::add_listener(const std::string& name, std::shared_ptr<simulation_listener> listener)
{ pimpl_->listeners_[name] = listener; }

void simulation::remove_listener(const std::string& name)
{ pimpl_->listeners_.erase(name); }

model_instance* simulation::get_instance(const std::string& name) const
{
    for (auto& instance : pimpl_->instances_) {
        if (instance->instanceName() == name) return instance.get();
    }
    return nullptr;
}

const std::vector<std::unique_ptr<model_instance>>& simulation::get_instances() const
{ return pimpl_->instances_; }

std::vector<variable_identifier> simulation::identifiers() const
{
    std::vector<variable_identifier> ids;
    for (const auto& inst : pimpl_->instances_) {
        for (const auto& name : inst->get_properties().get_property_names()) {
            ids.push_back({inst->instanceName(), name});
        }
    }
    return ids;
}

property_t<double>* simulation::get_real_property(const variable_identifier& identifier) const
{
    auto inst = get_instance(identifier.instanceName);
    return inst ? inst->get_properties().get_real_property(identifier.variableName) : nullptr;
}

property_t<int>* simulation::get_int_property(const variable_identifier& identifier) const
{
    auto inst = get_instance(identifier.instanceName);
    return inst ? inst->get_properties().get_int_property(identifier.variableName) : nullptr;
}

property_t<bool>* simulation::get_bool_property(const variable_identifier& identifier) const
{
    auto inst = get_instance(identifier.instanceName);
    return inst ? inst->get_properties().get_bool_property(identifier.variableName) : nullptr;
}

property_t<std::string>* simulation::get_string_property(const variable_identifier& identifier) const
{
    auto inst = get_instance(identifier.instanceName);
    return inst ? inst->get_properties().get_string_property(identifier.variableName) : nullptr;
}

property_t<std::vector<double>>* simulation::get_vector_property(const variable_identifier& identifier) const
{
    auto inst = get_instance(identifier.instanceName);
    return inst ? inst->get_properties().get_vector_property(identifier.variableName) : nullptr;
}

void simulation::sync_links()
{
    pimpl_->transfer_data();
}

void simulation::load_scenario(const std::filesystem::path& config) { pimpl_->load_scenario(config); }

} // namespace nova_sim
