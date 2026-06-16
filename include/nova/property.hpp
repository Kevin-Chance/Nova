#ifndef NOVA_PROPERTY_HPP
#define NOVA_PROPERTY_HPP

#include "nova/variable_identifier.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>
#include <string>

namespace nova_sim
{

struct property
{
    const variable_identifier id;

    explicit property(variable_identifier id)
        : id(std::move(id))
    { }

    virtual void applySet() = 0;

    virtual ~property() = default;
};


template<class T>
struct property_t : property
{

    explicit property_t(
        const variable_identifier& id,
        const std::function<T()>& getter,
        const std::optional<std::function<void(const T&)>>& setter = std::nullopt)
        : property(id)
        , getter(getter)
        , setter(setter)
    { }

    T get_value() const
    {
        auto value = getter();
        if (outputModifier_) {
            value = outputModifier_->operator()(value);
        }
        return value;
    }

    void set_value(const T& value)
    {
        cachedSet = value;
    }

    void applySet() override
    {
        if (setter && cachedSet) {
            T value = cachedSet.value();
            if (inputModifier_) {
                value = inputModifier_->operator()(value);
            }
            setter->operator()(value);
            cachedSet = std::nullopt;
        }
    }

    void set_input_modifier(std::function<T(const T&)> modifier)
    {
        inputModifier_ = std::move(modifier);
    }

    void clear_input_modifier()
    {
        inputModifier_ = std::nullopt;
    }

    void set_output_modifier(std::function<T(const T&)> modifier)
    {
        outputModifier_ = std::move(modifier);
    }

    void clear_output_modifier()
    {
        outputModifier_ = std::nullopt;
    }

    static std::unique_ptr<property_t> create(
        const variable_identifier& id,
        const std::function<T()>& getter,
        const std::optional<std::function<void(const T&)>>& setter = std::nullopt)
    {
        return std::make_unique<property_t>(id, getter, setter);
    }

private:
    std::function<T()> getter;
    std::optional<std::function<void(const T&)>> setter;
    std::optional<T> cachedSet;

    std::optional<std::function<T(const T&)>> inputModifier_;
    std::optional<std::function<T(const T&)>> outputModifier_;
};

struct property_listener
{
    virtual void pre_step() { }
    virtual void post_step() { }
    virtual void pre_gets() { }
    virtual ~property_listener() = default;
};

class properties
{

public:
    void apply_sets()
    {
        for (auto& pair : intProperties_) pair.second->applySet();
        for (auto& pair : boolProperties_) pair.second->applySet();
        for (auto& pair : realProperties_) pair.second->applySet();
        for (auto& pair : stringProperties_) pair.second->applySet();
        for (auto& pair : vectorProperties_) pair.second->applySet();
    }

    void pre_step()
    {
        for (const auto& l : listeners_) {
            l->pre_step();
        }
    }

    void post_step()
    {
        for (const auto& l : listeners_) {
            l->post_step();
        }
    }

    void apply_gets()
    {
        for (const auto& l : listeners_) {
            l->pre_gets();
        }
    }

    // Week 5: Linear search implementation
    template<typename T>
    property_t<T>* find_property(const std::vector<std::pair<std::string, std::unique_ptr<property_t<T>>>>& container, const std::string& name)
    {
        for (const auto& p : container) {
            if (p.first == name) return p.second.get();
        }
        return nullptr;
    }

    template<typename T>
    const property_t<T>* find_property(const std::vector<std::pair<std::string, std::unique_ptr<property_t<T>>>>& container, const std::string& name) const
    {
        for (const auto& p : container) {
            if (p.first == name) return p.second.get();
        }
        return nullptr;
    }

    property_t<double>* get_real_property(const std::string& name) { return find_property(realProperties_, name); }
    const property_t<double>* get_real_property(const std::string& name) const { return find_property(realProperties_, name); }
    
    property_t<int>* get_int_property(const std::string& name) { return find_property(intProperties_, name); }
    const property_t<int>* get_int_property(const std::string& name) const { return find_property(intProperties_, name); }
    
