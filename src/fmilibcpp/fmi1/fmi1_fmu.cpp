#include "fmi1_fmu.hpp"
#include <iostream>
#include <utility>
#include <cstdio>
#include <algorithm>

#ifdef _WIN32
#define NOVA_STDCALL __stdcall
#else
#define NOVA_STDCALL
#endif

namespace nova_fmi {

typedef void* fmi1Component;
typedef unsigned int fmi1ValueReference;
typedef double fmi1Real;
typedef int fmi1Integer;
typedef int fmi1Boolean;
typedef const char* fmi1String;

// FIX WEEK 1: FMI 1.0 passes fmiCallbackFunctions by VALUE. Must be a struct of 4 pointers!
struct fmi1CallbackFunctions {
    void* logger;
    void* allocateMemory;
    void* freeMemory;
    void* stepFinished;
};

typedef fmi1Component (NOVA_STDCALL *fmi1InstantiateSlave_t)(fmi1String, fmi1String, fmi1String, fmi1String, fmi1Real, fmi1Boolean, fmi1Boolean, fmi1CallbackFunctions, fmi1Boolean);
typedef void (NOVA_STDCALL *fmi1FreeSlaveInstance_t)(fmi1Component);
typedef int (NOVA_STDCALL *fmi1InitializeSlave_t)(fmi1Component, fmi1Real, fmi1Boolean, fmi1Real);
typedef int (NOVA_STDCALL *fmi1DoStep_t)(fmi1Component, fmi1Real, fmi1Real, fmi1Boolean);
typedef int (NOVA_STDCALL *fmi1TerminateSlave_t)(fmi1Component);
typedef int (NOVA_STDCALL *fmi1GetReal_t)(fmi1Component, const fmi1ValueReference[], size_t, fmi1Real[]);
typedef int (NOVA_STDCALL *fmi1SetReal_t)(fmi1Component, const fmi1ValueReference[], size_t, const fmi1Real[]);
typedef int (NOVA_STDCALL *fmi1GetInteger_t)(fmi1Component, const fmi1ValueReference[], size_t, fmi1Integer[]);
typedef int (NOVA_STDCALL *fmi1SetInteger_t)(fmi1Component, const fmi1ValueReference[], size_t, const fmi1Integer[]);
typedef int (NOVA_STDCALL *fmi1GetBoolean_t)(fmi1Component, const fmi1ValueReference[], size_t, fmi1Boolean[]);
typedef int (NOVA_STDCALL *fmi1SetBoolean_t)(fmi1Component, const fmi1ValueReference[], size_t, const fmi1Boolean[]);
typedef int (NOVA_STDCALL *fmi1GetString_t)(fmi1Component, const fmi1ValueReference[], size_t, fmi1String[]);
typedef int (NOVA_STDCALL *fmi1SetString_t)(fmi1Component, const fmi1ValueReference[], size_t, const fmi1String[]);

template<typename T>
T getFmi1Function(const std::shared_ptr<NovaFmiLibrary>& lib, const std::string& modelId, const std::string& name) {
    auto f = lib->getFunction<T>(name);
    if (!f) f = lib->getFunction<T>(modelId + "_" + name);
    if (!f) f = lib->getFunction<T>("_" + name + "@36"); // Win32 stdcall fallback heuristic
    return f;
}

fmi1_fmu::fmi1_fmu(std::shared_ptr<NovaFmiLibrary> lib, std::unique_ptr<nova_sim::temp_dir> temp, model_description md, bool fmiLogging)
    : lib_(std::move(lib)), temp_(std::move(temp)), md_(std::move(md)) {}

const model_description& fmi1_fmu::get_model_description() const { return md_; }

// FMI 1.0 initialization state to delay fmiInitializeSlave
struct Fmi1InitState {
    double start;
    double stop;
};

std::unique_ptr<NovaSlave> fmi1_fmu::new_instance(const std::string& instanceName) {
    auto s = std::make_unique<NovaSlave>(instanceName, md_, lib_);
    auto lib = lib_;
    std::string res = temp_->path().string();
    std::replace(res.begin(), res.end(), '\\', '/');
    auto resource_uri = "file:///" + res;

    auto fmi1InstantiateSlave = getFmi1Function<fmi1InstantiateSlave_t>(lib, md_.modelIdentifier, "fmiInstantiateSlave");
    auto fmi1FreeSlaveInstance = getFmi1Function<fmi1FreeSlaveInstance_t>(lib, md_.modelIdentifier, "fmiFreeSlaveInstance");
    
    if (!fmi1InstantiateSlave || !fmi1FreeSlaveInstance) return nullptr;

    fmi1CallbackFunctions funcs = {nullptr, nullptr, nullptr, nullptr};
    // Need a dummy allocator
    funcs.allocateMemory = (void*)calloc;
    funcs.freeMemory = (void*)free;

    // FMI 1.0 often prefers plain filesystem path over URI
    fmi1Component c = fmi1InstantiateSlave(instanceName.c_str(), md_.guid.c_str(), res.c_str(), "application/x-fmu-sharedlibrary", 0.0, 0, 0, funcs, 0);
    if (!c) c = fmi1InstantiateSlave(instanceName.c_str(), md_.guid.c_str(), resource_uri.c_str(), "application/x-fmu-sharedlibrary", 0.0, 0, 0, funcs, 0);
    if (!c) return nullptr;

    s->component_ = std::shared_ptr<void>(c, [fmi1FreeSlaveInstance, lib](void* ptr) {
        if (ptr && fmi1FreeSlaveInstance) fmi1FreeSlaveInstance(ptr);
    });

    auto fmi1InitializeSlave = getFmi1Function<fmi1InitializeSlave_t>(lib, md_.modelIdentifier, "fmiInitializeSlave");
    auto fmi1DoStep = getFmi1Function<fmi1DoStep_t>(lib, md_.modelIdentifier, "fmiDoStep");
    auto fmi1TerminateSlave = getFmi1Function<fmi1TerminateSlave_t>(lib, md_.modelIdentifier, "fmiTerminateSlave");
    auto fmi1GetReal = getFmi1Function<fmi1GetReal_t>(lib, md_.modelIdentifier, "fmiGetReal");
    auto fmi1SetReal = getFmi1Function<fmi1SetReal_t>(lib, md_.modelIdentifier, "fmiSetReal");
    auto fmi1GetInteger = getFmi1Function<fmi1GetInteger_t>(lib, md_.modelIdentifier, "fmiGetInteger");
    auto fmi1SetInteger = getFmi1Function<fmi1SetInteger_t>(lib, md_.modelIdentifier, "fmiSetInteger");
    auto fmi1GetBoolean = getFmi1Function<fmi1GetBoolean_t>(lib, md_.modelIdentifier, "fmiGetBoolean");
    auto fmi1SetBoolean = getFmi1Function<fmi1SetBoolean_t>(lib, md_.modelIdentifier, "fmiSetBoolean");
    auto fmi1GetString = getFmi1Function<fmi1GetString_t>(lib, md_.modelIdentifier, "fmiGetString");
    auto fmi1SetString = getFmi1Function<fmi1SetString_t>(lib, md_.modelIdentifier, "fmiSetString");

    auto initState = std::make_shared<Fmi1InitState>();
    initState->start = 0.0;
    initState->stop = 0.0;

    s->fmi.enter_init = [initState](void* ch, double start, double stop, double tol) {
        initState->start = start;
        initState->stop = stop;
        return true;
    };
    s->fmi.exit_init = [fmi1InitializeSlave, initState](void* ch) {
        if (!fmi1InitializeSlave) return false;
        int status = fmi1InitializeSlave(ch, (fmi1Real)initState->start, (fmi1Boolean)(initState->stop > 0), (fmi1Real)initState->stop);
        if (status != 0) std::cerr << "[FMI1] Initialize failed with status: " << status << std::endl;
        return status == 0;
    };
    s->fmi.step = [fmi1DoStep](void* ch, double t, double dt) { return fmi1DoStep ? fmi1DoStep(ch, t, dt, 1) == 0 : false; };
    s->fmi.terminate = [fmi1TerminateSlave](void* ch) { return fmi1TerminateSlave ? fmi1TerminateSlave(ch) == 0 : false; };
    s->fmi.get_real = [fmi1GetReal](void* ch, const value_ref* vr, size_t n, double* v) { return fmi1GetReal ? fmi1GetReal(ch, vr, n, v) == 0 : false; };
    s->fmi.set_real = [fmi1SetReal](void* ch, const value_ref* vr, size_t n, const double* v) { 
        int status = fmi1SetReal ? fmi1SetReal(ch, vr, n, const_cast<double*>(v)) : -1;
        if (status != 0) std::cerr << "[FMI1] SetReal failed with status: " << status << std::endl;
        return status == 0; 
    };
    s->fmi.get_int = [fmi1GetInteger](void* ch, const value_ref* vr, size_t n, int32_t* v) { return fmi1GetInteger ? fmi1GetInteger(ch, vr, n, v) == 0 : false; };
    s->fmi.set_int = [fmi1SetInteger](void* ch, const value_ref* vr, size_t n, const int32_t* v) { return fmi1SetInteger ? fmi1SetInteger(ch, vr, n, const_cast<int32_t*>(v)) == 0 : false; };
    s->fmi.get_bool = [fmi1GetBoolean](void* ch, const value_ref* vr, size_t n, int* v) { return fmi1GetBoolean ? fmi1GetBoolean(ch, vr, n, v) == 0 : false; };
    s->fmi.set_bool = [fmi1SetBoolean](void* ch, const value_ref* vr, size_t n, const int* v) { return fmi1SetBoolean ? fmi1SetBoolean(ch, vr, n, v) == 0 : false; };
    s->fmi.get_str = [fmi1GetString](void* ch, const value_ref* vr, size_t n, char** v) { return fmi1GetString ? fmi1GetString(ch, vr, n, const_cast<const char**>(v)) == 0 : false; };
    s->fmi.set_str = [fmi1SetString](void* ch, const value_ref* vr, size_t n, const char** v) { return fmi1SetString ? fmi1SetString(ch, vr, n, v) == 0 : false; };

    return s;
}

} // namespace nova_fmi
