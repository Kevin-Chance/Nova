
from nova_sim_py import *
from nova_sim_py.plotter import Plotter, TimeSeriesConfig
from pathlib import Path
import os

def main():
    EcosLib.set_log_level("debug")

    fmu_folder = (Path(__file__).parent.parent.parent / 'data' / 'fmus' / '1.0' / 'mass_spring_damper').resolve()
    
    os.makedirs("results/python", exist_ok=True)
    result_file = "results/python/nova_spring_mass_damper.csv"

    with EcosSimulationStructure() as ss:
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

        with EcosSimulation(structure=ss, step_size=1.0/100) as sim:

            sim.add_csv_writer(result_file)

            # Manually setting parameters as add_parameter_set is not yet implemented in Nova's Python wrapper
            sim.set_real("spring", "springStiffness", 5.0)
            sim.set_real("spring", "zeroForceLength", 5.0)
            sim.set_real("damper", "dampingCoefficient", 6.0)
            sim.set_real("mass", "initialPositionX", 6.0)
            sim.set_real("mass", "mediumDensity", 1.0)

            sim.init()
            sim.step_until(80)
            sim.terminate()

    config = TimeSeriesConfig(
        title="Nova Mass-spring-damper",
        y_label="Height[m]",
        identifiers=["mass::out_l_u"])
    # plotter = Plotter(result_file, config)
    # plotter.show()
    print(f"Nova Python Spring-Mass-Damper finished. Results: {result_file}")


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        import traceback
        traceback.print_exc()
        exit(1)
