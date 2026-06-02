from ctypes import c_void_p, c_char_p, c_int, c_bool, c_double

from .lib import dll, NovaLib


class NovaParameterSet:

    def __init__(self):
        self._parameter_set_add_int = dll.nova_parameter_set_add_int
        self._parameter_set_add_int.argtypes = [c_void_p, c_char_p, c_int]

        self._parameter_set_add_real = dll.nova_parameter_set_add_real
        self._parameter_set_add_real.argtypes = [c_void_p, c_char_p, c_double]

        self._parameter_set_add_string = dll.nova_parameter_set_add_string
        self._parameter_set_add_string.argtypes = [c_void_p, c_char_p, c_char_p]

        self._parameter_set_add_bool = dll.nova_parameter_set_add_bool
        self._parameter_set_add_bool.argtypes = [c_void_p, c_char_p, c_bool]

        create_parameter_set = dll.nova_parameter_set_create
        create_parameter_set.restype = c_void_p
        self._handle = create_parameter_set()
        if self._handle is None:
            raise Exception("Failed to create NovaParameterSet")

    def add_int(self, name: str, value: int):
        self._parameter_set_add_int(self.handle, name.encode(), value)

    def add_real(self, name: str, value: float):
        self._parameter_set_add_real(self.handle, name.encode(), value)

    def add_bool(self, name: str, value: bool):
        self._parameter_set_add_bool(self.handle, name.encode(), value)

    def add_string(self, name: str, value: str):
        self._parameter_set_add_string(self.handle, name.encode(), value.encode())

    @property
    def handle(self):
        return self._handle

    def free(self):
        if not self._handle is None:
            destroy_parameter_set = dll.nova_parameter_set_destroy
            destroy_parameter_set.argtypes = [c_void_p]

            destroy_parameter_set(self._handle)
            self._handle = None

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.free()

    def __del__(self):
        self.free()
