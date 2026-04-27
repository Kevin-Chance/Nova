
#include "fmi2_model_description.hpp"

namespace
{

std::optional<std::string> fmi2CausalityToString(fmi2Causality causality)
{
    switch (causality) {
        case fmi2CausalityInput: return "Input";
        case fmi2CausalityOutput: return "Output";
        case fmi2CausalityParameter: return "Parameter";
        case fmi2CausalityCalculatedParameter: return "CalculatedParameter";
        case fmi2CausalityLocal: return "Local";
        case fmi2CausalityIndependent: return "Independent";
        default: return std::nullopt;
    }
}

std::optional<std::string> fmi2VariabilityToString(fmi2Variability variability)
{
    switch (variability) {
        case fmi2VariabilityFixed: return "Fixed";
        case fmi2VariabilityTunable: return "Tunable";
        case fmi2VariabilityConstant: return "Constant";
        case fmi2VariabilityDiscrete: return "Discrete";
        case fmi2VariabilityContinuous: return "Continuous";
        default: return std::nullopt;
    }
}

std::optional<nova_fmi::scalar_variable> to_scalar_variable(fmi2VariableHandle* v)
{
    const auto type = fmi2_getVariableDataType(v);
    if (type == fmi2DataTypeEnumeration) {
        return std::nullopt;
    }

    nova_fmi::scalar_variable var;
    var.vr = fmi2_getVariableValueReference(v);
    var.name = fmi2_getVariableName(v);
    var.description = fmi2_getVariableDescription(v) ? fmi2_getVariableDescription(v) : "";
    var.causality = fmi2CausalityToString(fmi2_getVariableCausality(v));
    var.variability = fmi2VariabilityToString(fmi2_getVariableVariability(v));

    switch (type) {
        case fmi2DataTypeReal: {
            nova_fmi::real_attributes r{};
            if (fmi2_getVariableHasStartValue(v)) {
                r.start = fmi2_getVariableStartReal(v);
            }
            var.typeAttributes = r;
        } break;
        case fmi2DataTypeInteger: {
            nova_fmi::integer_attributes i{};
            if (fmi2_getVariableHasStartValue(v)) {
                i.start = fmi2_getVariableStartInteger(v);
            }
            var.typeAttributes = i;
        } break;
        case fmi2DataTypeBoolean: {
            nova_fmi::boolean_attributes b{};
            if (fmi2_getVariableHasStartValue(v)) {
                b.start = fmi2_getVariableStartBoolean(v);
            }
            var.typeAttributes = b;
        } break;
        case fmi2DataTypeString: {
            nova_fmi::string_attributes s{};
            if (fmi2_getVariableHasStartValue(v)) {
                s.start = fmi2_getVariableStartString(v);
            }
            var.typeAttributes = s;
        } break;
        case fmi2DataTypeVector: {
            nova_fmi::vector_attributes ve{};
            if (fmi2_getVariableHasStartValue(v)) {
                int len = 0;
                const double* rawVec = fmi2_getVariableStartVector(v, &len);
                if (rawVec && len > 0) {
                    ve.start = std::vector<double>(rawVec, rawVec + len);
                }
                var.vectorLength = len;
            }
            var.typeAttributes = ve;
            
        } break;
        default: break;
    }
    return var;
}

} // namespace

namespace nova_fmi
{

model_description create_fmi2_model_description(fmuHandle* handle)
{
    model_description md;
    md.fmiVersion = fmi2_getVersion(handle) ? fmi2_getVersion(handle) : "2.0";
    md.guid = fmi2_getGuid(handle);
    md.author = fmi2_getAuthor(handle) ? fmi2_getAuthor(handle) : "";
    md.modelName = fmi2_getModelName(handle);
    md.modelIdentifier = fmi2cs_getModelIdentifier(handle);
    md.description = fmi2_getModelDescription(handle) ? fmi2_getModelDescription(handle) : "";
    md.generationTool = fmi2_getGenerationTool(handle) ? fmi2_getGenerationTool(handle) : "";
    md.generationDateAndTime = fmi2_getGenerationDateAndTime(handle) ? fmi2_getGenerationDateAndTime(handle) : "";

    md.canGetAndSetState = fmi2cs_getCanGetAndSetFMUState(handle);

    md.defaultExperiment.startTime = fmi2_getDefaultStartTime(handle);
    md.defaultExperiment.stopTime = fmi2_getDefaultStopTime(handle);
    md.defaultExperiment.stepSize = fmi2_getDefaultStepSize(handle);
    md.defaultExperiment.tolerance = fmi2_getDefaultTolerance(handle);

    const auto varCount = fmi2_getNumberOfVariables(handle);
    for (auto i = 0; i < varCount; i++) {
        const auto var = fmi2_getVariableByIndex(handle, i+1);
        if (const auto scalar = to_scalar_variable(var)) {
            md.modelVariables.push_back(scalar.value());
        }
    }

    return md;
}

} // namespace nova_fmi
