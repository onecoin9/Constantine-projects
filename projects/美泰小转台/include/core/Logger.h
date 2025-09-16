/*
 * Logger.h - 统一日志系统
 * 
 * 这个日志系统基于spdlog库构建，提供了统一的日志记录接口
 * 支持多种输出方式：控制台、文件、Qt调试输出、UI显示
 * 
 * 主要特性：
 * 1. 单例模式，全局唯一的Logger实例
 * 2. 支持多模块独立日志记录
 * 3. 异步日志处理，不阻塞主线程
 * 4. 线程安全的日志输出
 * 5. 灵活的日志级别控制
 * 6. 支持自定义日志输出目标（Sink）
 * 
 * 使用方法：
 * 1. 基本日志记录：
 *    LOG_INFO("这是一条信息");
 *    LOG_ERROR("这是一条错误");
 * 
 * 2. 模块化日志记录：
 *    LOG_MODULE_INFO("DeviceManager", "设备初始化完成");
 *    LOG_MODULE_ERROR("TcpChannel", "连接失败");
 * 
 * 3. 特殊用途日志：
 *    LOG_SUCCESS("操作成功");
 *    LOG_EVENT("事件发生");
 *    LOG_CONFIG("配置更新");
 * 
 * 4. 高级用法：
 *    // 获取特定模块的logger
 *    auto logger = Logger::getInstance().getLogger("MyModule");
 *    logger->info("模块日志");
 * 
 *    // 设置模块日志级别
 *    Logger::getInstance().setModuleLevel("MyModule", LogLevel::Debug);
 * 
 * 注意事项：
 * - 必须在使用前调用initialize()方法
 * - 程序退出前调用shutdown()方法
 * - 日志消息会自动添加时间戳和级别标识
 * - UI显示的日志通过信号槽机制实现，确保线程安全
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <map>
#include <mutex>
#include <QSet>
#include <QDateTime>
#include <QMap>
#include <QMutex>
#include <QQueue> // For thread-safe message queueing
#include <atomic> // For atomic bool
#include <QObject> // <--- 新增

namespace TesterFramework {

/**
 * @brief 日志级别枚举
 * 
 * 日志级别从低到高：
 * - Trace: 最详细的调试信息
 * - Debug: 调试信息
 * - Info: 一般信息
 * - Warning: 警告信息
 * - Error: 错误信息
 * - Critical: 严重错误
 * - Off: 关闭日志
 */
enum class LogLevel {
    Trace = spdlog::level::trace,
    Debug = spdlog::level::debug,
    Info = spdlog::level::info,
    Warning = spdlog::level::warn,
    Error = spdlog::level::err,
    Critical = spdlog::level::critical,
    Off = spdlog::level::off
};



/**
 * @brief 日志接口类
 * 
 * 定义了日志系统的基本接口，所有日志记录器都应该实现这个接口
 */
class ILogger {
public:
    virtual ~ILogger() = default;
    
    /**
     * @brief 记录指定级别的日志
     * @param level 日志级别
     * @param message 日志消息
     */
    virtual void log(LogLevel level, const std::string& message) = 0;
    
    // 便利方法 - 各种日志级别的快捷方法
    virtual void trace(const std::string& message) { log(LogLevel::Trace, message); }
    virtual void debug(const std::string& message) { log(LogLevel::Debug, message); }
    virtual void info(const std::string& message) { log(LogLevel::Info, message); }
    virtual void warning(const std::string& message) { log(LogLevel::Warning, message); }
    virtual void error(const std::string& message) { log(LogLevel::Error, message); }
    virtual void critical(const std::string& message) { log(LogLevel::Critical, message); }
    
    /**
     * @brief 设置日志级别
     * @param level 要设置的日志级别
     */
    virtual void setLevel(LogLevel level) = 0;
    
    /**
     * @brief 刷新日志缓冲区
     */
    virtual void flush() = 0;
};



