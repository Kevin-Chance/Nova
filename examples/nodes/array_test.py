from ecospy import *
from pathlib import Path

def main():

    EcosLib.set_log_level("debug")

    fmu_folder = (Path(__file__).parent.parent.parent / 'data' / 'fmus' / 'test').resolve()
    result_file = "results/nodes.csv"

    with EcosSimulationStructure() as ss:

        ss.add_model("NodesInput", f"{fmu_folder}/nodes_input.fmu")
        ss.add_model("NodesOutput", f"{fmu_folder}/nodes_output.fmu")

        ss.make_vector_connection("NodesOutput::outputArray", "NodesInput::inputArray")
        ss.make_vector_connection("NodesInput::sumArray", "NodesOutput::inputArray")
        with EcosSimulation(structure=ss, step_size=1.0) as sim:

            sim.add_csv_writer(result_file)

            if not sim.init():
                print(EcosLib.get_last_error())

            sim.step_for(5)

            sim.terminate()


if __name__ == "__main__":
    main()

