from novapy import *
from novapy.plotter import *

from pathlib import Path

def main():
    print(f"Novalib version: {NovaLib().version()}")

    NovaLib().set_log_level("debug")

    ssp_dir = (Path(__file__).parent.parent.parent / 'data' / 'ssp' / '1.0' / 'quarter_truck').resolve()
    ssp_file = f"{ssp_dir}/quarter-truck.ssp"
    result_file = "results/python/quarter_truck_ssp.csv"

    with(NovaSimulation(ssp_path=ssp_file, step_size=1.0 / 100)) as sim:

        sim.add_csv_writer(result_file, f"{ssp_dir}/CsvConfig.xml")

        sim.init(parameter_set="initialValues")
        sim.step_until(time_point=10)
        sim.terminate()

    config = TimeSeriesConfig(
        title="Quarter-truck",
        y_label="Height[m]",
        identifiers=["chassis::zChassis", "wheel::zWheel", "ground::zGround"])
    plotter = Plotter(result_file, config)
    plotter.show()


if __name__ == "__main__":
    main()
