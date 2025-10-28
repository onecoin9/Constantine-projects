# C++/Qt 编码与评审规范（v1.0）

> **适用范围**：Winnie/Tigger 等基于 C++17+/Qt 6 的客户端、测试与自动化项目
> **目标读者**：研发、Code Review、QA、运维
> **关联文档**：`spdlog-logging-standard.md`（日志治理）、项目《发布流程》《测试策略》等
> **导出指南**：若需提交 Word 版本，可使用仓库内 `Owl/md_to_docx.py` 脚本转换
> **参考依据**：华为 C 语言编程规范等业界优秀实践，结合 Qt/C++17 特性改写适用条款

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
15. [附录：华为 C 语言编程规范整理与示例](#附录华为-c-语言编程规范整理与示例)
16. [变更记录](#变更记录)

---

## 基本设计原则

- **可读性优先**：为后续维护与评审提供清晰意图，适度添加注释、设计描述和示例。
- **最小惊讶原则**：遵循业界和团队约定，避免奇特或隐式行为。
- **单一职责**：模块、类、函数应聚焦于单一职责，配合 SOLID 原则进行设计。
- **可测试性**：编写可被单元测试、集成测试覆盖的结构；解耦硬件/I/O 依赖。
- **跨平台一致性**：Windows、Linux（如适用）需维护一致行为，避免平台条件编译带来行为差异。

## 项目结构与文件组织

- **目录层级**：遵循 `src/`, `include/`, `tests/`, `resources/`, `docs/`, `tools/` 等标准布局。
  - 顶层目录保持职责单一，禁止在同一目录混杂接口、实现、测试。
- **模块划分**：以功能域拆分（如 `Device/`, `Workflow/`, `UI/`），公共组件放 `common/`、`core/`。
- **头文件/源文件配对**：`ClassName.h/.cpp` 成对出现；接口定义放头文件，内部实现置于 `.cpp`。
  - 头文件仅暴露对外接口，内部工具函数、临时结构体禁止放在 `.h` 中。
  - 包含关系应"由不稳定指向稳定"：头文件尽量只依赖更通用/更稳定的模块，避免交叉依赖。
  - **前向声明**：必要时使用前向声明降低耦合，仅声明类名而不包含完整定义（用于指针/引用参数）。禁止在头文件中 `using namespace`。
    - 前向声明适用场景：
      ```cpp
      // device_manager.h
      class PressureSensor;     // ✓ 前向声明，仅告诉编译器有这个类
      class ConfigLoader;
      
      class DataProcessor {
      public:
        void setDevice(PressureSensor* mgr);        // ✓ 指针参数，前向声明够
        std::optional<std::string> loadConfig();    // ✓ 返回值，前向声明够
      private:
        PressureSensor* m_device;                   // ✓ 指针成员，前向声明够
        // PressureSensor m_sensor;                 // ✗ 成员变量需要完整定义
      };
      
      // device_processor.cpp —— 只在 .cpp 中包含完整定义
      #include "device_manager.h"
      #include "pressure_sensor.h"  // ✓ 在这里包含
      ```
    - 优势：修改 `PressureSensor` 内部 → `DataProcessor.h` 不需重新编译，降低编译依赖。
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
  - 类型/函数命名禁止使用无意义缩写（如 `tmp`, `doStuff`）；必要缩写需在注释中解释来源。
  - 布尔变量以 `is/has/should/can` 开头；集合使用复数名词（如 `devices`）。
  - **PascalCase vs camelCase 快速参考**：
    - **PascalCase**（首字母大写）：类、结构体、枚举名 → `class DeviceManager`, `enum class MeasurementState`
    - **camelCase**（首字母小写）：函数、方法、变量 → `void startMeasurement()`, `int sensorCount`
    - **SCREAMING_SNAKE_CASE**（全大写下划线）：常量 → `const int MAX_RETRY_COUNT = 3`
    - 示例：
      ```cpp
      // ✓ 正确的命名
      class TemperatureController {  // 类：PascalCase
      public:
        void startMonitoring();      // 函数：camelCase
        double getCurrentTemp();     // 函数：camelCase
      private:
        int m_sensorId;              // 成员：camelCase + m_
        bool m_isMonitoring;         // 布尔变量：is 前缀
        static constexpr int MAX_SENSORS = 24;  // 常量：SCREAMING_SNAKE_CASE
      };
      ```
  - **代码示例详解**：
    - **推荐的命名规范**：
      ```cpp
      // 类型定义：PascalCase
      class PressureController {
      private:
        double m_threshold;              // 类成员：m_ 前缀
        bool m_isInitialized;            // 布尔变量：is 前缀
        static int s_instanceCount;      // 静态成员：s_ 前缀
      
      public:
        void startMeasurement();         // 函数：camelCase
        bool validateInput(double value);
        const auto& getThreshold() const { return m_threshold; }
      };
      
      // 常量：SCREAMING_SNAKE_CASE
      constexpr double PRESSURE_MIN = 0.0;
      constexpr double PRESSURE_MAX = 500.0;
      constexpr int MAX_RETRY_COUNT = 3;
      
      // 枚举类：使用 enum class
      enum class MeasurementState {
        Idle,
        Running,
        Paused,
        Completed
      };
      ```
    
    - **反例：混乱的命名**：
      ```cpp
      // ✗ 反例1：无意义缩写，名字太短
      class PC {
      private:
        double threshold;               // ✗ 缺少 m_ 前缀
        bool init;                      // ✗ 缺少 is/has 前缀
        static int gInstanceCount;      // ✗ 不规范前缀
      
      public:
        void SMM();                     // ✗ 缩写过度，难以理解
        bool ValidateInput(double x);   // ✗ 函数使用 PascalCase
      };
      
      // ✗ 反例2：混用命名风格
      #define MAX_SIZE 100;             // ✗ 宏定义常量（应使用 constexpr）
      int tmp_value = 0;                // ✗ tmp 无实际意义
      double d = 3.14;                  // ✗ 单字母变量名
      bool flag = true;                 // ✗ flag 过于模糊
      enum MeasurementState {           // ✗ 使用传统枚举
        Idle, Running, Paused
      };
      ```
- **格式化与排版**：
  - 统一使用 UTF-8（无 BOM），行尾 LF。
  - 缩进 2 或 4 空格（团队约定），禁止 Tab 混用；请使用仓库根目录的 `.clang-format` 配置（或 `tools/style/clang-format.yaml`）统一格式，提交前执行 `clang-format`/`ninja format`。
  - 每行不超过 120 列，表达式适度换行；链式调用分行对齐。
  - 不在同一文件内混用 CRLF；提交前执行 `git diff --check` 确保无尾随空格。
  - 块结构遵循"左对齐，右移"原则，`if/else/while` 块必须使用 `{}`，即便只有一行。
  - **代码排版示例**：
    - **推荐的规范格式**：
      ```cpp
      // ✓ 推荐：清晰的块结构，适度的缩进
      class DeviceMonitor {
      public:
        explicit DeviceMonitor(const QString& deviceName)
          : m_deviceName(deviceName)
          , m_isConnected(false)
          , m_pollInterval(500)
        {
        }
      
        void start() {
          if (!ensureConnection()) {
            LOGE("monitor.start.failed", "Cannot connect to device");
            return;
          }
      
          m_isRunning = true;
          QTimer::singleShot(0, this, &DeviceMonitor::poll);
        }
      
      private:
        QString m_deviceName;
        bool m_isConnected = false;
        bool m_isRunning = false;
        int m_pollInterval = 500;
      };
      ```
- **注释**：
  - 使用 `//` 单行、`/* ... */` 多行，Doxygen 注释 `///` 或 `/** */` 描述公共接口。
  - 注释描述“为什么”而非“做什么”，避免陈述代码本身。
  - 文件头注释需包含版权信息、作者/维护人、最后更新时间、用途说明。
  - 公共 API 在头文件撰写 Doxygen 注释；重要算法、状态机在实现文件追加流程说明。
  - 注释须与代码保持一致，删除废弃代码时同步移除对应注释。
- **头文件防护**：采用 `#pragma once`（首选）或传统 include guard。
  - include guard 命名遵循 `PROJECT_MODULE_FILENAME_H` 模式。

## 现代 C++ 语言特性

- **标准版本**：默认 C++17 及以上，启用 `<filesystem>`, `<optional>`, `<variant>` 等特性。
- **智能指针**：优先使用 `std::unique_ptr`，必要时使用 `std::shared_ptr` + `std::weak_ptr`，避免裸指针拥有权。
- **RAII**：使用构造/析构管理资源，例如 `QScopedPointer`, `std::lock_guard`，禁止手写 `new/delete`。
- **`auto` 使用**：仅在类型显而易见、或冗长模板类型时使用；避免降低可读性。
- **枚举类**：使用 `enum class` 代替传统枚举，并提供转换函数或 `QMetaEnum` 集成。
- **constexpr/const**：常量、魔法数字使用 `constexpr` 或 `const`；函数尽可能声明 `noexcept`。
- **[[nodiscard]] 与 std::span**：返回值影响流程的函数务必使用 `[[nodiscard]]`；处理连续缓冲区时优先使用 `std::span`/`gsl::span`，同时注明 Qt 5/6 兼容策略。
- **结构化绑定 / if with init**：在遍历 Qt 容器、pair 返回值时可使用，注意编译器版本兼容。
- **命名空间使用**：
  - **禁止在头文件中 `using namespace`**（污染全局命名空间，导致名字冲突）
    - ✗ 错误做法：`using namespace std;` 放在头文件
    - ✓ 正确做法 1：头文件中显式写 `std::string`, `std::vector` 等完整名
    - ✓ 正确做法 2：`using namespace` 仅在 `.cpp` 文件或函数作用域内
  - 示例：
    ```cpp
    // processor.h —— 头文件中始终用完整名
    #pragma once
    #include <string>
    #include <vector>
    
    class DataProcessor {
    public:
      void process(const std::string& data);      // ✓ 显式 std::
      std::vector<int> getResults() const;        // ✓ 显式 std::
    };
    
    // processor.cpp —— .cpp 中可以 using namespace
    #include "processor.h"
    using namespace std;  // ✓ 只在 .cpp 中，不污染其他文件
    
    void DataProcessor::process(const string& data) {  // 可以省略 std::
      // ...
    }
    ```
  - 为什么禁止：
    - 避免名字冲突（多个库可能有 `string`、`vector` 等同名类型）
    - 防止污染全局命名空间（使用头文件的代码意外获得库的所有符号）
    - 提高代码清晰度（一眼看出每个符号来自哪里）
- **宏使用限制**：
  - 禁止滥用宏定义常量或内联函数，优先使用 `constexpr`、`inline`；确需宏时全部大写并添加注释。
  - 宏替换内容需加括号防止优先级问题；宏函数内部避免副作用。

## 内存与资源管理

- **RAII 原则**（Resource Acquisition Is Initialization）：
  - **核心思想**：用对象的生命周期（构造/析构）来管理资源，确保"获取资源即初始化，销毁对象即释放资源"。
  - **优势**：异常安全、无内存泄漏、代码清晰。
  - **应用场景**：
    - **文件管理**：`std::ifstream file("data.txt")` → 构造打开，析构自动关闭
    - **内存管理**：`auto ptr = std::make_unique<Device>()` → 构造分配，析构自动释放
    - **互斥锁**：`std::lock_guard<std::mutex> lock(mu)` → 构造加锁，析构自动解锁
    - **Qt 对象**：`new QPushButton(parent)` → 构造创建，parent 析构时自动删除子对象
  - 示例对比：
    ```cpp
    // ❌ 非 RAII（手动管理，容易泄漏）
    void processData() {
      FILE* file = fopen("data.txt", "r");
      PressureSensor* sensor = new PressureSensor();
      
      if (error) {
        return;  // ✗ 资源未释放！
      }
      
      fclose(file);
      delete sensor;
    }
    
    // ✓ RAII（自动管理，异常安全）
    void processData() {
      std::ifstream file("data.txt");                    // 构造打开
      auto sensor = std::make_unique<PressureSensor>();  // 构造创建
      
      if (error) {
        throw std::runtime_error("Error");  // ✓ 资源自动释放
      }
    }  // ← 自动释放资源
    ```

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
  - 新 API 使用 `functor`/`lambda` + `connect(sender, &Sender::signal, this, [=]{})` 的强类型连接；禁止旧式字符串连接。
  - 跨线程连接需显式指定 `Qt::QueuedConnection` 并确认目标线程拥有事件循环。
  - **信号槽示例**：
    - **推荐：使用新式 functor 连接**：
      ```cpp
      // ✓ 推荐：现代 Qt 信号槽
      class DataProcessor : public QObject {
        Q_OBJECT
      public:
        void connectSignals(DeviceManager* device) {
          // 强类型连接，编译期检查
          connect(device, &DeviceManager::dataReady,
                  this, &DataProcessor::onData,
                  Qt::QueuedConnection);
        }
      
      private slots:
        void onData(const QByteArray& data) {
          // 处理数据
        }
      
      signals:
        void errorOccurred(int code);
      };
      ```
    
    - **反例：旧式字符串连接**：
      ```cpp
      // ✗ 反例：过时的 SIGNAL/SLOT 写法
      class OldDevice : public QObject {
        Q_OBJECT
      public:
        void setup(QObject* receiver) {
          // ✗ 字符串连接，无编译检查，易出错
          connect(this, SIGNAL(dataReady(QByteArray)),
                  receiver, SLOT(onData(QByteArray)));
        }
      
      signals:
        void dataReady(const QByteArray& data);
      };
      ```
- **内存父子关系**：创建 QObject 时立即传入父对象，或调用 `setParent()`；禁止在析构中手写删除子对象。
- **线程亲和性**：
  - UI 操作必须在主线程 (`QApplication::instance()->thread()`) 执行。
  - 使用 `moveToThread()` 时，仅在对象构造完成且未开始处理事件前调用；确保目标线程启动事件循环后再触发信号，线程结束时需回到原线程释放。
  - **跨线程通讯示例**：
    ```cpp
    // ✓ 推荐：正确的线程亲和性管理
    class SocketWorker : public QObject {
      Q_OBJECT
    public:
      void moveToWorkerThread(QThread* thread) {
        QObject::moveToThread(thread);  // 必须先 moveToThread
        connect(thread, &QThread::started,
                this, &SocketWorker::startListening);
      }
    
    private slots:
      void startListening() {
        // 此时已在工作线程中，可以安全使用套接字
      }
    };
    ```
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
- **多线程最佳实践示例**：
  - **推荐：使用线程池和 RAII 锁**：
    ```cpp
    // ✓ 推荐：使用高级线程工具
    class DataAggregator : public QObject {
      Q_OBJECT
    public:
      void aggregateData() {
        // 使用 QtConcurrent 进行异步处理
        auto future = QtConcurrent::run(this, &DataAggregator::heavyComputation);
        m_watcher->setFuture(future);
      }
    
    private slots:
      void onResultReady() {
        emit resultReady(m_watcher->result());
      }
    
    private:
      QVector<Data> heavyComputation() {
        // 长时间运算，不阻塞 UI
        return processData();
      }
    
      std::unique_ptr<QFutureWatcher<QVector<Data>>> m_watcher;
    
    signals:
      void resultReady(const QVector<Data>& data);
    };
    
    // ✓ 推荐：使用 RAII 锁保护共享数据
    class ThreadSafeCache {
    private:
      mutable std::shared_mutex m_mutex;
      std::unordered_map<std::string, std::string> m_data;
    
    public:
      std::string get(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);  // 读锁
        auto it = m_data.find(key);
        return it != m_data.end() ? it->second : "";
      }
    
      void set(const std::string& key, const std::string& value) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);  // 写锁
        m_data[key] = value;
      }
    };
    ```
  
  - **反例：线程管理不当**：
    ```cpp
    // ✗ 反例1：手工管理线程（容易泄漏）
    class BadAggregator {
    private:
      std::thread m_thread;  // ✗ 需要手工 join/detach
    
    public:
      void start() {
        m_thread = std::thread(&BadAggregator::run, this);
      }
    
      ~BadAggregator() {
        // ✗ 析构中 join 会阻塞，detach 会泄漏
      }
    };
    
    // ✗ 反例2：数据竞争
    class BadCache {
    private:
      std::unordered_map<std::string, std::string> m_data;  // ✗ 无保护
    
    public:
      void set(const std::string& key, const std::string& value) {
        m_data[key] = value;  // ✗ 竞争！
      }
    };
    
    // ✗ 反例3：死锁
    class DeadlockExample {
    private:
      std::mutex m_mutex1, m_mutex2;
    
    public:
      void threadA() {
        std::lock_guard<std::mutex> lock1(m_mutex1);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::lock_guard<std::mutex> lock2(m_mutex2);  // ✗ 等待 lock2
      }
    
      void threadB() {
        std::lock_guard<std::mutex> lock2(m_mutex2);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::lock_guard<std::mutex> lock1(m_mutex1);  // ✗ 等待 lock1，死锁！
      }
    };
    ```

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
- **代码清理**：
  - 删除未使用的函数、变量、头文件包含；在提交前运行 `include-what-you-use`/`clang-tidy` 修剪死代码。
  - 重复逻辑提炼为公共函数或模板，避免“复制粘贴”式维护。

## 代码评审清单

| 检查项           | 关键问题                                 | 典型陷阱                               | 评审记录   |
| ---------------- | ---------------------------------------- | -------------------------------------- | ---------- |
| 设计符合单一职责 | 模块边界是否清晰，依赖是否必要           | 类/模块耦合过紧，隐藏全局状态          | ☑/☐ 说明 |
| 命名与风格一致   | 符合命名约定与 clang-format 规则         | 匈牙利命名、蛇形混用，行宽超限         | ☑/☐ 说明 |
| 现代 C++ 使用    | 智能指针、RAII 是否正确使用              | 裸指针泄漏、`shared_ptr` 环引用      | ☑/☐ 说明 |
| Qt 信号槽正确    | 强类型连接？跨线程连接显式声明？         | 旧式 SIGNAL/SLOT 字符串、槽未解绑      | ☑/☐ 说明 |
| 线程安全         | 跨线程访问是否安全、锁粒度合理           | UI 线程阻塞、死锁、数据竞争            | ☑/☐ 说明 |
| 错误处理完备     | 是否返回/传播错误码，日志上下文齐全      | 静默失败、异常泄露到 Qt 事件循环       | ☑/☐ 说明 |
| 安全与隐私       | 是否存在敏感信息泄露、权限漏洞           | 凭据硬编码、日志泄露密码               | ☑/☐ 说明 |
| 测试覆盖充分     | 单元/集成测试、自动化脚本是否更新        | 漏测关键路径、测试数据过旧             | ☑/☐ 说明 |
| 性能与资源       | 是否评估性能影响，资源释放               | 循环中动态分配、内存增长               | ☑/☐ 说明 |
| 文档与注释       | 接口、协议、配置是否有文档更新           | 忘记更新 README/版本说明               | ☑/☐ 说明 |
| 基础风格守则     | 命名、注释、宏、头文件等是否符合团队规范 | 缺少文件头注释、宏滥用、头文件职责混乱 | ☑/☐ 说明 |

> **提示**：评审记录需提交至代码平台或质量系统，确保问题整改可追踪。

## 附录：华为 C 语言编程规范整理与示例

> 本附录整理自华为 C 语言编程规范精华，结合团队 C++/Qt 实际改写。所有示例均使用现代 C++ 语法，强调“清晰第一、简洁为美、风格一致”。

### A.1 总体原则

- **清晰性优先**：代码先写给人看，再写给机器执行；必要时牺牲少量性能换取可读性。
- **简洁可维护**：删除死代码，提炼重复逻辑；保持函数短小，平均不超过 80 行。
- **风格统一**：继承所在模块既有风格，必要时通过 clang-format & `.editorconfig` 自动化修正。

```cpp
// 推荐：拆分责任清晰的函数
void WorkflowRunner::start() {
  if (!ensurePrerequisite()) {
    LOGE("workflow.start.failed", "missing prerequisite");
    return;
  }
  launchExecution();
}

bool WorkflowRunner::ensurePrerequisite() {
  // ... existing code ...
  return true;
}
```

### A.2 头文件与模块边界

- 头文件仅暴露接口，任何内部实现细节移动到 `.cpp`。
- 遵循“稳定依赖原则”：业务层引用基础层，禁止反向依赖。
- 尽量使用前向声明，降低编译依赖；禁止在头文件引入不必要的大头文件链。

```cpp
// DeviceMonitor.h —— 只含接口声明
#pragma once

class PressureSensor;

class DeviceMonitor final {
public:
  explicit DeviceMonitor(PressureSensor* sensor);
  void poll();

private:
  PressureSensor* m_sensor = nullptr; // 前向声明即可
};

// DeviceMonitor.cpp —— 实现文件再包含具体头
#include "PressureSensor.h"

DeviceMonitor::DeviceMonitor(PressureSensor* sensor) : m_sensor(sensor) {}

void DeviceMonitor::poll() {
  if (!m_sensor) {
    LOGW("device.monitor", "no sensor bound");
    return;
  }
  // ... existing code ...
}
```

### A.3 函数设计

- 单一职责，函数名体现动作；避免深度嵌套，超过 3 级即应拆分。
- 输入参数使用 `const&` 或 `std::span`，输出通过返回值或结构体；禁止通过裸指针隐式返回。
- 严格校验参数合法性，必要时返回错误码或使用 `Expected` 类型。

```cpp
/// @brief 校验并保存压力阈值
Expected<void, QString> PressureService::updateThreshold(double value) {
  constexpr double kMin = 0.0;
  constexpr double kMax = 500.0;

  if (value < kMin || value > kMax) {
    return makeUnexpected(tr("阈值超出范围 [%1, %2]").arg(kMin).arg(kMax));
  }
  m_threshold = value;
  return {};
}
```

### A.4 命名与类型定义

- 变量命名清晰表达含义，禁止 `tmp`, `data1` 等模糊词。
- 结构体、枚举命名以领域名开头：`RecipeStep`, `UserRole`；枚举值统一大写。
- `typedef` 更倾向 `using`；多层 `using` 统一收敛至一个头文件集中维护。

```cpp
enum class RecipeStepType { Load, Measure, Verify, Unload };

using Timestamp = std::chrono::system_clock::time_point;

struct RecipeStep {
  RecipeStepType type;
  QString description;
  Timestamp scheduledAt;
};
```

### A.5 变量、常量与宏

- 变量定义就近，初始化即赋值；禁止先声明后赋值导致未定义行为。
- 常量使用 `constexpr` 或 `const`; 尽量使用具名常量替代魔法数字。
- 宏仅用于编译开关或条件编译；宏体必须加括号并解释目的。

```cpp
constexpr std::chrono::milliseconds kSensorPollInterval{500};

for (int retry = 0; retry < kMaxRetryCount; ++retry) {
  if (tryConnect()) {
    break;
  }
  QThread::msleep(200);
}

#ifdef ENABLE_SENSOR_SIMULATION
  LOGI("sensor.sim", "running in simulation mode");
#endif
```

### A.6 表达式与控制流

- 避免复杂表达式；对关键逻辑拆分临时变量。
- 条件表达式左值使用常量在前形式 `if (0 == errorCode)`，防止误写 `=`。
- `switch` 必须列出默认分支处理；使用 `[[fallthrough]]` 明确贯穿。

```cpp
switch (mode) {
case OperationMode::Manual:
  return handleManual(config);
case OperationMode::Automatic:
  return handleAutomatic(config);
default:
  LOGE("workflow.mode.invalid", static_cast<int>(mode));
  return makeUnexpected(QStringLiteral("未知工作模式"));
}
```

### A.7 注释与文档化

- 文件头注释包含用途、作者、版权、历史版本；模块内维护 `CHANGELOG`。
- 对外接口使用 Doxygen，参数、返回值、异常情况逐条说明。
- 对复杂状态机加入时序图或表格说明；注释需随代码更新。

```cpp
/**
 * @file TemperatureController.h
 * @brief 控制温区温度的公共接口，支持 PSI5 传感器
 * @author Tester Team
 * @date 2025-09-27
 */

/// @brief 进入指定温区并等待稳定
/// @param zone 目标温区
/// @param timeout 等待超时 (秒)
/// @return true 表示稳定，false 表示超时
bool enterZone(TemperatureZone zone, std::chrono::seconds timeout);
```

### A.8 排版与格式

- 缩进统一 2 或 4 空格；花括号自立一行或与语句同行保持一致性。
- `if/else`、`for` 等语句无论执行体是否单行都必须加花括号。
- 保持逻辑块之间适当空行，分组相关语句。

```cpp
for (const auto& task : m_tasks) {
  if (!task->isEnabled()) {
    continue;
  }

  task->prepare();
  task->execute();
}
```

### A.9 编辑、编译与测试要求

- 所有提交必须通过 clang-format、clang-tidy、静态检查；禁止警告未清除。
- 编译开关按平台分类维护，如 `config/compiler/msvc.cmake`、`config/compiler/gcc.cmake`。
- 单元测试覆盖率写入 CI，核心模块不低于 70%。

```bash
# 推荐：提交前检查脚本
python tools/precommit/check_format.py
ctest --output-on-failure

# MSVC: 启用 /W4 /WX
# GCC/Clang: 启用 -Wall -Wextra -Werror -Wpedantic
```

### A.10 常见反例速查

- **反例 1：头文件实现** —— 在 `.h` 编写完整函数导致重复编译。
- **反例 2：宏副作用** —— `#define MAX(a,b) a > b ? a : b` 遇到自增表达式出错。
- **反例 3：隐式类型转换** —— 混用窄宽整数字段，未显式 `static_cast`。
- **反例 4：注释过期** —— 修改代码忘记同步注释，评审时必须指出。

```cpp
// 反例：宏副作用
#define INC_AND_GET(x) (++(x))
int value = 0;
int sum = INC_AND_GET(value) + INC_AND_GET(value); // 结果不可预期

// 正例：使用函数
inline int incAndGet(int& value) {
  return ++value;
}
```

## 变更记录

| 版本 | 日期       | 负责人      | 变更内容                                                          |
| ---- | ---------- | ----------- | ----------------------------------------------------------------- |
| v1.2 | 2025-09-27 | Winnie 团队 | 新增华为 C 语言规范附录与示例，扩充评审参考                       |
| v1.1 | 2025-09-27 | Winnie 团队 | 补充华为 C 语言规范抽取的基础风格条款，细化命名/注释/头文件等要求 |
| v1.0 | 2025-09-26 | Winnie 团队 | 初版，覆盖 C++/Qt 编码、评审要点                                  |
