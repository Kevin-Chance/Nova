
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
    
    result_dir = project_root / 'results' / 'python'
    result_dir.mkdir(parents=True, exist_ok=True)
    result_file = str(result_dir / "nova_quarter_truck_ssp_py.csv")

    with NovaSimulation(ssp_path=str(ssp_dir), step_size=1.0 / 100) as sim:
        sim.add_csv_writer(result_file, str(ssp_dir / "CsvConfig.xml"))

        sim.init(parameter_set="initialValues")
        sim.step_until(10)
        sim.terminate()

    print(f"Nova Python SSP Quarter-Truck finished. Results: {result_file}")

if __name__ == "__main__":
    main()
