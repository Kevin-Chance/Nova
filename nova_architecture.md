# Nova Nova 完整架构详述文档

本报告详尽列出了重构后的 `nova` 项目的架构，涵盖了所有命名空间、类及其方法。

## 1. 命名空间：`nova_sim` (核心仿真内核)

### 1.1 类：`simulation_structure`
**文件路径**：`include/nova/components/structure/simulation_structure.hpp`
*   **功能**：定义仿真系统的静态拓扑结构、变量连接及模型注册。
*   **内部结构**：
    *   `struct DataLink`: 存储 `src_instance`, `src_variable`, `dst_instance`, `dst_variable`, `type`。
    *   `struct VariableEntry`: 存储 `name`, `instance_name`, `vr`, `type`。
*   **公开方法**：
    *   `simulation_structure()`: 构造函数。
    *   `add_model(const std::string& instanceName, const std::string& uri, std::optional<double> stepSizeHint)`: 根据 URI 添加模型（由 `NovaFmuLocator` 解析）。
    *   `add_model(const std::string& instanceName, const std::filesystem::path& path, std::optional<double> stepSizeHint)`: 根据文件路径添加模型。
    *   `add_model(const std::string& instanceName, std::shared_ptr<model> model, std::optional<double> stepSizeHint)`: 直接注入已加载的模型对象。
    *   `make_connection<T>(variable_identifier source, variable_identifier sink, const std::optional<std::function<T(const T&)>>& modifier)`: 模板方法，建立变量间的连接。
    *   `add_parameter_set(const std::string& name, const std::map<variable_identifier, scalar_value>& paramMap)`: 注册参数集。
    *   `load(std::unique_ptr<algorithm> algorithm)`: 实例化并返回 `nova_engine` 对象。

### 1.2 类：`nova_engine`
**文件路径**：`include/nova/engine/nova_engine.hpp`
*   **功能**：仿真运行时的主控引擎，管理时间步进、数据交换和监听器通知。
*   **公开方法**：
    *   `nova_engine(std::unique_ptr<algorithm> algorithm)`: 构造函数。
    *   `init(std::optional<double> startTime, const std::optional<std::string>& parameterSet)`: 初始化所有模型及算法。
    *   `step(unsigned int numStep)`: 推进指定步数。
    *   `step_until(double timePoint)`: 步进直至达到目标时间点。
    *   `step_for(double duration)`: 步进指定的持续时间。
    *   `terminate()`: 停止仿真。
    *   `reset()`: 重置仿真到初始状态。
    *   `add_slave(std::unique_ptr<model_instance> slave)`: 向仿真中添加一个模型实例。
    *   `add_listener(const std::string& name, std::shared_ptr<engine_observer> listener)`: 注册仿真生命周期监听器。
    *   `remove_listener(const std::string& name)`: 移除监听器。
    *   `get_instance(const std::string& name)`: 返回指定名称的模型实例。
    *   `add_link(const NovaDataLink& link)`: 直接向仿真注入连接关系。
    *   `sync_links()`: 手动触发变量数据同步。
    *   `time()`: 获取当前仿真时间。
    *   `iterations()`: 获取总迭代次数。
    *   `get_real_property(id)` / `get_int_property(id)` 等: 获取属性对象接口。
    *   `load_scenario(const std::filesystem::path& config)`: 加载剧本驱动配置。

### 1.3 类：`properties`
**文件路径**：`include/nova/engine/property.hpp`
*   **功能**：管理模型实例的变量映射。
*   **公开方法**：
    *   `apply_sets()`: 批量将缓存的 setter 值写入底层 FMU。
    *   `apply_gets()`: 触发监听器钩子。
    *   `get_real_property(const std::string& name)`: **线性搜索实现**。
    *   `get_int_property(const std::string& name)`: 线性搜索。
    *   `get_string_property(const std::string& name)`: 线性搜索。
    *   `get_bool_property(const std::string& name)`: 线性搜索。
    *   `get_vector_property(const std::string& name)`: 线性搜索。
    *   `add_real_property(name, p)` / `add_int_property(name, p)` ... : 注册属性。
    *   `hasProperty(const std::string& name)`: 检查变量是否存在。
    *   `get_property_names()`: 获取所有变量名称。

### 1.4 类：`NovaFmuLocator`
**文件路径**：`include/nova/engine/nova_fmu_locator.hpp`
*   **公开方法**：
    *   `static resolve(const std::filesystem::path& base, const std::string& uri)`: 解析模型 URI。
    *   `static resolve(const std::string& uri)`: 默认路径解析。

