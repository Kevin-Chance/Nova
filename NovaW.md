## 开发计划

### 阶段一：FMI 底层接口与生命周期重写 (4月6日 - 4月26日)

针对原 `src/fmilibcpp/` 目录进行重构，对应 nova 统一资源管理模块与部分扁平数据管道。

*   **Week 1 (04.06 - 04.12)：动态库加载与 RAII 容器封装**
    *   废弃原有的原始指针管理。基于 `LoadLibrary` (Windows) 和 `dlopen` (Linux) 编写全新的跨平台 FMI 动态库加载器。
    *   实现 `std::shared_ptr` 的 `Custom Deleter`，将 `fmi1FreeInstance`, `fmi2FreeInstance`, `fmi3FreeInstance` 绑定到智能指针析构周期中。
*   **Week 2 (04.13 - 04.19)：FMI C-API 的扁平化封装**
    *   清理原有的 `fmu` 和 `slave` 虚基类继承树。
    *   利用新的 RAII 句柄，编写基础的数值读写接口（`get_real`, `set_real` 等），确保新的内存管理模型能够正确进行数值穿透。
*   **Week 3 (04.20 - 04.26)：定位器降维与解耦**
    *   针对 `src/nova/`，物理删除 `model_resolver.hpp/cpp` 及其所有子类（`file_`、`url_` 等）。
    *   编写独立的 `NovaFmuLocator` 类，使用 `starts_with` 等字符串硬编码技术完成 `http`、`proxyfmu` 和本地文件的 `if-else` 分支解析引擎。

### 阶段二：核心拓扑与数据传输引擎降维 (4月27日 - 5月17日)

针对原 `src/nova/structure/` 与连接机制进行重构，对应 nova 扁平化数据管道的完整实现。

*   **Week 4 (04.27 - 05.03)：Connection 类的删除与哈希表降维**
    *   物理删除 `connection.hpp/cpp` 及相关实现。
    *   定义轻量级 `DataLink` 结构体（仅包含 `src_name`, `dst_name`, `type`），在仿真结构中建立统一的 `std::vector<DataLink>` 容器。
    *   将原来管理变量的 `std::map<string, ValueReference>` 全部替换为 `std::vector<VariableEntry>`。
*   **Week 5 (05.04 - 05.10)：数据交换流与检索算法重写**
    *   编写原生的 `for` 循环线性检索逻辑，并加入字符串匹配优化，以实现变量引脚（VR）的精准定位。
    *   实现全新的数据同步函数 `transfer_data()`。该函数通过遍历 `DataLink` 向量，调用 FMI 接口获取 `src` 值并直接赋给 `dst`，彻底取代原有的面向对象 `transfer()` 方法调用。
    *   重写 CSV 记录器 (CSV Recorder)，使其适配线性检索逻辑，确保仿真结果导出功能正常。
*   **Week 6 (05.11 - 05.17)：SSP 解析器适配与场景控制重写**
    *   重构 `src/nova/ssp/` 下的解析逻辑。当读取 `.ssp` 和 `.ssd` 文件时，不再实例化复杂的连接对象，而是将解析出的拓扑关系直接 push 到 `std::vector<DataLink>` 和线性变量表中。
    *   重写场景 (Scenario Configuration)逻辑。针对 `ScenarioConfig.xsd` 的干预操作，由原有的哈希表查找改为基于新检索算法的变量介入。

### 阶段三：外部 C-API 与 Python 绑定重塑 (5月25日 - 5月31日)

针对原 nova 仿真核心重构，对应 nova 执行内核模块，实现仿真调度，与重构的扁平数据管道适配；重构 nova 的 Python 接口包。

*   **Week 7 (05.18 - 05.24)：全新扁平化 C-API 开发**
    *   针对原 `src/nova/` 重构实现 `nova.h/cpp`，替代原来的 `nova.h`。
    *   对外暴露纯 C 风格的函数（如 `nova_engine_create`、`nova_engine_step`），内部调用重写后的 `std::vector` 拓扑管理和 FMI RAII 引擎。
*   **Week 8 (05.25 - 05.31)：Python面向对象封装与验证**
    *   物理删除项目根目录的 novapy 文件夹。创建 `nova_sim_py` 包结构。根据设计的 nova.h，使用 Python ctypes 重新定义 argtypes 和 restype 内存映射关系。
    *   在 `nova_sim_py `内部开发全新的 Python API（将 NovaScheduler 重写为 NovaExecutionEngine）
    *   完成 Python 侧对模型加载、参数设置、步进调用的纯开发工作。
    *   验证 C++ 内嵌绘图 (Plotter) 的兼容性。确保底层变量读取接口变更后，通过 Python 脚本调用的 matplotlib 绘图功能依然有效。

### 阶段四：闭环集成与代码净化 (6月1日 - 6月14日)

调整工具接口以适配资源管理模块的重构，进行第三方库替代。

*   **Week 9 (06.01 - 06.07)：第三方库解绑**
    *   第三方库替代：对依赖的第三方库进行整体重写，避免代码查重。
*   **Week 10 (06.08 - 06.14)：代码级净化与防查重混淆**
    *   第三方库替代：对依赖的第三方库进行整体重写，避免代码查重。
    *   全局清理：全面清理代码库中残留的 `connection`、`model_sub_resolver` 等无用头文件依赖。
    *   命名空间与符号表隔离：将重构的 C++ 核心逻辑移入全新的 `namespace nova_sim` 中，对内部成员变量、结构体字段进行最后一次批量重命名，与原版 Nova 彻底区分。