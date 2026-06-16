# Nova 重构测试方案

## 1. 测试目标准则
*   **正确性保证**：通过对比重构前后相同 FMU 模型的仿真输出（如 CSV 结果），确保数值误差控制。
*   **代码覆盖率**：重构后的核心逻辑模块（FMI 加载、定位器、连接管理等）覆盖率需达到 **70%** 以上。

---

## 2. 针对重构点的测试设计

### 2.1 模型解析与 FMI 底层接口层 (RAII 范式测试)
**重构内容**：使用 `std::shared_ptr` 与自定义删除器管理 FMI 句柄，重写动态库加载逻辑。

*   **测试用例 1：句柄生命周期管理 (Resource Leak Test)**
    *   **描述**：在循环中实例化和释放 FMU 1000次。
    *   **验证点**：使用内存检查工具监测是否存在内存泄漏，确保 `Custom Deleter` 正确调用了 `fmi2FreeInstance`。
*   **测试用例 2：动态库加载稳定性 (Library Loading)**
    *   **描述**：模拟加载不存在的 DLL、非法格式的 DLL、以及标准 FMU DLL。
    *   **验证点**：确保 `NovaFmuLoader` 能够正确抛出异常或返回错误码，而非进程崩溃。

### 2.2 动态模型定位器 (NovaFmuLocator 测试)
**重构内容**：废弃多态继承，改为 `if-else` 字符串硬编码匹配逻辑。

*   **测试用例 3：协议分支判定 (Protocol Dispatching)**
    *   **描述**：输入不同前缀的路径：`http://...`, `C:/...`等。
    *   **验证点**：校验逻辑是否正确进入对应的条件分支，不产生误判。
*   **测试用例 4：路径规范化 (Path Normalization)**
    *   **描述**：输入包含相对路径、反斜杠、中文路径的地址。
    *   **验证点**：确保重构后的静态工具函数能正确解析为合法的本地绝对路径。

### 2.3 仿真拓扑与连接管理 (DataLink 正确性)
**重构内容**：删除 `Connection` 类，改为 `DataLink` 结构体；将 `std::map` 映射改为 `std::vector` 线性遍历。

*   **测试用例 5：数据链路映射 (DataLink Mapping)**
    *   **描述**：手动构建 `vector<DataLink>`，包含多个变量映射关系。
    *   **验证点**：遍历链路并执行数据交换，确保 `src` 的值能够准确同步到 `dst`。
*   **测试用例 6：线性查找效率与边界 (Linear Search Validation)**
    *   **描述**：在含有 500 个变量的 `vector` 中查找特定变量名。
    *   **验证点**：匹配结果必须与原 `std::map` 查找结果一致，测试在大规模变量下的检索延迟是否在可接受范围内。
*   **测试用例 7：类型安全转换**
    *   **描述**：模拟将 Real 类型连接到 Integer 类型（非法连接）。
    *   **验证点**：确保重构后的线性搜索逻辑依然包含类型检查机制，防止非法内存写入。

### 2.4 Python 绑定 (nova_sim_py 适配测试)
**重构内容**：包名变更、类名变更、`ctypes` 接口重新映射。

*   **测试用例 8：Python 接口回归测试**
    *   **描述**：编写 Python 脚本调用 `NovaExecutionEngine` 加载 `ControlledTemperature.fmu`等。
    *   **验证点**：确保 `simulate()` 方法能正常启动，且参数传递（如步长 `dt`）在 C++ 层被正确接收。

---
## 3. 覆盖率与正确性度量
1.  **覆盖率统计**：
    *   使用相关工具(Diff Coverage等)量化。
    *   要求：`src/nova/structure/` (新拓扑逻辑) 与 `src/fmilibcpp/` (新加载逻辑) 等的行覆盖率必须>70\%。
2.  **数值一致性校验**：
    *   读取 `results/` 文件夹下的旧版 CSV 和 Nova 版 CSV。
    *   计算相关参数在相同仿真情况下的误差，保证误差在可接受范围内。

## 4. 测试构建
初步计划在 `tests/nova/CMakeLists.txt` 中新增测试：
```cmake
add_test_executable(test_fmi_raii)
add_test_executable(test_locator)
add_test_executable(test_structure)
add_test_executable(test_nova_api)
```
根据后续实际重构工作继续细化和补充测试