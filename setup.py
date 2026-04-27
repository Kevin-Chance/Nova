import os
from setuptools import setup, find_packages

WINDOWS = (os.name == 'nt')


def version():
    with open("version.txt", "r") as f:
        return f.readline().strip()


def binary_suffix():
    return ".exe" if WINDOWS else ""


setup(
    name="nova_sim_py",
    version=version(),
    packages=find_packages(),  
    include_package_data=True,  
    data_files=[
        ("Scripts", [f"nova_sim_py/binaries/ecos{binary_suffix()}"]) 
    ],
)
