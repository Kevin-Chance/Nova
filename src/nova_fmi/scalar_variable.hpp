#ifndef NOVA_FMI_SCALAR_VARIABLE_HPP
#define NOVA_FMI_SCALAR_VARIABLE_HPP

#include <string>
#include <variant>
#include <vector>
#include <optional>

namespace nova_fmi
{

struct real_attributes {
    std::optional<double> start;
};
struct integer_attributes {
    std::optional<int> start;
};
struct boolean_attributes {
    std::optional<bool> start;
};
struct string_attributes {
    std::optional<std::string> start;
};

// 增加 start 以修复 fmi2_model_description.cpp 的编译
struct vector_attributes {
    std::optional<unsigned int> length;
    std::optional<std::vector<double>> start;
};

using variable_attributes = std::variant<real_attributes, integer_attributes, boolean_attributes, string_attributes, vector_attributes>;

struct scalar_variable
{
    std::string name;
    unsigned int vr;
    std::optional<std::string> description;
    std::optional<std::string> causality;
    std::optional<std::string> variability;
    variable_attributes typeAttributes;

    // FMI 2.0+ 属性
    std::optional<std::string> initial;
    std::optional<unsigned int> vectorLength;

    [[nodiscard]] bool is_real() const { return std::holds_alternative<real_attributes>(typeAttributes); }
    [[nodiscard]] bool is_integer() const { return std::holds_alternative<integer_attributes>(typeAttributes); }
    [[nodiscard]] bool is_boolean() const { return std::holds_alternative<boolean_attributes>(typeAttributes); }
    [[nodiscard]] bool is_string() const { return std::holds_alternative<string_attributes>(typeAttributes); }
};

using model_variables = std::vector<scalar_variable>;

} // namespace nova_fmi

#endif // NOVA_FMI_SCALAR_VARIABLE_HPP
