
import os
import sys
from pathlib import Path

# Add project root to sys.path to import nova_sim_py
project_root = Path(__file__).parent.parent.parent
sys.path.append(str(project_root))

from nova_sim_py import NovaSimulation, NovaSimulationStructure, EcosLib

def main():
    print(f"Nova Ecoslib version: {EcosLib.version()}")

    EcosLib.set_log_level("debug")

    ssp_dir = (project_root / 'data' / 'ssp' / '1.0' / 'quarter_truck').resolve()
    # Note: Nova's Python API doesn't support direct SSP loading yet.
    # We will simulate the "logic" of loading an SSP by manually defining the structure 
    # to maintain functional consistency with the original quarter_truck_ssp.py.
    
    result_dir = project_root / 'results'
    result_dir.mkdir(exist_ok=True)
    result_file = str(result_dir / "nova_quarter_truck_ssp_py.csv")

    fmu_dir = ssp_dir / 'resources'

    with NovaSimulationStructure() as ss:
        # Replicating SSP components
        ss.add_model("chassis", str(fmu_dir / "chassis.fmu"))
        ss.add_model("ground", str(fmu_dir / "ground.fmu"))
        ss.add_model("wheel", str(fmu_dir / "wheel.fmu"))

        # Replicating SSP connections
        ss.make_connection("chassis", "p.e", "wheel", "p1.e", "real")
        ss.make_connection("wheel", "p1.f", "chassis", "p.f", "real")
        ss.make_connection("wheel", "p.e", "ground", "p.e", "real")
        ss.make_connection("ground", "p.f", "wheel", "p.f", "real")

        with NovaSimulation(structure=ss, step_size=1.0 / 100) as sim:
            sim.add_csv_writer(result_file)

            sim.init()
            # Replicating SSP initial values
            sim.set_real("chassis", "C.mChassis", 400.0)
            
            sim.step_until(10)
            sim.terminate()

    print(f"Nova Python SSP Quarter-Truck finished. Results: {result_file}")

if __name__ == "__main__":
    main()
