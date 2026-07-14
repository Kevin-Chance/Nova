#ifndef NOVA_FMI_SLAVE_HPP
#define NOVA_FMI_SLAVE_HPP

#include "model_description.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace nova_fmi {

using value_ref = uint32_t;

/**
 * @brief FMU slave 实例的抽象基接口。
 * 
 * 定义了所有 FMU 实例（无论其遵循哪种 FMI 标准版本）都必须实现的
 * 通用生命周期和状态访问方法。
 */
class slave {
public:
    /** @brief 分配给该 slave 实例的唯一名称 */
    const std::string instanceName;

    explicit slave(std::string name) : instanceName(std::move(name)) {}
    virtual ~slave() = default;

    /**
     * @brief 获取该 FMU 解析后的模型描述
     * @return 返回 model_description 的常量引用
     */
    virtual const model_description& get_model_description() const = 0;

    /**
     * @brief 进入 FMU 的初始化模式
     * 必须在调用 exit_initialization_mode() 和执行任何仿真步进之前被调用。
     * @param start 仿真开始时间
     * @param stop 仿真结束时间
     * @param tol 容差值
     * @return 成功返回 true，失败返回 false
     */
    virtual bool enter_initialization_mode(double start = 0, double stop = 0, double tol = 0) = 0;
    
    /**
     * @brief 退出初始化模式
     * @return 成功返回 true，失败返回 false
     */
    virtual bool exit_initialization_mode() = 0;
    
    /**
     * @brief 执行仿真步进
     * @param t 当前时间
     * @param dt 步进大小
     * @return 成功返回 true，步进失败则返回 false
     */
    virtual bool step(double t, double dt) = 0;
    
    /**
     * @brief 在 FMU 内部优雅地终止仿真
     * @return 成功返回 true，失败返回 false
     */
    virtual bool terminate() = 0;
    
    /**
     * @brief 将 FMU 重置为其初始状态
     * @return 成功返回 true，失败返回 false
     */
    virtual bool reset() = 0;
    
    /**
     * @brief 释放底层的 FMU 组件并释放相关联的内存
     */
    virtual void freeInstance() = 0;

    /**
     * @brief 获取当前内部的 FMU 状态
     * @return 返回状态指针，当不再需要时必须通过 free_state() 释放
     */
    virtual void* get_state() { return nullptr; }
    
    /**
     * @brief 将内部 FMU 状态设置为给定的 state
     * @param state 要恢复的状态指针
     * @return 成功返回 true，失败返回 false
     */
    virtual bool set_state(void* state) { return false; }
    virtual bool free_state(void* state) { return false; }

    virtual bool get_real(const std::vector<value_ref>& vr, std::vector<double>& values) = 0;
    virtual bool set_real(const std::vector<value_ref>& vr, const std::vector<double>& values) = 0;
    virtual bool get_integer(const std::vector<value_ref>& vr, std::vector<int32_t>& values) = 0;
    virtual bool set_integer(const std::vector<value_ref>& vr, const std::vector<int32_t>& values) = 0;
    virtual bool get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values) = 0;
    virtual bool set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values) = 0;
    virtual bool get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values) = 0;
    virtual bool set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values) = 0;
};

} // namespace nova_fmi

#endif // NOVA_FMI_SLAVE_HPP
