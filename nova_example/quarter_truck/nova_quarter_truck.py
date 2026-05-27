
import os
import sys
from pathlib import Path

# Add project root to sys.path to import nova_sim_py
project_root = Path(__file__).parent.parent.parent
sys.path.append(str(project_root))

from nova_sim_py import NovaSimulation, NovaSimulationStructure, EcosLib

def main():
    print(f"Nova Ecoslib version: {EcosLib.version()}")

    # Note: set_log_level is currently a no-op in Nova's Python lib.py
    EcosLib.set_log_level("debug")

    ssp_dir = (project_root / 'data' / 'ssp' / '1.0' / 'quarter_truck').resolve()
    fmu_dir = ssp_dir / 'resources'
    
    # Use RESULT_FOLDER logic if available, or relative to project root
    # For Python scripts, we'll manually point to the project results folder.
    result_dir = project_root / 'results' / 'python'
    result_dir.mkdir(parents=True, exist_ok=True)
    result_file = str(result_dir / "nova_quarter_truck_py.csv")

    # Nova Python API currently requires manual structure building as ssp_path is not in C wrapper
    with NovaSimulationStructure() as ss:
        ss.add_model("chassis", str(fmu_dir / "chassis.fmu"))
        ss.add_model("ground", str(fmu_dir / "ground.fmu"))
        ss.add_model("wheel", str(fmu_dir / "wheel.fmu"))

        ss.make_connection("chassis", "p.e", "wheel", "p1.e", "real")
        ss.make_connection("wheel", "p1.f", "chassis", "p.f", "real")
        ss.make_connection("wheel", "p.e", "ground", "p.e", "real")
        ss.make_connection("ground", "p.f", "wheel", "p.f", "real")

        ss.add_parameter_set("initialValues", {"chassis::C.mChassis": 400.0})

        with NovaSimulation(structure=ss, step_size=1.0 / 100) as sim:
            sim.add_csv_writer(result_file, str(ssp_dir / "CsvConfig.xml"))
            
            sim.init(parameter_set="initialValues")
            
            sim.step_until(10)
            sim.terminate()

    print(f"Nova Python Quarter-Truck finished. Results: {result_file}")

    from nova_sim_py.plotter import Plotter, TimeSeriesConfig
    configs = [
        TimeSeriesConfig(
            title="Quarter-truck",
            y_label="Height[m]",
            identifiers=["chassis::zChassis"]),
        TimeSeriesConfig(
            title="Quarter-truck",
            y_label="Height[m]",
            identifiers=["wheel::zWheel"]),
        TimeSeriesConfig(
            title="Quarter-truck",
            y_label="Height[m]",
            identifiers=["ground::zGround"])
    ]
    plotter = Plotter(result_file, configs)
    plotter.show()

if __name__ == "__main__":
    main()
