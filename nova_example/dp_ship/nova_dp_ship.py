import os
import sys
import signal
from pathlib import Path

# Add project root to sys.path to import nova_sim_py
project_root = Path(__file__).parent.parent.parent
sys.path.append(str(project_root))

from nova_sim_py import NovaSimulation, NovaSimulationStructure, EcosLib

def signal_handler(sig, frame):
    print("\nSimulation interrupted by user (CTRL+C).")
    sys.exit(0)

def main():
    signal.signal(signal.SIGINT, signal_handler)
    print(f"Nova Ecoslib version: {EcosLib.version()}")
    EcosLib.set_log_level("debug")

    ssp_dir = (project_root / 'data' / 'ssp' / '1.0' / 'dp_ship').resolve()
    log_config = str(ssp_dir / "CsvConfig.xml")
    scenario = str(ssp_dir / "waypoints_scenario.xml")
    
    result_dir = project_root / 'results' / 'python'
    result_dir.mkdir(parents=True, exist_ok=True)
    result_file = str(result_dir / "nova_dp_ship_py.csv")

    fmu_dir = ssp_dir / 'resources'

    with NovaSimulation(ssp_path=str(ssp_dir), step_size=0.04) as sim:
        sim.add_csv_writer(result_file, log_config)
        
        if not sim.load_scenario(scenario):
            print("Failed to load scenario.")

        sim.init()
        try:
            print("Press CTRL+C to terminate the simulation.")
            t = 0
            while t < 1500:
                t = sim.step(1)
        except SystemExit:
            print(f"Simulation requested to stop at t={t}")

        sim.terminate()

    print(f"Nova Python dp_ship finished. Results: {result_file}")

if __name__ == "__main__":
    main()
