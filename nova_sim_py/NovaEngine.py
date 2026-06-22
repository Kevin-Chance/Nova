from .lib import dll
from ctypes import c_void_p, c_double, c_char_p, c_bool, byref, c_int

from .NovaSimulationStructure import NovaSimulationStructure

class NovaEngine:
    def __init__(self, structure=None, step_size=0.01, ss=None, ssp_path=None):
        _create = dll.nova_engine_create
        _create.restype = c_void_p
        _create.argtypes = [c_void_p, c_double]
        
        actual_ss = structure if structure else ss
        if not actual_ss and ssp_path:
            self._ss_internal = NovaSimulationStructure(ssp_path=ssp_path)
            actual_ss = self._ss_internal
            
        if not actual_ss:
            raise Exception("No nova_engine structure or SSP path provided")
            
        if not actual_ss._handle:
            raise Exception("Simulation structure handle is invalid (perhaps SSP loading failed)")
            
        self.sim = _create(actual_ss._handle, step_size)
        if not self.sim:
            raise Exception("Failed to create Nova nova_engine")

    def init(self, start_time=0.0, parameter_set=None):
        _init = dll.nova_engine_init
        _init.argtypes = [c_void_p, c_double, c_char_p]
        _init.restype = c_bool
        param_set_str = parameter_set.encode() if parameter_set else None
        return _init(self.sim, start_time, param_set_str)

    def step(self, num_steps=1):
        _step = dll.nova_engine_step
        _step.restype = c_double
        _step.argtypes = [c_void_p, c_int]
        return _step(self.sim, num_steps)

    def step_until(self, time_point):
        _step = dll.nova_engine_step_until
        _step.argtypes = [c_void_p, c_double]
        _step(self.sim, time_point)

    def step_for(self, duration):
        _step = dll.nova_engine_step_for
        _step.argtypes = [c_void_p, c_double]
        _step(self.sim, duration)

    def terminate(self):
        _term = dll.nova_engine_terminate
        _term.argtypes = [c_void_p]
        _term(self.sim)

    def load_scenario(self, scenario_file):
        _load = dll.nova_engine_load_scenario
        _load.argtypes = [c_void_p, c_char_p]
        _load.restype = c_bool
        return _load(self.sim, scenario_file.encode())

    def add_csv_writer(self, result_file, config_path=None):
        _create_writer = dll.nova_csv_recorder_create
        _create_writer.argtypes = [c_char_p, c_char_p]
        _create_writer.restype = c_void_p
        
        cfg_path_str = config_path.encode() if config_path else None
        writer_ptr = _create_writer(result_file.encode(), cfg_path_str)
        
        _add_listener = dll.nova_engine_add_listener
        _add_listener.argtypes = [c_void_p, c_char_p, c_void_p]
        _add_listener(self.sim, b"csv_recorder", writer_ptr)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.terminate()
        self.destroy()

    def destroy(self):
        _destroy = dll.nova_engine_destroy
        _destroy.argtypes = [c_void_p]
        if hasattr(self, 'sim') and self.sim:
            _destroy(self.sim)
            self.sim = None

    def __del__(self):
        self.destroy()

    def get_real(self, instance, variable):
        _get = dll.nova_engine_get_real
        val = c_double()
        _get.argtypes = [c_void_p, c_char_p, c_char_p, c_void_p]
        if _get(self.sim, instance.encode(), variable.encode(), byref(val)):
            return val.value
        return None

    def set_real(self, instance, variable, value):
        _set = dll.nova_engine_set_real
        _set.argtypes = [c_void_p, c_char_p, c_char_p, c_double]
        _set.restype = c_bool
        return _set(self.sim, instance.encode(), variable.encode(), value)