/**
 * @brief 日志管理器（单例模式）
 * 
 * 继承QObject以使用Qt的信号槽机制，确保UI更新在主线程中进行
 * 
 * 主要功能：
 * 1. 管理全局日志配置
 * 2. 创建和管理模块化日志记录器
 * 3. 处理日志输出到不同的目标（控制台、文件、UI等）
 * 4. 提供线程安全的日志记录
 * 
 * 使用示例：
 * ```cpp
 * // 初始化日志系统
 * Logger::getInstance().initialize("app.log");
 * 
 * // 设置全局日志级别
 * Logger::getInstance().setLevel(LogLevel::Info);
 * 
 * // 为特定模块设置日志级别
 * Logger::getInstance().setModuleLevel("NetworkModule", LogLevel::Debug);
 * 
 * // 注册UI日志接收器
 * Logger::getInstance().addSink([](LogLevel level, const std::string& msg) {
 *     // 处理日志消息，如显示在UI中
 * });
 * ```
 */
class Logger : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 获取Logger单例实例
     * @return Logger的唯一实例
     */
    static Logger& getInstance();

    // 禁用拷贝构造和赋值操作
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief 初始化日志系统
     * @param logFileName 日志文件名，文件将保存在logs目录下
     * 
     * 此方法会：
     * 1. 创建日志目录
     * 2. 初始化spdlog线程池
     * 3. 创建各种日志输出目标（文件、控制台、UI等）
     * 4. 设置默认的日志格式
     */
    void initialize(const std::string& logFileName);
    
    /**
     * @brief 关闭日志系统
     * 
     * 清理资源，刷新所有缓冲区
     */
    void shutdown();
    
    /**
     * @brief 设置日志输出格式
     * @param pattern spdlog格式字符串
     * 
     * 格式说明：
     * %H:%M:%S.%f - 时间戳
     * %l - 日志级别
     * %v - 日志消息
     * %^和%$ - 颜色控制
     */
    void setPattern(const std::string& pattern);
    
    /**
     * @brief UI日志接收器回调函数类型
     * @param level 日志级别
     * @param message 日志消息
     */
    using SinkCallback = std::function<void(LogLevel, const std::string&)>;
    
    /**
     * @brief 添加日志接收器
     * @param sink 日志接收器回调函数
     * @return 接收器的唯一标识符，用于后续删除
     * 
     * 用于将日志输出到自定义目标，如UI显示
     */
    size_t addSink(SinkCallback sink);
    
    /**
     * @brief 移除日志接收器
     * @param sinkId 接收器的唯一标识符（由addSink返回）
     * 
     * 用于在对象销毁时取消注册接收器，防止访问已销毁的对象
     */
    void removeSink(size_t sinkId);
    
    /**
     * @brief 设置全局日志级别
     * @param level 要设置的日志级别
     */
    void setLevel(LogLevel level);
    
    /**
     * @brief 设置指定模块的日志级别
     * @param moduleName 模块名称
     * @param level 要设置的日志级别
     */
    void setModuleLevel(const std::string& moduleName, LogLevel level);
    
    /**
     * @brief 获取spdlog logger实例
     * @param moduleName 模块名称，默认为"default"
     * @return spdlog logger的共享指针
     * 
     * 如果模块logger不存在，会自动创建
     */
    std::shared_ptr<spdlog::logger> getLogger(const std::string& moduleName = "default");

    /**
     * @brief 获取所有已注册的logger名称
     * @return logger名称列表
     */
    std::vector<std::string> getRegisteredLoggerNames();
    
    /**
     * @brief 获取指定模块的日志级别
     * @param moduleName 模块名称
     * @return 模块的日志级别
     */
    LogLevel getModuleLevel(const std::string& moduleName);

private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    Logger();
    
    /**
     * @brief 析构函数
     */
    ~Logger();
    
    // 内部类声明
    class CallbackSink;  // 用于将日志转发到UI回调
    class QtDebugSink;   // 用于将日志输出到Qt调试系统
    
    // 声明为友元类，允许访问私有成员
    friend class CallbackSink;
    friend class QtDebugSink;

    /**
     * @brief 分发日志消息到所有已注册的接收器
     * @param level 日志级别
     * @param msg 日志消息
     */
    void dispatchToSinks(LogLevel level, const std::string& msg);

    // 成员变量
    std::atomic<bool> m_initialized;  // 初始化状态（原子变量，线程安全）
    std::shared_ptr<spdlog::logger> m_logger;  // 主要的spdlog logger
    std::vector<std::shared_ptr<spdlog::sinks::sink>> m_shared_sinks;  // 共享的sink列表

    // UI接收器相关（线程安全）
    std::mutex m_sink_mutex;  // 保护sink列表的互斥锁
    std::map<size_t, SinkCallback> m_sinks;  // UI接收器映射表，使用ID作为键
    size_t m_nextSinkId;  // 下一个sink的ID

    // Logger创建的线程安全
    std::mutex m_logger_mutex;  // 保护logger创建的互斥锁

    // 早期日志缓冲（在UI接收器注册前暂存日志）
    std::vector<std::pair<LogLevel, std::string>> m_earlyLogBuffer;

