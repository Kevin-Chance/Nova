import os
from pathlib import Path
from ctypes import CDLL, Structure, c_char_p, c_int, c_void_p, c_double, c_bool, byref


def load_library():
    def suffix() -> str:
        return ".dll" if os.name == "nt" else ".so"

    bin_folder = str((Path(__file__).parent / 'binaries').resolve())
    lib_name = f"libnova_simc{suffix()}"
    
    current_path = os.environ.get("PATH", "")
    if bin_folder not in current_path:
        os.environ["PATH"] = bin_folder + os.pathsep + current_path

    if os.name == "nt":
        with os.add_dll_directory(bin_folder):
            return CDLL(lib_name)
    else:
        return CDLL(f"{bin_folder}/{lib_name}")


dll = load_library()


class Version(Structure):
    _fields_ = [("major", c_int), ("minor", c_int), ("patch", c_int)]

    def __repr__(self):
        return f"v{self.major}.{self.minor}.{self.patch}"


class NovaLib:

    @staticmethod
    def version():
        major = c_int()
        minor = c_int()
        patch = c_int()
        dll.nova_library_version(byref(major), byref(minor), byref(patch))
        v = Version()
        v.major = major.value
        v.minor = minor.value
        v.patch = patch.value
        return v

    @staticmethod
    def set_log_level(lvl: str):
        _set = dll.nova_set_log_level
        _set.argtypes = [c_char_p]
        _set(lvl.encode())
