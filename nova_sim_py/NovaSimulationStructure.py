from .lib import dll
from ctypes import c_void_p, c_char_p, c_bool, c_double

class NovaSimulationStructure:
    def __init__(self, ssp_path=None):
        if ssp_path:
            _load = dll.nova_simulation_structure_load_ssp
            _load.restype = c_void_p
            _load.argtypes = [c_char_p]
            self._handle = _load(str(ssp_path).encode())
        else:
            _create = dll.nova_simulation_structure_create
            _create.restype = c_void_p
            self._handle = _create()

    def add_model(self, name, uri):
        _add = dll.nova_simulation_structure_add_model
        _add.argtypes = [c_void_p, c_char_p, c_char_p]
        _add.restype = c_bool
        return _add(self._handle, str(name).encode(), str(uri).encode())

    def make_connection(self, src_inst, src_var, dst_inst, dst_var, type="real"):
        _conn = dll.nova_simulation_structure_make_connection
        _conn.argtypes = [c_void_p, c_char_p, c_char_p, c_char_p, c_char_p, c_char_p]
        _conn(self._handle, src_inst.encode(), src_var.encode(), dst_inst.encode(), dst_var.encode(), type.encode())

    def add_parameter_set(self, name, parameters):
        _create_pps = dll.nova_parameter_set_create
        _create_pps.restype = c_void_p
        pps = _create_pps()
        
        _add_real = dll.nova_parameter_set_add_real
        _add_real.argtypes = [c_void_p, c_char_p, c_double]
        
        for k, v in parameters.items():
            _add_real(pps, k.encode(), float(v))
            
        _add_to_ss = dll.nova_simulation_structure_add_parameter_set
        _add_to_ss.argtypes = [c_void_p, c_char_p, c_void_p]
        _add_to_ss(self._handle, name.encode(), pps)
        
        _destroy_pps = dll.nova_parameter_set_destroy
        _destroy_pps.argtypes = [c_void_p]
        _destroy_pps(pps)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.__del__()

    def __del__(self):
        _destroy = dll.nova_simulation_structure_destroy
        _destroy.argtypes = [c_void_p]
        if hasattr(self, '_handle') and self._handle:
            _destroy(self._handle)
            self._handle = None