### 1.5 类：`model_instance` (抽象基类)
**文件路径**：`include/nova/engine/model_instance.hpp`
*   **公开方法**：
    *   `instanceName()`, `stepSizeHint()`: 基本属性获取。
    *   `enter_initialization_mode()`, `exit_initialization_mode()`: 生命周期。
    *   `step(currentTime, stepSize)`: 推进计算。
    *   `terminate()`, `reset()`: 终止与重置。
    *   `apply_parameter_set(name)`: 应用参数集。

### 1.6 算法类
*   **`fixed_step_algorithm`**: `initialize()`, `step()`, `model_instance_added()`。
*   **`first_order_hold_algorithm`**: 一阶保持调度。

---

## 2. 命名空间：`nova_fmi` (底层接口层)

### 2.1 类：`NovaSlave`
**文件路径**：`src/fmilibcpp/nova_slave.hpp`
*   **核心方法**：
    *   `step(t, dt)`: 推进。
    *   `get_real()`, `set_real()`: 数值读写。
    *   `get_integer()`, `set_integer()`: 整型读写。
    *   `get_boolean()`, `set_boolean()`: 布尔读写。
    *   `get_string()`, `set_string()`: 字符串读写。
    *   `get_state()`, `set_state()`, `free_state()`: 状态存取。

### 2.2 类：`NovaFmiLibrary`
**文件路径**：`src/fmilibcpp/nova_fmi_library.hpp`
*   **方法**：
    *   `getFunction<T>(const std::string& name)`: 函数映射。

### 3.3 类：`NovaFmiLoader`
**文件路径**：`src/fmilibcpp/nova_fmi_loader.hpp`
*   **功能**：跨平台 RAII 动态库加载实现。

---

## 3. 全局 C-API (外部集成层)

**头文件**：`include/nova/api/nova.h`
*   **主要函数**：
    *   `nova_simulation_structure_create()` / `_destroy()`
    *   `nova_simulation_structure_add_model()`
    *   `nova_simulation_structure_make_connection()`
    *   `nova_engine_create()` / `_destroy()`
    *   `nova_engine_init()`
    *   `nova_engine_step()`
    *   `nova_engine_get_real() / _set_real()`
    *   `nova_engine_add_csv_recorder(sim, filename)`: 为 C 接口提供 CSV 记录支持。

---

## 4. 辅助工具与分布式逻辑

### 4.1 CSV 记录组件 (`nova_sim::csv_recorder`)
**文件路径**：`include/nova/components/listeners/csv_recorder.hpp`
*   **`csv_config` 方法**：
    *   `register_variable(instance, variable)`: 注册待记录变量。
    *   `register_variable(variable_identifier)`: 通过标识符注册。
    *   `is_empty()`: 检查是否配置了记录变量。
    *   `should_log(inst, var)`: 变量记录判定。
    *   `decimation_factor`: 设置记录频率（步数间隔）。
*   **`csv_recorder` 方法**：
    *   `explicit csv_recorder(const std::string& filename)`: 构造函数。
    *   `config()`: 获取配置句柄。
    *   `post_init(nova_engine& sim)`: 仿真启动时写入表头。
    *   `post_step(nova_engine& sim)`: 每步步进后记录数值。
    *   `post_terminate(nova_engine& sim)`: 仿真结束时关闭文件。

### 4.2 日志系统 (`nova_sim::log`)
**文件路径**：`include/nova/components/logger/logger.hpp`
*   **功能**：提供统一的格式化日志接口。
*   **核心方法**：
    *   `set_logging_level(level)`: 设置日志过滤级别。
    *   `trace()`, `debug()`, `info()`, `warn()`, `err()`: 格式化输出方法。
    *   `log(level, msg)`: 基础记录方法。

### 4.3 SSP 解析 (`nova_sim::load_ssp`)
**文件路径**：`include/nova/components/ssp/ssp_loader.hpp`
*   `std::unique_ptr<simulation_structure> load_ssp(const std::filesystem::path& path)`: 从 `.ssp` 文件包加载完整的仿真拓扑结构。

### 4.4 资源管理工具 (`nova_sim::util`)
*   **`unzipper`**: `extract()`: FMU 解压。
*   **`temp_dir`**: 临时目录生命周期管理。

### 4.5 分布式逻辑 (`proxyfmu`)
*   **类：`proxy_slave`**:
    *   `setup_rpc()`: 建立远程连接。
    *   `step()`, `get_xxx()`, `set_xxx()`: RPC 代理调用。


