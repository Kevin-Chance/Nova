from nova_sim_py import *
from nova_sim_py.plotter import *

from pathlib import Path

def kelvin_to_deg(value: float) -> float:
    return value - 273.15

def main():
    print(f"Novalib version: {NovaLib.version()}")

    NovaLib.set_log_level("debug")

    fmu_path = str((Path(__file__).parent.parent.parent / 'data' / 'fmus' / '2.0' / '20sim' / 'ControlledTemperature.fmu').resolve())
    result_file = "results/python/temperature.csv"

    with NovaSimulationStructure() as ss:
        ss.add_model("model", fmu_path)

        with (NovaSimulation(structure=ss, step_size=1/100)) as sim:

            sim.add_csv_writer(result_file)
            sim.init()
            sim.step_until(40)
            sim.terminate()

    config = TimeSeriesConfig(
        title="ControlledTemperature",
        y_label="Temperature[deg]",
        identifiers=["model::Temperature_Reference", "model::Temperature_Room"],
        modifiers={
            "model::Temperature_Reference" : kelvin_to_deg,
            "model::Temperature_Room" : kelvin_to_deg
        }
    )
    plotter = Plotter(result_file, config)
    plotter.show()


if __name__ == "__main__":
    main()