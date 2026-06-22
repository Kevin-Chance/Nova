
from pathlib import Path
import os
import sys

# Add project root to sys.path to import nova_sim_py
project_root = Path(__file__).parent.parent.parent
sys.path.append(str(project_root))

from nova_sim_py import *
from nova_sim_py.chart_plotter import Plotter, TimeSeriesConfig

def main():
    NovaLib.set_log_level("debug")

    fmu_folder = (project_root / 'data' / 'fmus' / '1.0' / 'mass_spring_damper').resolve()
    
    result_dir = project_root / 'results' / 'python'
    result_dir.mkdir(parents=True, exist_ok=True)
    result_file = str(result_dir / "nova_spring_mass_damper.csv")

    with NovaSimulationStructure() as ss:
        ss.add_model("damper", f"{fmu_folder}/Damper.fmu")
        ss.add_model("mass", f"{fmu_folder}/Mass.fmu")
        ss.add_model("spring", f"{fmu_folder}/Spring.fmu")

        # In Nova, make_connection is used instead of make_real_connection
        ss.make_connection("spring", "for_xx", "mass", "in_l_u")
        ss.make_connection("spring", "for_yx", "mass", "in_l_w")
        ss.make_connection("mass", "out_l_u", "spring", "dis_xx")
        ss.make_connection("mass", "out_l_w", "spring", "dis_yx")
        ss.make_connection("damper", "df_0", "mass", "in_f_u")
        ss.make_connection("damper", "df_1", "mass", "in_f_w")
        ss.make_connection("mass", "out_f_u", "damper", "lv_0")
        ss.make_connection("mass", "out_f_w", "damper", "lv_1")

        ss.add_parameter_set("initialValues", {
            "spring::springStiffness": 5.0,
            "spring::zeroForceLength": 5.0,
            "damper::dampingCoefficient": 2.0,
            "mass::initialPositionX": 6.0,
            "mass::mediumDensity": 1.0
        })

        with NovaEngine(structure=ss, step_size=1.0/100) as sim:

            sim.add_csv_writer(result_file)

            sim.init(parameter_set="initialValues")
            sim.step_until(80)
            sim.terminate()

    config = TimeSeriesConfig(
        title="Mass-spring-damper",
        y_label="Height[m]",
        identifiers=["mass::out_l_u"])
    chart_plotter = Plotter(result_file, config)
    chart_plotter.show()
    print(f"Nova Python Spring-Mass-Damper finished. Results: {result_file}")


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        import traceback
        traceback.print_exc()
        exit(1)
