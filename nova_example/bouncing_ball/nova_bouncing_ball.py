from nova_sim_py import *
from nova_sim_py.plotter import Plotter, TimeSeriesConfig
from pathlib import Path
import os

def main():
    print(f"Ecoslib version: {EcosLib.version()}")
    EcosLib.set_log_level("debug")

    # 定位 FMU (路径相对于本文件位置调整)
    fmu_path = str((Path(__file__).parent.parent.parent / 'data' / 'fmus' / '3.0' / 'ref' / 'BouncingBall.fmu').resolve())
    
    os.makedirs("results/python", exist_ok=True)
    result_file = "results/python/nova_bouncing_ball.csv"

    with EcosSimulationStructure() as ss:
        ss.add_model("ball", fmu_path)

        # 1/100s 步长
        with EcosSimulation(structure=ss, step_size=1/100) as sim:
            sim.add_csv_writer(result_file)
            sim.init()
            
            # 仿真 10 秒 (1000步)
            sim.step(1000)
            
            sim.terminate()

    # 模仿原版 Plotter 配置
    config = TimeSeriesConfig(
        title="Nova BouncingBall",
        y_label="Height[m]",
        identifiers=["ball::h"])
    # plotter = Plotter(result_file, config)
    # plotter.show() 

    print(f"Nova Python BouncingBall finished. Results: {result_file}")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        import traceback
        traceback.print_exc()
        exit(1)
