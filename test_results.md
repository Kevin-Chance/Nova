# ecos_nova 核心功能测试映射与结果报告

本报告详细记录了 `nova_ecos` 重构后的测试覆盖情况。所有测试均基于 `build\bin` 下的真实可执行文件运行，结果通过率 100%。

---

## 1. 基础架构测试 (Infrastructure)

### tests/ecos/test_variable_identifier.cpp
*   **测试功能**：验证变量唯一标识符的解析与比较逻辑（如 `instance::variable` 格式）。
*   **涵盖代码**：`include/ecos/variable_identifier.hpp`
*   **终端输出**：
```
Randomness seeded to: 1227609031
All tests passed (10 assertions in 1 test case)
```

### tests/ecos/test_property.cpp
*   **测试功能**：测试扁平化架构后的属性系统，包括值的获取（Get）、延迟设置（Set）及同步（Apply）。
*   **涵盖代码**：`include/ecos/property.hpp`
*   **终端输出**：
```
Randomness seeded to: 2611556297
All tests passed (8 assertions in 1 test case)
```

### tests/ecos/test_unzipper.cpp
*   **测试功能**：验证 FMU 及 SSP 压缩包的跨平台解压逻辑。
*   **涵盖代码**：`src/util/unzipper.cpp`, `include/util/unzipper.hpp`
*   **终端输出**：
```
Randomness seeded to: 2154163740
All tests passed (8 assertions in 1 test case)
```

---

## 2. FMI 标准兼容性驱动测试 (FMI Drivers)

### tests/fmilibcpp/test_identity.cpp
*   **测试功能**：**FMI 1.0** 核心驱动兼容性，验证回调函数（Callbacks）及基本读写。
*   **涵盖代码**：`src/fmilibcpp/fmi1/fmi1_fmu.cpp`, `src/fmilibcpp/fmu.cpp`
*   **终端输出**：
```
Randomness seeded to: 1764305321
All tests passed (17 assertions in 1 test case)
```

### tests/fmilibcpp/test_controlled_temp.cpp
*   **测试功能**：**FMI 2.0** 驱动兼容性，验证 Co-Simulation 实例化及步进逻辑。
*   **涵盖代码**：`src/fmilibcpp/fmi2/fmi2_fmu.cpp`
*   **终端输出**：
```
Randomness seeded to: 2183348659
All tests passed (10 assertions in 1 test case)
```

### tests/fmilibcpp/test_bouncingball.cpp
*   **测试功能**：**FMI 3.0** 驱动兼容性，验证新标准下的 `instantiationToken` 及 Float64 接口。
*   **涵盖代码**：`src/fmilibcpp/fmi3/fmi3_fmu.cpp`
*   **终端输出**：
```
Randomness seeded to: 2998991037
All tests passed (20 assertions in 1 test case)
```

### tests/fmilibcpp/test_state.cpp
*   **测试功能**：验证 FMI 状态获取与恢复（Get/Set FMU State），确保仿真回溯能力。
*   **涵盖代码**：各版本驱动中的 `get_state`/`set_state` 实现。
*   **终端输出**：
```
Randomness seeded to: 2438579564
All tests passed (12 assertions in 1 test case)
```

---

## 3. 仿真引擎与系统解析测试 (Engine & Parsing)

### tests/ecos/test_ssp_parser.cpp
*   **测试功能**：验证符合 SSP 1.0 标准的系统结构定义文件解析。
*   **涵盖代码**：`src/ecos/ssp/ssp.cpp`, `include/ecos/ssp/ssp.hpp`
*   **终端输出**：
```
Randomness seeded to: 4068225918
All tests passed (80 assertions in 2 test cases)
```

### tests/ecos/test_runner.cpp
*   **测试功能**：测试异步仿真运行器，验证实时因子（RTF）控制及线程同步。
*   **涵盖代码**：`src/ecos/simulation_runner.cpp`, `include/ecos/simulation_runner.hpp`
*   **终端输出**：
```
Randomness seeded to: 225406168
[ecos] [debug] Simulated 0.110s in 0.1045s, RTF=1.053
All tests passed (3 assertions in 1 test case)
```

### tests/ecos/test_clib.cpp
*   **测试功能**：验证暴露给外部调用的 C 语言兼容接口（用于 Python/C 调用）。
*   **涵盖代码**：`src/ecos/nova_ecos.cpp`, `include/ecos/nova_ecos.h`
*   **终端输出**：
```
Randomness seeded to: 694375777
Using libecos version: 0.5.8
All tests passed (5 assertions in 1 test case)
```

---

## 4. 综合稳定性测试 (Mixed Validation)

### tests/reference_fmus/test_reference_fmus.cpp
*   **测试功能**：遍历所有版本的 FMI 参考库进行批量实例化与步进压力测试。
*   **涵盖代码**：`src/ecos/nova_fmu_locator.cpp`, `src/fmilibcpp/fmu.cpp`
*   **终端输出**：
```
Randomness seeded to: 1775006905
[ecos] [warning] Failed to remove temp folder ... (Windows DLL Lock Delay)
All tests passed (1 assertion in 1 test case)
```
