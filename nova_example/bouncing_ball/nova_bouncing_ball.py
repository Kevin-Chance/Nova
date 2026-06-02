from pathlib import Path
import os
import sys

# Add project root to sys.path to import nova_sim_py
project_root = Path(__file__).parent.parent.parent
sys.path.append(str(project_root))

from nova_sim_py import *
from nova_sim_py.plotter import Plotter, TimeSeriesConfig

def main():
    print(f"NovaLib version: {NovaLib.version()}")
    NovaLib.set_log_level("debug")

    # 定位 FMU (路径相对于本文件位置调整)
    fmu_path = str((Path(__file__).parent.parent.parent / 'data' / 'fmus' / '3.0' / 'ref' / 'BouncingBall.fmu').resolve())
    
    result_dir = project_root / 'results' / 'python'
    result_dir.mkdir(parents=True, exist_ok=True)
    result_file = str(result_dir / "nova_bouncing_ball.csv")

    with NovaSimulationStructure() as ss:
        ss.add_model("ball", fmu_path)

        # 1/100s 步长
        with NovaSimulation(structure=ss, step_size=1/100) as sim:
            sim.add_csv_writer(result_file)
            sim.init()
            
            # 仿真 10 秒 (1000步)
            sim.step(1000)
            
            sim.terminate()

    config = TimeSeriesConfig(
        title="BouncingBall",
        y_label="Height[m]",
        identifiers=["ball::h"])
    plotter = Plotter(result_file, config)
    plotter.show() 

    print(f"Nova Python BouncingBall finished. Results: {result_file}")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        import traceback
        traceback.print_exc()
        exit(1)
