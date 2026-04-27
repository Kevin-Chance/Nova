from .NovaSimulation import NovaSimulation as EcosSimulation
from .NovaSimulationStructure import NovaSimulationStructure as EcosSimulationStructure
from .lib import EcosLib

# 为了完全匹配原有代码风格，保留 EcosSimulation 和 EcosSimulationStructure 的别名
NovaSimulation = EcosSimulation
NovaSimulationStructure = EcosSimulationStructure