private slots:
    /**
     * @brief 处理Qt调试输出的槽函数
     * @param msg 要输出的消息
     * 
     * 确保qDebug调用在主线程中执行
     */
    void onLogToQtDebug(QString msg);

signals:
    /**
     * @brief 请求在主线程中输出Qt调试信息的信号
     * @param msg 要输出的消息
     */
    void logToQtDebug(QString msg);
};



/**
 * @brief 便利宏定义 - 基础日志记录
 * 
 * 这些宏提供了简单的日志记录接口，使用默认的logger
 * 
 * 使用示例：
 * LOG_INFO("系统启动完成");
 * LOG_ERROR("文件打开失败");
 * LOG_DEBUG("变量值: {}", value);
 */
#define LOG_TRACE(msg) TesterFramework::Logger::getInstance().getLogger()->trace(msg)
#define LOG_DEBUG(msg) TesterFramework::Logger::getInstance().getLogger()->debug(msg)
#define LOG_INFO(msg) TesterFramework::Logger::getInstance().getLogger()->info(msg)
#define LOG_WARNING(msg) TesterFramework::Logger::getInstance().getLogger()->warn(msg)
#define LOG_ERROR(msg) TesterFramework::Logger::getInstance().getLogger()->error(msg)
#define LOG_CRITICAL(msg) TesterFramework::Logger::getInstance().getLogger()->critical(msg)

/**
 * @brief 便利宏定义 - 特殊用途日志
 * 
 * 这些宏使用特定的logger名称，便于分类和过滤
 * 
 * 使用示例：
 * LOG_SUCCESS("操作成功完成");
 * LOG_EVENT("用户点击了按钮");
 * LOG_CONFIG("配置文件已更新");
 */
#define LOG_SUCCESS(msg) TesterFramework::Logger::getInstance().getLogger("SUCCESS")->info(msg)
#define LOG_EVENT(msg) TesterFramework::Logger::getInstance().getLogger("EVENT")->info(msg)
#define LOG_RAW(msg) TesterFramework::Logger::getInstance().getLogger("RAW")->info(msg)
#define LOG_JSON(msg) TesterFramework::Logger::getInstance().getLogger("JSON")->info(msg)
#define LOG_STATUS(msg) TesterFramework::Logger::getInstance().getLogger("STATUS")->info(msg)
#define LOG_CONFIG(msg) TesterFramework::Logger::getInstance().getLogger("CONFIG")->info(msg)

/**
 * @brief 便利宏定义 - 模块化日志记录
 * 
 * 这些宏支持指定模块名称，便于按模块过滤日志
 * 
 * 使用示例：
 * LOG_MODULE_INFO("DeviceManager", "设备初始化完成");
 * LOG_MODULE_ERROR("NetworkModule", "连接失败: {}", errorMsg);
 * LOG_MODULE_DEBUG("ConfigLoader", "加载配置文件: {}", filename);
 * 
 * 注意：
 * - 第一个参数是模块名称（字符串）
 * - 第二个参数开始是日志消息和参数（支持fmt格式化）
 */
#define LOG_MODULE_TRACE(module, ...)   TesterFramework::Logger::getInstance().getLogger(module)->trace(__VA_ARGS__)
#define LOG_MODULE_DEBUG(module, ...)   TesterFramework::Logger::getInstance().getLogger(module)->debug(__VA_ARGS__)
#define LOG_MODULE_INFO(module, ...)    TesterFramework::Logger::getInstance().getLogger(module)->info(__VA_ARGS__)
#define LOG_MODULE_WARNING(module, ...) TesterFramework::Logger::getInstance().getLogger(module)->warn(__VA_ARGS__)
#define LOG_MODULE_ERROR(module, ...)   TesterFramework::Logger::getInstance().getLogger(module)->error(__VA_ARGS__)
#define LOG_MODULE_CRITICAL(module, ...) TesterFramework::Logger::getInstance().getLogger(module)->critical(__VA_ARGS__)




} // namespace TesterFramework

#endif // LOGGER_H 