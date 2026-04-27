#include "fmi3_fmu.hpp"
#include <utility>
#include <cstdio>

#ifdef _WIN32
#define NOVA_STDCALL __stdcall
#else
#define NOVA_STDCALL
#endif

namespace nova_fmi {

// FMI 3.0 Standard Types
typedef void* fmi3Instance;
typedef unsigned int fmi3ValueReference;
typedef double fmi3Float64;
typedef int fmi3Int32;
typedef int fmi3Boolean;
typedef const char* fmi3String;

typedef fmi3Instance (NOVA_STDCALL *fmi3InstantiateCoSimulation_t)(fmi3String, fmi3String, fmi3String, fmi3Boolean, fmi3Boolean, fmi3Boolean, fmi3Boolean, const fmi3ValueReference[], size_t, const void*, void*, void*);
typedef void (NOVA_STDCALL *fmi3FreeInstance_t)(fmi3Instance);
typedef int (NOVA_STDCALL *fmi3EnterInitializationMode_t)(fmi3Instance, fmi3Boolean, fmi3Float64, fmi3Float64, fmi3Boolean, fmi3Float64);
typedef int (NOVA_STDCALL *fmi3ExitInitializationMode_t)(fmi3Instance);
typedef int (NOVA_STDCALL *fmi3DoStep_t)(fmi3Instance, fmi3Float64, fmi3Float64, fmi3Boolean, fmi3Boolean*, fmi3Boolean*, fmi3Boolean*, fmi3Float64*);
typedef int (NOVA_STDCALL *fmi3Terminate_t)(fmi3Instance);
typedef int (NOVA_STDCALL *fmi3GetFloat64_t)(fmi3Instance, const fmi3ValueReference[], size_t, fmi3Float64[], size_t);
typedef int (NOVA_STDCALL *fmi3SetFloat64_t)(fmi3Instance, const fmi3ValueReference[], size_t, const fmi3Float64[], size_t);

typedef int (NOVA_STDCALL *fmi3GetFMUState_t)(fmi3Instance, void**);
typedef int (NOVA_STDCALL *fmi3SetFMUState_t)(fmi3Instance, void*);
typedef int (NOVA_STDCALL *fmi3FreeFMUState_t)(fmi3Instance, void**);

fmi3_fmu::fmi3_fmu(std::shared_ptr<NovaFmiLibrary> lib, std::unique_ptr<nova_sim::temp_dir> temp, model_description md, bool fmiLogging)
    : lib_(std::move(lib)), temp_(std::move(temp)), md_(std::move(md)), fmiLogging_(fmiLogging) {}

const model_description& fmi3_fmu::get_model_description() const { return md_; }

std::unique_ptr<NovaSlave> fmi3_fmu::new_instance(const std::string& instanceName) {
    auto s = std::make_unique<NovaSlave>(instanceName, md_, lib_);
    
    auto lib = lib_;
    auto fmi3Instantiate = lib->getFunction<fmi3InstantiateCoSimulation_t>("fmi3InstantiateCoSimulation");
    auto fmi3FreeInstance = lib->getFunction<fmi3FreeInstance_t>("fmi3FreeInstance");
    
    if (!fmi3Instantiate || !fmi3FreeInstance) return nullptr;

    fmi3Instance c = fmi3Instantiate(instanceName.c_str(), md_.guid.c_str(), "", 0, 0, 0, 0, nullptr, 0, nullptr, nullptr, nullptr);
    if (!c) return nullptr;

    s->component_ = std::shared_ptr<void>(c, [fmi3FreeInstance, lib](void* ptr) {
        if (ptr && fmi3FreeInstance) fmi3FreeInstance(ptr);
    });

    auto fmi3EnterInit = lib->getFunction<fmi3EnterInitializationMode_t>("fmi3EnterInitializationMode");
    auto fmi3ExitInit = lib->getFunction<fmi3ExitInitializationMode_t>("fmi3ExitInitializationMode");
    auto fmi3DoStep = lib->getFunction<fmi3DoStep_t>("fmi3DoStep");
    auto fmi3Terminate = lib->getFunction<fmi3Terminate_t>("fmi3Terminate");
    auto fmi3GetFloat64 = lib->getFunction<fmi3GetFloat64_t>("fmi3GetFloat64");
    auto fmi3SetFloat64 = lib->getFunction<fmi3SetFloat64_t>("fmi3SetFloat64");
    auto fmi3GetFMUState = lib->getFunction<fmi3GetFMUState_t>("fmi3GetFMUState");
    auto fmi3SetFMUState = lib->getFunction<fmi3SetFMUState_t>("fmi3SetFMUState");
    auto fmi3FreeFMUState = lib->getFunction<fmi3FreeFMUState_t>("fmi3FreeFMUState");

    s->fmi.enter_init = [fmi3EnterInit](void* ch, double start, double stop, double tol) {
        return fmi3EnterInit ? fmi3EnterInit(ch, 0, 0.0, start, (stop > 0), stop) == 0 : false;
    };
    s->fmi.exit_init = [fmi3ExitInit](void* ch) { return fmi3ExitInit ? fmi3ExitInit(ch) == 0 : false; };
    s->fmi.step = [fmi3DoStep](void* ch, double t, double dt) { 
        fmi3Boolean event, term, sync; fmi3Float64 tout;
        return fmi3DoStep ? fmi3DoStep(ch, t, dt, 1, &event, &term, &sync, &tout) == 0 : false; 
    };
    s->fmi.terminate = [fmi3Terminate](void* ch) { return fmi3Terminate ? fmi3Terminate(ch) == 0 : false; };
    s->fmi.get_real = [fmi3GetFloat64](void* ch, const value_ref* vr, size_t n, double* v) { 
        return fmi3GetFloat64 ? fmi3GetFloat64(ch, vr, n, v, n) == 0 : false; 
    };
    s->fmi.set_real = [fmi3SetFloat64](void* ch, const value_ref* vr, size_t n, const double* v) { 
        return fmi3SetFloat64 ? fmi3SetFloat64(ch, vr, n, v, n) == 0 : false; 
    };
    
    s->fmi.get_state = [fmi3GetFMUState](void* ch) {
        void* state = nullptr;
        if (fmi3GetFMUState && fmi3GetFMUState(ch, &state) == 0) return state;
        return (void*)nullptr;
    };
    s->fmi.set_state = [fmi3SetFMUState](void* ch, void* state) {
        return fmi3SetFMUState ? fmi3SetFMUState(ch, state) == 0 : false;
    };
    s->fmi.free_state = [fmi3FreeFMUState](void* ch, void* state) {
        return fmi3FreeFMUState ? fmi3FreeFMUState(ch, &state) == 0 : false;
    };

    return s;
}

} // namespace nova_fmi
