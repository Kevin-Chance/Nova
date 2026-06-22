#ifndef NOVA_FMI_SLAVE_HPP
#define NOVA_FMI_SLAVE_HPP

#include "model_description.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace nova_fmi {

using value_ref = uint32_t;

class slave {
public:
    const std::string instanceName;

    explicit slave(std::string name) : instanceName(std::move(name)) {}
    virtual ~slave() = default;

    virtual const model_description& get_model_description() const = 0;

    virtual bool enter_initialization_mode(double start = 0, double stop = 0, double tol = 0) = 0;
    virtual bool exit_initialization_mode() = 0;
    virtual bool step(double t, double dt) = 0;
    virtual bool terminate() = 0;
    virtual bool reset() = 0;
    virtual void freeInstance() = 0;

    virtual void* get_state() { return nullptr; }
    virtual bool set_state(void* state) { return false; }
    virtual bool free_state(void* state) { return false; }

    virtual bool get_real(const std::vector<value_ref>& vr, std::vector<double>& values) = 0;
    virtual bool set_real(const std::vector<value_ref>& vr, const std::vector<double>& values) = 0;
    virtual bool get_integer(const std::vector<value_ref>& vr, std::vector<int32_t>& values) = 0;
    virtual bool set_integer(const std::vector<value_ref>& vr, const std::vector<int32_t>& values) = 0;
    virtual bool get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values) = 0;
    virtual bool set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values) = 0;
    virtual bool get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values) = 0;
    virtual bool set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values) = 0;
};

} // namespace nova_fmi

#endif // NOVA_FMI_SLAVE_HPP
