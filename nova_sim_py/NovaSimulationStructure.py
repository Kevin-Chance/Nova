from .lib import dll
from ctypes import c_void_p, c_char_p, c_bool

class NovaSimulationStructure:
    def __init__(self):
        _create = dll.nova_simulation_structure_create
        _create.restype = c_void_p
        self._handle = _create()

    def add_model(self, name, uri):
        _add = dll.nova_simulation_structure_add_model
        _add.argtypes = [c_void_p, c_char_p, c_char_p]
        _add.restype = c_bool
        return _add(self._handle, name.encode(), uri.encode())

    def make_connection(self, src_inst, src_var, dst_inst, dst_var, type="real"):
        _conn = dll.nova_simulation_structure_make_connection
        _conn.argtypes = [c_void_p, c_char_p, c_char_p, c_char_p, c_char_p, c_char_p]
        _conn(self._handle, src_inst.encode(), src_var.encode(), dst_inst.encode(), dst_var.encode(), type.encode())

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
