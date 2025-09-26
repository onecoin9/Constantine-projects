# C++/Qt 编码与评审规范（v1.0）

> **适用范围**：Winnie/Tigger 等基于 C++17+/Qt 6 的客户端、测试与自动化项目
> **目标读者**：研发、Code Review、QA、运维
> **关联文档**：`spdlog-logging-standard.md`（日志治理）、项目《发布流程》《测试策略》等
> **导出指南**：若需提交 Word 版本，可使用仓库内 `Owl/md_to_docx.py` 脚本转换

---

## 目录

1. [基本设计原则](#基本设计原则)
2. [项目结构与文件组织](#项目结构与文件组织)
3. [命名与代码风格](#命名与代码风格)
4. [现代 C++ 语言特性](#现代-c-语言特性)
5. [内存与资源管理](#内存与资源管理)
6. [错误处理与诊断](#错误处理与诊断)
7. [Qt 框架专项规范](#qt-框架专项规范)
8. [多线程与并发](#多线程与并发)
9. [UI 与交互层规范](#ui-与交互层规范)
10. [构建、依赖与配置管理](#构建依赖与配置管理)
11. [测试与质量保障](#测试与质量保障)
12. [安全与隐私要求](#安全与隐私要求)
13. [性能优化与资源消耗](#性能优化与资源消耗)
14. [代码评审清单](#代码评审清单)
15. [变更记录](#变更记录)

---

## 基本设计原则

- **可读性优先**：为后续维护与评审提供清晰意图，适度添加注释、设计描述和示例。
- **最小惊讶原则**：遵循业界和团队约定，避免奇特或隐式行为。
- **单一职责**：模块、类、函数应聚焦于单一职责，配合 SOLID 原则进行设计。
- **可测试性**：编写可被单元测试、集成测试覆盖的结构；解耦硬件/I/O 依赖。
- **跨平台一致性**：Windows、Linux（如适用）需维护一致行为，避免平台条件编译带来行为差异。

## 项目结构与文件组织

- **目录层级**：遵循 `src/`, `include/`, `tests/`, `resources/`, `docs/`, `tools/` 等标准布局。
- **模块划分**：以功能域拆分（如 `Device/`, `Workflow/`, `UI/`），公共组件放 `common/`、`core/`。
- **头文件/源文件配对**：`ClassName.h/.cpp` 成对出现；接口定义放头文件，内部实现置于 `.cpp`。
- **CMake/Qt 项目文件**：
  - `CMakeLists.txt` 按模块拆分，使用 `target_link_libraries`、`target_compile_definitions` 维护依赖。
  - Qt `.pro` / `.pri` 文件需与 CMake 配置保持同步，避免重复或冲突。
- **资源与翻译文件**：图片、样式、QML 放入 `resources/`，通过 `qrc` 统一管理；字符串使用 `qsTr()` 并维护 `.ts` 语言包。

## 命名与代码风格

- **命名约定**：
  - 类型（类、结构、枚举）使用 `PascalCase`，命名应表达责任，如 `PressureController`。
  - 函数、方法采用 `camelCase`，动宾短语表示动作，如 `startMeasurement()`。
  - 变量：局部变量使用 `camelCase`；类成员变量以 `m_` 前缀（只在成员上使用，避免与局部变量混淆）；静态成员以 `s_` 前缀。跨线程访问的共享状态应以明显名称（如 `m_sharedState`）突出用途。
  - 常量及枚举值使用 `SCREAMING_SNAKE_CASE`。
- **格式化与排版**：
  - 统一使用 UTF-8（无 BOM），行尾 LF。
  - 缩进 2 或 4 空格（团队约定），禁止 Tab 混用；请使用仓库根目录的 `.clang-format` 配置（或 `tools/style/clang-format.yaml`）统一格式，提交前执行 `clang-format`/`ninja format`。
  - 每行不超过 120 列，表达式适度换行；链式调用分行对齐。
- **注释**：
  - 使用 `//` 单行、`/* ... */` 多行，Doxygen 注释 `///` 或 `/** */` 描述公共接口。
  - 注释描述“为什么”而非“做什么”，避免陈述代码本身。
- **头文件防护**：采用 `#pragma once`（首选）或传统 include guard。

## 现代 C++ 语言特性

- **标准版本**：默认 C++17 及以上，启用 `<filesystem>`, `<optional>`, `<variant>` 等特性。
- **智能指针**：优先使用 `std::unique_ptr`，必要时使用 `std::shared_ptr` + `std::weak_ptr`，避免裸指针拥有权。
- **RAII**：使用构造/析构管理资源，例如 `QScopedPointer`, `std::lock_guard`，禁止手写 `new/delete`。
- **`auto` 使用**：仅在类型显而易见、或冗长模板类型时使用；避免降低可读性。
- **枚举类**：使用 `enum class` 代替传统枚举，并提供转换函数或 `QMetaEnum` 集成。
- **constexpr/const**：常量、魔法数字使用 `constexpr` 或 `const`；函数尽可能声明 `noexcept`。
- **[[nodiscard]] 与 std::span**：返回值影响流程的函数务必使用 `[[nodiscard]]`；处理连续缓冲区时优先使用 `std::span`/`gsl::span`，同时注明 Qt 5/6 兼容策略。
- **结构化绑定 / if with init**：在遍历 Qt 容器、pair 返回值时可使用，注意编译器版本兼容。

## 内存与资源管理

- **对象生命周期**：
  - Qt 对象若有父子关系，使用父指针自动管理；无父对象时使用智能指针。
  - 避免在栈上创建大型 QObject 子类（包含信号槽），优先使用 `std::unique_ptr<QObject>`。
- **容器选择**：
  - 优先使用标准容器；若需 Qt 特性（如隐式共享、信号槽自动更新）可使用 `QVector`, `QMap` 等。
  - 明确迭代器与索引安全性，避免越界、迭代器失效。
- **资源封装**：对外设句柄、文件、套接字等封装为 RAII 类，确保异常或早退路径自动释放。
- **内存泄漏检测**：启用 ASan/Valgrind（Linux）或 Visual Studio 内存诊断（Windows）进行验证。

## 错误处理与诊断

- **异常策略**：
  - 核心业务逻辑优先返回 `Expected<T, Error>`/`std::optional`/`outcome::result` 等显式错误模型。
  - Qt 信号槽或回调中避免抛出异常；如需，必须捕获并记录。
- **错误码体系**：统一定义错误枚举或结构体（含 `code`, `desc`, `severity`）。
- **日志与追踪**：
  - 遵循 `spdlog-logging-standard.md`，通过统一封装宏记录上下文。
  - `ERROR/FATAL` 级别必须包含 `module`, `event`, `traceId`, `error.code` 等关键信息。
- **断言与验证**：
  - 使用 `Q_ASSERT`, `Q_ASSERT_X` 或 `gsl::Expects` 确保前置条件；Release 中保留必要的运行时代码路径检查。

## Qt 框架专项规范

- **QObject 与信号槽**：
  - QObject 子类需使用 `Q_OBJECT` 宏，确保元对象系统生效。
  - 新 API 使用`functor`/`lambda` + `connect(sender, &Sender::signal, this, [=]{})` 的强类型连接；禁止旧式字符串连接。
  - 跨线程连接需显式指定 `Qt::QueuedConnection` 并确认目标线程拥有事件循环。
- **内存父子关系**：创建 QObject 时立即传入父对象，或调用 `setParent()`；禁止在析构中手写删除子对象。
- **线程亲和性**：
  - UI 操作必须在主线程 (`QApplication::instance()->thread()`) 执行。
  - 使用 `moveToThread()` 时，仅在对象构造完成且未开始处理事件前调用；确保目标线程启动事件循环后再触发信号，线程结束时需回到原线程释放。
- **QML / Qt Quick**：
  - C++ 暴露 QML 类型需通过 `qmlRegisterType`/`qmlRegisterSingletonType`，避免直接暴露裸指针。
  - 在 QML 中调用 C++ 接口，应使用 async/await 或信号反馈，防止阻塞 UI。
- **资源加载**：使用 `QResource` 或 `QFile` 访问资源；路径统一使用正斜杠，禁止硬编码绝对路径。

## 多线程与并发

- **线程创建**：优先使用 `QThreadPool`, `QtConcurrent`, `std::async`；避免裸 `std::thread`/`CreateThread`。
- **同步原语**：使用 `std::mutex`, `std::shared_mutex`, `QMutex`；搭配 RAII (`std::lock_guard`)。
- **数据共享**：
  - 明确读写锁策略，避免双向锁导致死锁；跨线程访问 UI/QObject 属性需通过信号槽或 `QMetaObject::invokeMethod`。
  - 共享状态封装为线程安全类，禁止全局可变单例。
- **原子与无锁**：使用 `std::atomic` 管理计数器、标志位；复杂无锁结构需评审通过并配备测试。

## UI 与交互层规范

- **界面布局**：
  - 优先使用 Qt Designer + `.ui` 或 QML 文件；禁止在 C++ 中手写大量布局代码。
  - 使用布局管理器（`QGridLayout`, `QVBoxLayout`）确保缩放自适应；资源字符串从翻译文件加载。
- **高分屏适配**：统一启用 `QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling)`、`Qt::AA_UseHighDpiPixmaps`（Qt 6 默认开启）；字体通过 `QFontDatabase::systemFont(QFontDatabase::GeneralFont)` 获取；在 UI 设计中使用布局而非固定像素计算。
- **状态管理**：
  - UI 控件状态与业务模型解耦，使用 `Model-View`、`MVVM` 模式；通过信号槽同步。
  - 复杂流程需有状态机（`QStateMachine` 或自定义 FSM ）并记录状态转换。
- **可用性与国际化**：
  - 文字使用 `qsTr()` 并在翻译文件中维护；UI 需适配中英文长度。
  - 重要操作提供确认与撤销机制；错误提示需包含诊断信息。

## 构建依赖与配置管理

- **CMake 规范**：
  - 使用 `FetchContent`/`find_package` 管理依赖，禁止手工拷贝二进制到仓库根目录。
  - 所有目标显式声明 `target_compile_features`、`target_include_directories`、`target_compile_definitions`。
  - Release 构建启用 `-DNDEBUG`、`-O2`，Debug 构建启用 `-DDEBUG`、`/Zi`（MSVC）。
- **Qt 工具链**：
  - 使用 `qt_add_executable`/`qt_add_qml_module`（Qt 6），启用 `AUTOMOC/AUTORCC/AUTOUIC`。
  - 确保 `windeployqt`/`macdeployqt` 步骤纳入 CI/CD。
- **配置文件**：
  - 所有可调参数放入 JSON/YAML/INI 配置，并提供版本字段。
  - 配置结构定义集中管理，提供加载、校验、热更新机制。

## 测试与质量保障

- **测试层级**：
  - 单元测试（`QTest`, `Catch2`, `GoogleTest` 等）覆盖核心算法和业务规则。
  - 集成测试模拟设备/网络交互，使用 Mock 或仿真器隔离外部依赖。
  - UI 自动化测试（`squish`, `Qt Test` Events, `pytest-qt`）用于关键场景回归。
- **覆盖率目标**：核心模块 > 70%，关键路径（安全、计费、设备控制） > 90%。
- **静态分析**：启用 `clang-tidy`, `cppcheck`, `include-what-you-use`；Qt 项目启用 `lupdate`、`lrelease` 检测翻译缺失。
- **CI/CD**：在流水线中执行构建、测试、打包，输出制品及测试报告；失败自动阻断。

## 安全与隐私要求

- **数据脱敏**：敏感字段遵循公司脱敏策略，与日志标准一致。
- **凭据管理**：禁止硬编码账号、口令、密钥；通过 `QSettings` + OS 凭据库或专用服务加载。
- **输入校验**：对外部数据（TCP/串口/MES）进行严格校验，避免注入。
- **权限控制**：界面和命令应按照角色、工位权限控制；日志记录权限变更。
- **依赖漏洞扫描**：定期检查第三方库版本，关注 Qt 安全公告。

## 性能优化与资源消耗

- **性能预算**：在 PRD 或设计阶段明确 CPU、内存、时延指标。
- **剖析工具**：使用 `Perf`, `VTune`, `Visual Studio Profiler`, `Qt Creator Analyzer` 定期分析热点。
- **懒加载与缓存**：
  - UI 组件惰性初始化，避免一次性加载全部页面。
  - 设备通讯结果可缓存，设置过期策略。
- **异步 I/O**：串口/网口采用异步读写；UI 线程不可阻塞，必要时展示进度/加载动画。
- **资源释放**：确保长时间运行后内存、句柄无增长；使用 `QTimer` 清理过期资源。

## 代码评审清单

| 检查项 | 关键问题 | 典型陷阱 | 评审记录 |
|--------|----------|----------|----------|
| 设计符合单一职责 | 模块边界是否清晰，依赖是否必要 | 类/模块耦合过紧，隐藏全局状态 | ☑/☐ 说明 |
| 命名与风格一致 | 符合命名约定与 clang-format 规则 | 匈牙利命名、蛇形混用，行宽超限 | ☑/☐ 说明 |
| 现代 C++ 使用 | 智能指针、RAII 是否正确使用 | 裸指针泄漏、`shared_ptr` 环引用 | ☑/☐ 说明 |
| Qt 信号槽正确 | 强类型连接？跨线程连接显式声明？ | 旧式 SIGNAL/SLOT 字符串、槽未解绑 | ☑/☐ 说明 |
| 线程安全 | 跨线程访问是否安全、锁粒度合理 | UI 线程阻塞、死锁、数据竞争 | ☑/☐ 说明 |
| 错误处理完备 | 是否返回/传播错误码，日志上下文齐全 | 静默失败、异常泄露到 Qt 事件循环 | ☑/☐ 说明 |
| 安全与隐私 | 是否存在敏感信息泄露、权限漏洞 | 凭据硬编码、日志泄露密码 | ☑/☐ 说明 |
| 测试覆盖充分 | 单元/集成测试、自动化脚本是否更新 | 漏测关键路径、测试数据过旧 | ☑/☐ 说明 |
| 性能与资源 | 是否评估性能影响，资源释放 | 循环中动态分配、内存增长 | ☑/☐ 说明 |
| 文档与注释 | 接口、协议、配置是否有文档更新 | 忘记更新 README/版本说明 | ☑/☐ 说明 |

> **提示**：评审记录需提交至代码平台或质量系统，确保问题整改可追踪。

## 变更记录

| 版本 | 日期 | 负责人 | 变更内容 |
|------|------|--------|----------|
| v1.0 | 2025-09-26 | Winnie 团队 | 初版，覆盖 C++/Qt 编码、评审要点 |
