from pathlib import Path
import os
import sys

# Add project root to sys.path to import nova_sim_py
project_root = Path(__file__).parent.parent.parent
sys.path.append(str(project_root))

from nova_sim_py import *
from nova_sim_py.plotter import Plotter, TimeSeriesConfig

def kelvin_to_deg(value: float) -> float:
    return value - 273.15

def main():
    print(f"Ecoslib version: {EcosLib.version()}")

    EcosLib.set_log_level("debug")

    # 定位 FMU (严格对齐原版路径逻辑)
    fmu_path = str((Path(__file__).parent.parent.parent / 'data' / 'fmus' / '2.0' / '20sim' / 'ControlledTemperature.fmu').resolve())
    
    result_dir = project_root / 'results' / 'python'
    result_dir.mkdir(parents=True, exist_ok=True)
    result_file = str(result_dir / "nova_controlled_temperature.csv")

    with EcosSimulationStructure() as ss:
        ss.add_model("model", fmu_path)

        with (EcosSimulation(structure=ss, step_size=1/100)) as sim:

            sim.add_csv_writer(result_file)
            sim.init()
            sim.step_until(10)
            sim.terminate()

    # 严格对齐原版绘图配置结构
    config = TimeSeriesConfig(
        title="ControlledTemperature",
        y_label="Temperature[deg]",
        identifiers=["model::Temperature_Reference", "model::Temperature_Room"],
        modifiers={
            "model::Temperature_Reference" : kelvin_to_deg,
            "model::Temperature_Room" : kelvin_to_deg
        }
    )
    
    # plotter = Plotter(result_file, config)
    # plotter.show()
    
    print(f"Nova Python ControlledTemperature finished. Results: {result_file}")


if __name__ == "__main__":
    main()
