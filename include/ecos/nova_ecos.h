#ifndef NOVA_ECOS_H
#define NOVA_ECOS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifdef NOVA_EXPORT_DLL
#define NOVA_API __declspec(dllexport)
#else
#define NOVA_API __declspec(dllimport)
#endif
#else
#define NOVA_API
#endif

typedef struct nova_simulation_t nova_simulation_t;
typedef struct nova_simulation_structure_t nova_simulation_structure_t;

NOVA_API nova_simulation_structure_t* nova_simulation_structure_create();
NOVA_API void nova_simulation_structure_destroy(nova_simulation_structure_t* ss);
NOVA_API bool nova_simulation_structure_add_model(nova_simulation_structure_t* ss, const char* name, const char* uri);
NOVA_API void nova_simulation_structure_make_connection(nova_simulation_structure_t* ss, const char* src_inst, const char* src_var, const char* dst_inst, const char* dst_var, const char* type);

NOVA_API nova_simulation_t* nova_simulation_create(nova_simulation_structure_t* ss, double step_size);
NOVA_API void nova_simulation_destroy(nova_simulation_t* sim);
NOVA_API bool nova_simulation_init(nova_simulation_t* sim, double start_time);
NOVA_API double nova_simulation_step(nova_simulation_t* sim, unsigned int num_steps);
NOVA_API void nova_simulation_step_until(nova_simulation_t* sim, double time_point);
NOVA_API void nova_simulation_step_for(nova_simulation_t* sim, double duration);
NOVA_API void nova_simulation_terminate(nova_simulation_t* sim);
NOVA_API void nova_simulation_add_csv_writer(nova_simulation_t* sim, const char* filename);

NOVA_API bool nova_simulation_get_real(nova_simulation_t* sim, const char* instance, const char* variable, double* value);
NOVA_API bool nova_simulation_set_real(nova_simulation_t* sim, const char* instance, const char* variable, double value);

NOVA_API void nova_library_version(int* major, int* minor, int* patch);

#ifdef __cplusplus
}
#endif

#endif // NOVA_ECOS_H
