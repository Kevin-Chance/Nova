#ifndef NOVA_FMI_NOVA_SLAVE_HPP
#define NOVA_FMI_NOVA_SLAVE_HPP

#include "slave.hpp"
#include "nova_fmi_library.hpp"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace nova_fmi {

class NovaSlave : public slave {
public:
    model_description md;

    std::shared_ptr<void> component_;
    std::shared_ptr<NovaFmiLibrary> lib_;

    struct {
        std::function<bool(void*, double, double, double)> enter_init;
        std::function<bool(void*)> exit_init;
        std::function<bool(void*, double, double)> step;
        std::function<bool(void*)> terminate;
        std::function<bool(void*)> reset;
        std::function<bool(void*, const value_ref*, size_t, double*)> get_real;
        std::function<bool(void*, const value_ref*, size_t, const double*)> set_real;
        std::function<bool(void*, const value_ref*, size_t, int32_t*)> get_int;
        std::function<bool(void*, const value_ref*, size_t, const int32_t*)> set_int;
        std::function<bool(void*, const value_ref*, size_t, int*)> get_bool;
        std::function<bool(void*, const value_ref*, size_t, const int*)> set_bool;
        std::function<bool(void*, const value_ref*, size_t, char**)> get_str;
        std::function<bool(void*, const value_ref*, size_t, const char**)> set_str;
        
        std::function<void*(void*)> get_state;
        std::function<bool(void*, void*)> set_state;
        std::function<bool(void*, void*)> free_state;
    } fmi;

    NovaSlave(std::string name, model_description desc, std::shared_ptr<NovaFmiLibrary> lib)
        : slave(std::move(name)), md(std::move(desc)), lib_(std::move(lib)) {}

    virtual ~NovaSlave() = default;

    virtual const model_description& get_model_description() const { return md; }

    virtual bool enter_initialization_mode(double start = 0, double stop = 0, double tol = 0) {
        return fmi.enter_init ? fmi.enter_init(component_.get(), start, stop, tol) : false;
    }
    virtual bool exit_initialization_mode() { return fmi.exit_init ? fmi.exit_init(component_.get()) : false; }
    virtual bool step(double t, double dt) { return fmi.step ? fmi.step(component_.get(), t, dt) : false; }
    virtual bool terminate() { return fmi.terminate ? fmi.terminate(component_.get()) : false; }
    virtual bool reset() { return fmi.reset ? fmi.reset(component_.get()) : false; }
    virtual void freeInstance() { component_.reset(); }

    virtual void* get_state() { return fmi.get_state ? fmi.get_state(component_.get()) : nullptr; }
    virtual bool set_state(void* state) { return fmi.set_state ? fmi.set_state(component_.get(), state) : false; }
    virtual bool free_state(void* state) { return fmi.free_state ? fmi.free_state(component_.get(), state) : false; }

    // Batch interfaces
    virtual bool get_real(const std::vector<value_ref>& vr, std::vector<double>& values) {
        return fmi.get_real ? fmi.get_real(component_.get(), vr.data(), vr.size(), values.data()) : false;
    }
    virtual bool set_real(const std::vector<value_ref>& vr, const std::vector<double>& values) {
        return fmi.set_real ? fmi.set_real(component_.get(), vr.data(), vr.size(), values.data()) : false;
    }
    virtual bool get_integer(const std::vector<value_ref>& vr, std::vector<int32_t>& values) {
        return fmi.get_int ? fmi.get_int(component_.get(), vr.data(), vr.size(), values.data()) : false;
    }
    virtual bool set_integer(const std::vector<value_ref>& vr, const std::vector<int32_t>& values) {
        return fmi.set_int ? fmi.set_int(component_.get(), vr.data(), vr.size(), values.data()) : false;
    }
    virtual bool get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values) {
        if (!fmi.get_bool) return false;
        std::vector<int> tmp(vr.size());
        if (fmi.get_bool(component_.get(), vr.data(), vr.size(), tmp.data())) {
            values.clear();
            for (int v : tmp) values.push_back(v != 0);
            return true;
        }
        return false;
    }
    virtual bool set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values) {
        if (!fmi.set_bool) return false;
        std::vector<int> tmp;
        for (bool v : values) tmp.push_back(v ? 1 : 0);
        return fmi.set_bool(component_.get(), vr.data(), vr.size(), tmp.data());
    }
    virtual bool get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values) {
        std::vector<char*> ptrs(vr.size());
        if (fmi.get_str && fmi.get_str(component_.get(), vr.data(), vr.size(), ptrs.data())) {
            for(size_t i=0; i<vr.size(); ++i) values[i] = ptrs[i];
            return true;
        }
        return false;
    }
    virtual bool set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values) {
        std::vector<const char*> ptrs(vr.size());
        for(size_t i=0; i<vr.size(); ++i) ptrs[i] = values[i].c_str();
        return fmi.set_str ? fmi.set_str(component_.get(), vr.data(), vr.size(), ptrs.data()) : false;
    }

    // Scalar versions
    double get_real(value_ref vr) { double v = 0; fmi.get_real(component_.get(), &vr, 1, &v); return v; }
    void set_real(value_ref vr, double v) { fmi.set_real(component_.get(), &vr, 1, &v); }
    int32_t get_integer(value_ref vr) { int32_t v = 0; fmi.get_int(component_.get(), &vr, 1, &v); return v; }
    void set_integer(value_ref vr, int32_t v) { fmi.set_int(component_.get(), &vr, 1, &v); }
};

} // namespace nova_fmi

#endif // NOVA_FMI_NOVA_SLAVE_HPP
