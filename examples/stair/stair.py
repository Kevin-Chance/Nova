from novapy import *
from novapy.plotter import *

from pathlib import Path

def main():
    print(f"Novalib version: {NovaLib.version()}")

    NovaLib.set_log_level("debug")

    fmu_path = str((Path(__file__).parent.parent.parent / 'data' / 'fmus' / '3.0' / 'ref' / 'Stair.fmu').resolve())
    result_file = "results/python/stair.csv"

    with NovaSimulationStructure() as ss:
        ss.add_model("model", fmu_path)

        step = 0.2
        with (NovaSimulation(structure=ss, step_size=step)) as sim:

            sim.add_csv_writer(result_file)
            sim.init()
            sim.step_until(10-step)
            sim.terminate()

    config = TimeSeriesConfig(
        title="ControlledTemperature",
        y_label="Steps",
        identifiers=["model::counter"]
    )
    plotter = Plotter(result_file, config)
    plotter.show()


if __name__ == "__main__":
    main()