    property_t<std::string>* get_string_property(const std::string& name) { return find_property(stringProperties_, name); }
    const property_t<std::string>* get_string_property(const std::string& name) const { return find_property(stringProperties_, name); }
    
    property_t<bool>* get_bool_property(const std::string& name) { return find_property(boolProperties_, name); }
    const property_t<bool>* get_bool_property(const std::string& name) const { return find_property(boolProperties_, name); }
    
    property_t<std::vector<double>>* get_vector_property(const std::string& name) { return find_property(vectorProperties_, name); }
    const property_t<std::vector<double>>* get_vector_property(const std::string& name) const { return find_property(vectorProperties_, name); }

    void add_real_property(const std::string& name, std::unique_ptr<property_t<double>> p) { realProperties_.push_back({name, std::move(p)}); }
    void add_int_property(const std::string& name, std::unique_ptr<property_t<int>> p) { intProperties_.push_back({name, std::move(p)}); }
    void add_string_property(const std::string& name, std::unique_ptr<property_t<std::string>> p) { stringProperties_.push_back({name, std::move(p)}); }
    void add_bool_property(const std::string& name, std::unique_ptr<property_t<bool>> p) { boolProperties_.push_back({name, std::move(p)}); }
    void add_vector_property(const std::string& name, std::unique_ptr<property_t<std::vector<double>>> p) { vectorProperties_.push_back({name, std::move(p)}); }

    [[nodiscard]] bool hasProperty(const std::string& name) const
    {
        for (const auto& p : intProperties_) if (p.first == name) return true;
        for (const auto& p : boolProperties_) if (p.first == name) return true;
        for (const auto& p : realProperties_) if (p.first == name) return true;
        for (const auto& p : stringProperties_) if (p.first == name) return true;
        for (const auto& p : vectorProperties_) if (p.first == name) return true;
        return false;
    }

    [[nodiscard]] std::vector<std::string> get_property_names() const
    {
        std::vector<std::string> names;
        for (const auto& p : intProperties_) names.push_back(p.first);
        for (const auto& p : boolProperties_) names.push_back(p.first);
        for (const auto& p : realProperties_) names.push_back(p.first);
        for (const auto& p : stringProperties_) names.push_back(p.first);
        for (const auto& p : vectorProperties_) names.push_back(p.first);
        return names;
    }

    void add_listener(std::unique_ptr<property_listener> l)
    {
        listeners_.emplace_back(std::move(l));
    }

    // Adaptors for existing code that expects maps (temporary or refactored)
    const std::vector<std::pair<std::string, std::unique_ptr<property_t<double>>>>& get_reals() const { return realProperties_; }
    const std::vector<std::pair<std::string, std::unique_ptr<property_t<int>>>>& get_integers() const { return intProperties_; }
    const std::vector<std::pair<std::string, std::unique_ptr<property_t<bool>>>>& get_booleans() const { return boolProperties_; }
    const std::vector<std::pair<std::string, std::unique_ptr<property_t<std::string>>>>& get_strings() const { return stringProperties_; }
    const std::vector<std::pair<std::string, std::unique_ptr<property_t<std::vector<double>>>>>& get_vectors() const { return vectorProperties_; }

private:
    std::vector<std::unique_ptr<property_listener>> listeners_;
    std::vector<std::pair<std::string, std::unique_ptr<property_t<int>>>> intProperties_;
    std::vector<std::pair<std::string, std::unique_ptr<property_t<bool>>>> boolProperties_;
    std::vector<std::pair<std::string, std::unique_ptr<property_t<double>>>> realProperties_;
    std::vector<std::pair<std::string, std::unique_ptr<property_t<std::string>>>> stringProperties_;
    std::vector<std::pair<std::string, std::unique_ptr<property_t<std::vector<double>>>>> vectorProperties_;
};

} // namespace nova_sim

#endif // NOVA_PROPERTY_HPP
