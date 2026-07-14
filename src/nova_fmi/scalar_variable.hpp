#ifndef NOVA_FMI_SCALAR_VARIABLE_HPP
#define NOVA_FMI_SCALAR_VARIABLE_HPP

#include <string>
#include <variant>
#include <vector>
#include <optional>

namespace nova_fmi
{

/** @brief Real 类型的属性 */
struct real_attributes {
    std::optional<double> start;
};
/** @brief Integer 类型的属性 */
struct integer_attributes {
    std::optional<int> start;
};
/** @brief Boolean 类型的属性 */
struct boolean_attributes {
    std::optional<bool> start;
};
/** @brief String 类型的属性 */
struct string_attributes {
    std::optional<std::string> start;
};

// 增加 start 属性
/** @brief Vector 类型的属性（仅限支持向量特性的部分分支） */
struct vector_attributes {
    std::optional<unsigned int> length;
    std::optional<std::vector<double>> start;
};

/** @brief 变量的类型属性变体，可以是五种基础类型之一 */
using variable_attributes = std::variant<real_attributes, integer_attributes, boolean_attributes, string_attributes, vector_attributes>;

/**
 * @brief 标量变量 (ScalarVariable) 的完整描述
 * 包含了名字、值引用（ValueReference）以及类型等一切通过 XML 解析得到的信息
 */
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

/** @brief 用于存放多个标量变量的集合类型 */
using model_variables = std::vector<scalar_variable>;

} // namespace nova_fmi

#endif // NOVA_FMI_SCALAR_VARIABLE_HPP
