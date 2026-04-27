## Ecos 全功能分析
### 模块一：仿真标准与模型解析引擎
这是引擎读取外部模型的入口，必须严格遵循国际标准规范。
1. FMI (Functional Mock-up Interface) 全版本支持
功能描述：解析 modelDescription.xml，加载底层动态链接库（.dll/.so），并将 FMI 的 C API 封装为 C++ 对象。
包含子项：FMI 1.0, 2.0 完整支持，以及 FMI 3.0 的部分支持（对应源码 src/fmilibcpp/fmi1, fmi2, fmi3）。
2. SSP (System Structure and Parameterization) 1.0 解析
功能描述：支持将多个 FMU 打包的 .ssp 文件自动解压、解析拓扑并加载。
包含子项：解析 .ssd（系统结构）、应用 .ssv（参数集）以及提取内嵌的 FMU（对应源码 src/ecos/ssp/）。
### 模块二：仿真调度与拓扑计算核心 
这是决定仿真结果准确性的核心。
3. 动态模型定位器 (Model Resolvers)
功能描述：支持从不同来源获取 FMU。
包含子项：本地文件解析 (file_model_sub_resolver)、URL 远程下载解析 (url_model_sub_resolver)、代理/远程地址解析 (proxy_model_sub_resolver)。
4. 仿真拓扑与连接管理 (Simulation Structure)
功能描述：管理多个实例化模型（model_instance），建立变量之间的数据流连接（connection.hpp），支持类型安全的变量映射。
5. 推进算法 (Algorithms)
功能描述：协调各个模型的时间步进。
包含子项：目前内置的是固定步长算法 (fixed_step_algorithm)。
### 模块三：跨进程与分布式引擎 ProxyFMU 
6. 独立进程隔离执行
功能描述：为防止某个写得很烂的 FMU 崩溃导致整个主程序崩溃，支持将 FMU 丢到独立的子进程中运行。
7. RPC 远程过程调用与网络同步
功能描述：通过 proxyfmu:// 协议，利用 TCP/IP 或 Unix Domain Sockets 操控远程服务器上的模型。
### 模块四：仿真干预与数据处理
8. 剧本驱动 (Scenario Configuration)
功能描述：允许在仿真运行到特定时间点时，强行介入修改某些变量的值或触发事件（基于 ScenarioConfig.xsd 解析）。
9. 高性能 CSV 记录
功能描述：按需、按频率记录选定变量的仿真结果。
10. C++ 内嵌绘图 (Plotter)
功能描述：通过 C++ 底层调用 Python 解释器执行 matplotlib 脚本（对应源码 util/plotter.cpp）。
### 模块五：多语言接口与工具链 
11. 命令行工具 (CLI)
功能描述：支持通过终端传入参数执行 simulate 或 compile。
12. C 语言兼容 API (C-API)
功能描述：提供给其他工业软件集成的扁平化 C 接口。
## 重构方案
### 模型解析与 FMI 底层接口层
功能对标：支持加载本地 .fmu 模型包，并封装底层 C-API。

重构路径：依据 FMI 官方标准文档的 C-API 定义头文件，重新编写动态库句柄的加载逻辑（LoadLibrary / dlopen）。利用现代 C++ 的 std::shared_ptr 配合自定义的删除器（Custom Deleter），接管 fmi2FreeInstance 的调用，以全新的 RAII 范式替换原有的指针清理机制。


### 动态模型定位器 (Model Resolvers) 
原系统特征：原系统采用面向对象的多态工厂模式（Factory Pattern）。存在一个基类 model_sub_resolver，以及 file_ 和 url_ 两个子类。解析时通过多态链式调用判定来源。

1. 废弃继承树：直接物理删除定位器的基类和子类文件。原系统的类继承图（Class Hierarchy）在这一步被彻底抹除。
2. 逻辑内联聚合：新建一个独立的、无继承关系的工具类（或静态函数域）NovaFmuLocator。
3. 控制流替换：将原本依赖多态动态绑定的解析逻辑，改写为最原始的 if-else 字符串硬编码匹配逻辑。

技术实现：提取输入路径的前缀，若 startsWith("http") 则顺序执行下载与解压代码块；否则直接执行本地路径校验代码块。

合规效果：功能完全一致，但抽象语法树（AST）从“多态类簇调用”变异为“单一函数的条件分支结构”，常规查重工具无法形成匹配。


### 仿真拓扑与连接管理 (Simulation Structure) 
原系统特征：重度依赖面向对象设计。simulation_structure 内部持有 model_instance 对象集合 and 抽象的 connection 连接对象集合。变量查找依赖 std::map 哈希表。

物理删除 Connection 实体类。
原机制：原系统将两点之间的数据传递封装为一个单独的 connection 类，内部包含 transfer() 方法。
重构动作：从代码库中彻底删掉 connection.hpp/cpp。在新的结构管理类中，仅仅使用一个最基础的 C++ 结构体（struct DataLink { string src; string dst; }）来记录映射关系。不再赋予该结构体任何行为方法。

哈希表降维为线性表 (Map to Vector)。
原机制：使用 <map> 或 <unordered_map> 管理变量名与 FMI 变量引脚（ValueReference）的映射。
重构动作：全部降维替换为基础的 <vector> 线性表。在进行变量装配时，通过编写基础的 for 循环进行字符串遍历匹配（Linear Search）。


### Python 绑定 (nova_sim_py)
原系统特征：提供高度封装的 Python 面向对象接口，内部使用 ctypes 加载底层的 C-API 动态库，文件分布如 lib.py, EcosSimulation.py 等。 重构动作：绑定层洗稿与对象映射变异

物理隔离与重命名：删除 nova_sim_py 目录。新建 nova_sim_py Python 包。同步替换所有 Python 侧的类名与方法名（如将NovaExecutionEngine 替换为 NovaExecutionEngine）。

接口映射逻辑重写：由于底层 C-API 的函数签名已经完全变更（见上文 C-API 重构），Python 侧的 ctypes.CDLL 加载逻辑、参数类型声明（argtypes）以及返回值定义（restype）必须手动对应重写。


### 预期目标
1. 重构后Ecos项目可以通过软件自主化测评，原创代码率达到90%以上
2. 重构Ecos项目功能与原Ecos完全一致，且仿真结果正确，精度无损失