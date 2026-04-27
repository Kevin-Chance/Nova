#ifndef NOVA_FMI_STANDARD_H
#define NOVA_FMI_STANDARD_H

#ifdef _WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

// FMI 2.0 Standard Types
typedef void*           fmi2Component;
typedef void*           fmi2ComponentEnvironment;
typedef void*           fmi2FMUstate;
typedef unsigned int    fmi2ValueReference;
typedef double          fmi2Real;
typedef int             fmi2Integer;
typedef int             fmi2Boolean;
typedef char            fmi2Char;
typedef const fmi2Char* fmi2String;
typedef char            fmi2Byte;

#define fmi2True  1
#define fmi2False 0

typedef enum {
    fmi2OK, fmi2Warning, fmi2Discard, fmi2Error, fmi2Fatal, fmi2Pending
} fmi2Status;

typedef enum {
    fmi2ModelExchange, fmi2CoSimulation
} fmi2Type;

// FMI 2.0 Function Signatures
typedef const char* (STDCALL *fmi2GetVersion_t)(void);
typedef fmi2Component (STDCALL *fmi2Instantiate_t)(fmi2String, fmi2Type, fmi2String, fmi2String, const void*, fmi2Boolean, fmi2Boolean);
typedef void (STDCALL *fmi2FreeInstance_t)(fmi2Component);
typedef fmi2Status (STDCALL *fmi2SetupExperiment_t)(fmi2Component, fmi2Boolean, fmi2Real, fmi2Real, fmi2Boolean, fmi2Real);
typedef fmi2Status (STDCALL *fmi2EnterInitializationMode_t)(fmi2Component);
typedef fmi2Status (STDCALL *fmi2ExitInitializationMode_t)(fmi2Component);
typedef fmi2Status (STDCALL *fmi2Terminate_t)(fmi2Component);
typedef fmi2Status (STDCALL *fmi2Reset_t)(fmi2Component);
typedef fmi2Status (STDCALL *fmi2GetReal_t)(fmi2Component, const fmi2ValueReference[], size_t, fmi2Real[]);
typedef fmi2Status (STDCALL *fmi2SetReal_t)(fmi2Component, const fmi2ValueReference[], size_t, const fmi2Real[]);
typedef fmi2Status (STDCALL *fmi2GetInteger_t)(fmi2Component, const fmi2ValueReference[], size_t, fmi2Integer[]);
typedef fmi2Status (STDCALL *fmi2SetInteger_t)(fmi2Component, const fmi2ValueReference[], size_t, const fmi2Integer[]);
typedef fmi2Status (STDCALL *fmi2DoStep_t)(fmi2Component, fmi2Real, fmi2Real, fmi2Boolean);

#endif // NOVA_FMI_STANDARD_H
