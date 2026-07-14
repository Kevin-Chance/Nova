#ifndef NOVA_FMI_NOVA_FMU_HPP
#define NOVA_FMI_NOVA_FMU_HPP

#include "model_description.hpp"
#include "nova_slave.hpp"
#include <filesystem>
#include <memory>

namespace nova_fmi {

/**
 * @brief FMU (Functional Mock-up Unit) 的抽象基接口
 * 代表一个已加载解析完毕的 FMU 模块，能够作为工厂去实例化具体的 Slave 对象。
 */
class fmu {
public:
    /**
     * @brief 获取该 FMU 的静态模型描述
     * @return 返回 model_description 的常量引用
     */
    virtual const model_description& get_model_description() const = 0;
    
    /**
     * @brief 实例化一个新的 FMU 执行实例
     * @param instanceName 实例名称
     * @return 成功返回实例的智能指针，失败返回 nullptr
     */
    virtual std::unique_ptr<NovaSlave> new_instance(const std::string& instanceName) = 0;
    virtual ~fmu() = default;
};

/**
 * @brief 从指定的路径加载并解析 FMU 文件
 * @param fmuPath FMU 文件的路径
 * @param fmiLogging 是否开启 FMI 运行日志
 * @return 成功加载并解析后返回具体的 fmu 实例，否则返回 nullptr
 */
std::unique_ptr<fmu> loadFmu(const std::filesystem::path& fmuPath, bool fmiLogging = true);

} // namespace nova_fmi

#endif // NOVA_FMI_NOVA_FMU_HPP
