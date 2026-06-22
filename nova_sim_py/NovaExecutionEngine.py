from .lib import dll, NovaLib
from .NovaEngine import NovaEngine

from ctypes import c_void_p, c_double


class NovaExecutionEngine:

    def __init__(self, sim: NovaEngine):

        _simulation_runner_create = dll.nova_scheduler_create
        _simulation_runner_create.restype = c_void_p
        _simulation_runner_create.argtypes = [c_void_p]

        self._handle = _simulation_runner_create(sim.sim)
        if self._handle is None:
            raise Exception("Failed to create NovaExecutionEngine")

    def start(self):
        _simulation_runner_start = dll.nova_scheduler_start
        _simulation_runner_start.argtypes = [c_void_p]
        _simulation_runner_start(self._handle)

    def stop(self):
        _simulation_runner_stop = dll.nova_scheduler_stop
        _simulation_runner_stop.argtypes = [c_void_p]
        _simulation_runner_stop(self._handle)

    def set_real_time_factor(self, factor: float):
        _simulation_runner_set_real_time_factor = dll.nova_scheduler_set_real_time_factor
        _simulation_runner_set_real_time_factor.argtypes = [c_void_p, c_double]
        _simulation_runner_set_real_time_factor(self._handle, factor)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.free()

    def free(self):
        if not self._handle is None:
            destroy_simulation_runner = dll.nova_scheduler_destroy
            destroy_simulation_runner.argtypes = [c_void_p]
            destroy_simulation_runner(self._handle)
            self._handle = None

    def __del__(self):
        self.free()
