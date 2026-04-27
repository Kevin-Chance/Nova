#include "fmi2_fmu.hpp"
#include "fmilibcpp/nova_fmi_standard.h"
#include <iostream>
#include <utility>
#include <cstdio>
#include <algorithm>
#include <cstdlib>

namespace nova_fmi {

typedef fmi2Status (STDCALL *fmi2GetBoolean_t)(fmi2Component, const fmi2ValueReference[], size_t, fmi2Boolean[]);
typedef fmi2Status (STDCALL *fmi2SetBoolean_t)(fmi2Component, const fmi2ValueReference[], size_t, const fmi2Boolean[]);
typedef fmi2Status (STDCALL *fmi2GetString_t)(fmi2Component, const fmi2ValueReference[], size_t, fmi2String[]);
typedef fmi2Status (STDCALL *fmi2SetString_t)(fmi2Component, const fmi2ValueReference[], size_t, const fmi2String[]);

typedef fmi2Status (STDCALL *fmi2GetFMUstate_t)(fmi2Component, fmi2FMUstate*);
typedef fmi2Status (STDCALL *fmi2SetFMUstate_t)(fmi2Component, fmi2FMUstate);
typedef fmi2Status (STDCALL *fmi2FreeFMUstate_t)(fmi2Component, fmi2FMUstate*);

struct fmi2CallbackFunctions {
    void* logger;
    void* allocateMemory;
    void* freeMemory;
    void* stepFinished;
    void* componentEnvironment;
};

static void STDCALL dummy_logger(fmi2ComponentEnvironment, fmi2String instanceName, fmi2Status status, fmi2String category, fmi2String message, ...) {
    std::cerr << "[FMI2 Logger] " << (instanceName ? instanceName : "unknown") << ": " << message << std::endl;
}
static void* STDCALL dummy_allocate(size_t nobj, size_t size) { return calloc(nobj, size); }
static void STDCALL dummy_free(void* obj) { free(obj); }
static void STDCALL dummy_step_finished(fmi2ComponentEnvironment, fmi2Status) {}

fmi2_fmu::fmi2_fmu(std::shared_ptr<NovaFmiLibrary> lib, std::unique_ptr<nova_sim::temp_dir> temp, model_description md, bool fmiLogging)
    : lib_(std::move(lib)), temp_(std::move(temp)), md_(std::move(md)), fmiLogging_(fmiLogging) {}

const model_description& fmi2_fmu::get_model_description() const { return md_; }

std::unique_ptr<NovaSlave> fmi2_fmu::new_instance(const std::string& instanceName) {
    auto s = std::make_unique<NovaSlave>(instanceName, md_, lib_);
    
    auto lib = lib_; 
    std::string res = temp_->path().string();
    std::replace(res.begin(), res.end(), '\\', '/');
    auto uri_res = "file:///" + res + "/resources";

    auto fmi2Instantiate = lib->getFunction<fmi2Instantiate_t>("fmi2Instantiate");
    auto fmi2FreeInstance = lib->getFunction<fmi2FreeInstance_t>("fmi2FreeInstance");
    
    if (!fmi2Instantiate || !fmi2FreeInstance) return nullptr;

    static fmi2CallbackFunctions callbacks = {
        (void*)dummy_logger,
        (void*)dummy_allocate,
        (void*)dummy_free,
        (void*)dummy_step_finished,
        nullptr
    };

    fmi2Component c = fmi2Instantiate(instanceName.c_str(), fmi2CoSimulation, md_.guid.c_str(), uri_res.c_str(), (const void*)&callbacks, fmi2False, fmiLogging_ ? fmi2True : fmi2False);
    if (!c) {
        c = fmi2Instantiate(instanceName.c_str(), fmi2CoSimulation, md_.guid.c_str(), (res + "/resources").c_str(), (const void*)&callbacks, fmi2False, fmiLogging_ ? fmi2True : fmi2False);
    }
    if (!c) return nullptr;

    s->component_ = std::shared_ptr<void>(c, [fmi2FreeInstance, lib](void* ptr) { 
        if (ptr && fmi2FreeInstance) fmi2FreeInstance(ptr); 
    });

    auto fmi2SetupExperiment = lib->getFunction<fmi2SetupExperiment_t>("fmi2SetupExperiment");
    auto fmi2EnterInitializationMode = lib->getFunction<fmi2EnterInitializationMode_t>("fmi2EnterInitializationMode");
    auto fmi2ExitInitializationMode = lib->getFunction<fmi2ExitInitializationMode_t>("fmi2ExitInitializationMode");
    auto fmi2DoStep = lib->getFunction<fmi2DoStep_t>("fmi2DoStep");
    auto fmi2Terminate = lib->getFunction<fmi2Terminate_t>("fmi2Terminate");
    auto fmi2Reset = lib->getFunction<fmi2Reset_t>("fmi2Reset");
    auto fmi2GetReal = lib->getFunction<fmi2GetReal_t>("fmi2GetReal");
    auto fmi2SetReal = lib->getFunction<fmi2SetReal_t>("fmi2SetReal");
    auto fmi2GetInteger = lib->getFunction<fmi2GetInteger_t>("fmi2GetInteger");
    auto fmi2SetInteger = lib->getFunction<fmi2SetInteger_t>("fmi2SetInteger");
    
    auto fmi2GetBoolean = lib->getFunction<fmi2GetBoolean_t>("fmi2GetBoolean");
    auto fmi2SetBoolean = lib->getFunction<fmi2SetBoolean_t>("fmi2SetBoolean");
    auto fmi2GetString = lib->getFunction<fmi2GetString_t>("fmi2GetString");
    auto fmi2SetString = lib->getFunction<fmi2SetString_t>("fmi2SetString");

    auto fmi2GetFMUstate = lib->getFunction<fmi2GetFMUstate_t>("fmi2GetFMUstate");
    auto fmi2SetFMUstate = lib->getFunction<fmi2SetFMUstate_t>("fmi2SetFMUstate");
    auto fmi2FreeFMUstate = lib->getFunction<fmi2FreeFMUstate_t>("fmi2FreeFMUstate");

    s->fmi.enter_init = [fmi2SetupExperiment, fmi2EnterInitializationMode](void* ch, double start, double stop, double tol) {
        if (fmi2SetupExperiment) fmi2SetupExperiment(ch, (tol > 0) ? fmi2True : fmi2False, tol, start, (stop > 0) ? fmi2True : fmi2False, stop);
        return fmi2EnterInitializationMode ? fmi2EnterInitializationMode(ch) == fmi2OK : false;
    };
    s->fmi.exit_init = [fmi2ExitInitializationMode](void* ch) { return fmi2ExitInitializationMode ? fmi2ExitInitializationMode(ch) == fmi2OK : false; };
    s->fmi.step = [fmi2DoStep](void* ch, double t, double dt) { return fmi2DoStep ? fmi2DoStep(ch, t, dt, fmi2True) == fmi2OK : false; };
    s->fmi.terminate = [fmi2Terminate](void* ch) { return fmi2Terminate ? fmi2Terminate(ch) == fmi2OK : false; };
    s->fmi.reset = [fmi2Reset](void* ch) { return fmi2Reset ? fmi2Reset(ch) == fmi2OK : false; };
    
    s->fmi.get_real = [fmi2GetReal](void* ch, const value_ref* vr, size_t n, double* v) { return fmi2GetReal ? fmi2GetReal(ch, (const fmi2ValueReference*)vr, n, v) == fmi2OK : false; };
    s->fmi.set_real = [fmi2SetReal](void* ch, const value_ref* vr, size_t n, const double* v) { return fmi2SetReal ? fmi2SetReal(ch, (const fmi2ValueReference*)vr, n, (const fmi2Real*)v) == fmi2OK : false; };
    s->fmi.get_int = [fmi2GetInteger](void* ch, const value_ref* vr, size_t n, int32_t* v) { return fmi2GetInteger ? fmi2GetInteger(ch, (const fmi2ValueReference*)vr, n, (fmi2Integer*)v) == fmi2OK : false; };
    s->fmi.set_int = [fmi2SetInteger](void* ch, const value_ref* vr, size_t n, const int32_t* v) { return fmi2SetInteger ? fmi2SetInteger(ch, (const fmi2ValueReference*)vr, n, (const fmi2Integer*)v) == fmi2OK : false; };
    s->fmi.get_bool = [fmi2GetBoolean](void* ch, const value_ref* vr, size_t n, int* v) { return fmi2GetBoolean ? fmi2GetBoolean(ch, (const fmi2ValueReference*)vr, n, (fmi2Boolean*)v) == fmi2OK : false; };
    s->fmi.set_bool = [fmi2SetBoolean](void* ch, const value_ref* vr, size_t n, const int* v) { return fmi2SetBoolean ? fmi2SetBoolean(ch, (const fmi2ValueReference*)vr, n, (const fmi2Boolean*)v) == fmi2OK : false; };
    s->fmi.get_str = [fmi2GetString](void* ch, const value_ref* vr, size_t n, char** v) { return fmi2GetString ? fmi2GetString(ch, (const fmi2ValueReference*)vr, n, (fmi2String*)v) == fmi2OK : false; };
    s->fmi.set_str = [fmi2SetString](void* ch, const value_ref* vr, size_t n, const char** v) { return fmi2SetString ? fmi2SetString(ch, (const fmi2ValueReference*)vr, n, (const fmi2String*)v) == fmi2OK : false; };

    s->fmi.get_state = [fmi2GetFMUstate](void* ch) { 
        fmi2FMUstate state = nullptr;
        if (fmi2GetFMUstate && fmi2GetFMUstate(ch, &state) == fmi2OK) return (void*)state;
        return (void*)nullptr;
    };
    s->fmi.set_state = [fmi2SetFMUstate](void* ch, void* state) {
        return fmi2SetFMUstate ? fmi2SetFMUstate(ch, (fmi2FMUstate)state) == fmi2OK : false;
    };
    s->fmi.free_state = [fmi2FreeFMUstate](void* ch, void* state) {
        fmi2FMUstate s = (fmi2FMUstate)state;
        return fmi2FreeFMUstate ? fmi2FreeFMUstate(ch, &s) == fmi2OK : false;
    };

    return s;
}

} // namespace nova_fmi
