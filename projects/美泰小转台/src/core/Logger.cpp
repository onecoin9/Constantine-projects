/*
 * Logger.cpp - 统一日志系统实现
 * 
 * 这个文件实现了Logger类的所有功能，包括：
 * 1. 单例模式的Logger管理
 * 2. 多种日志输出目标（文件、控制台、Qt调试、UI）
 * 3. 线程安全的日志处理
 * 4. 模块化日志管理
 * 5. 异步日志处理
 */

#include "core/Logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/fmt/fmt.h>
#include <iostream>
#include <mutex>
#include <QDebug>
#include <QMutexLocker>
#include <QMutex>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif


namespace TesterFramework {

/**
 * @brief 自定义Sink类 - 用于将日志分发到UI等自定义目标
 * 
 * 这个Sink继承自spdlog的base_sink，用于拦截日志消息并转发到
 * 注册的回调函数中，主要用于UI显示
 */
class Logger::CallbackSink : public spdlog::sinks::base_sink<std::mutex> {
public:
    explicit CallbackSink(Logger* outer) : m_outer(outer) {}
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override {}
private:
    Logger* m_outer;
};

/**
 * @brief 自定义Sink类 - 用于将日志输出到Qt的调试系统
 * 
 * 这个Sink将日志消息转发到Qt的qDebug系统，确保在主线程中调用
 */
class Logger::QtDebugSink : public spdlog::sinks::base_sink<std::mutex> {
public:
    explicit QtDebugSink(Logger* outer) : m_outer(outer) {}
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override {}
private:
    Logger* m_outer;
};


/* --- 单例实现 --- */
/**
 * @brief 获取Logger的唯一实例
 * 
 * 使用局部静态变量实现线程安全的单例模式
 * C++11保证静态变量的初始化是线程安全的
 */
Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

/* --- 构造/析构 --- */
/**
 * @brief 构造函数
 * 
 * 初始化Logger的基本状态，不依赖QApplication
 * 连接信号槽确保Qt调试输出在正确的线程中执行
 */
Logger::Logger() : m_initialized(false), m_nextSinkId(1)
{
    // 构造函数现在为空，不依赖QApplication
    // 关键修复：连接信号和槽，确保qDebug在正确的线程中被调用
    connect(this, &Logger::logToQtDebug, this, &Logger::onLogToQtDebug, Qt::QueuedConnection);
}

/**
 * @brief 析构函数
 * 
 * 确保日志系统正确关闭，释放所有资源
 */
Logger::~Logger()
{
    if (m_initialized.load()) {
        shutdown();
    }
}

/* --- 初始化方法 --- */
/**
 * @brief 初始化日志系统
 * @param logFileName 日志文件名
 * 
 * 这个方法执行以下操作：
 * 1. 创建日志目录
 * 2. 初始化spdlog的异步线程池
 * 3. 创建各种日志输出目标（Sink）
 * 4. 设置日志格式和刷新策略
 * 5. 注册主日志器
 * 
 * 注意：此方法使用原子操作防止重复初始化
 */
void Logger::initialize(const std::string& logFileName)
{
    if (m_initialized.exchange(true)) { // 原子操作，防止重入
        return;
    }
    
    try {
        // 创建logs目录
        std::string logDir = "logs";
        #ifdef _WIN32
        CreateDirectoryA(logDir.c_str(), NULL);
        #else
        mkdir(logDir.c_str(), 0755);
        #endif
        
        // 初始化线程池（缓冲区大小8192，1个后台线程）
        spdlog::init_thread_pool(8192, 1);
        
        // 创建各种日志输出目标（Sink）
        auto qt_debug_sink = std::make_shared<QtDebugSink>(this);  // Qt调试输出
        std::string logFilePath = "logs/" + logFileName;
        auto rotating_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFilePath, 1024 * 1024 * 5, 3);  // 轮转文件输出
        auto callback_sink = std::make_shared<CallbackSink>(this);  // UI回调输出
        // 注意：移除console_sink避免重复输出和中文乱码问题

        // 逐个添加sink到共享列表
        m_shared_sinks.push_back(qt_debug_sink);
        m_shared_sinks.push_back(rotating_file_sink);
        m_shared_sinks.push_back(callback_sink);

        // 创建异步主日志器
        m_logger = std::make_shared<spdlog::async_logger>("default", m_shared_sinks.begin(), m_shared_sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        
        // 设置日志级别为最详细（Trace）
        m_logger->set_level(spdlog::level::trace);
        spdlog::register_logger(m_logger);
        
        // 设置自动刷新频率，实现"准实时"写入
        spdlog::flush_every(std::chrono::seconds(1));

        // 设置默认模式，包含模块名但不包含时间戳（UI会添加时间戳）
        // %n 是logger名称（模块名），%^%l%$ 是带颜色的日志级别，%v 是消息内容
        spdlog::set_pattern("[%^%l%$] [%n] %v");
        

    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        m_initialized = false;
    }
}

/**
 * @brief 关闭日志系统
 * 
 * 安全地关闭日志系统，刷新所有缓冲区并释放资源
 */
void Logger::shutdown()
{
    if (m_initialized.exchange(false)) {
        spdlog::shutdown();
    }
}

/* --- 设置模式方法 --- */
/**
 * @brief 设置日志输出格式
 * @param pattern spdlog格式字符串
 * 
 * 常用格式符号：
 * %H:%M:%S.%f - 时间戳
 * %l - 日志级别
 * %v - 日志消息
 * %n - logger名称
 * %^ 和 %$ - 颜色控制开始和结束
 */
void Logger::setPattern(const std::string& pattern)
{
    try {
        spdlog::set_pattern(pattern);
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Failed to set log pattern: " << ex.what() << std::endl;
    }
}

/* --- 公共接口实现 --- */
/**
 * @brief 添加日志接收器
 * @param sink 回调函数，用于处理日志消息
 * @return 接收器的唯一标识符，用于后续删除
 * 
 * 这个方法允许外部代码注册自定义的日志处理器，
 * 主要用于UI显示日志。如果有缓冲的早期日志，
 * 会立即发送给新注册的接收器。
 */
size_t Logger::addSink(SinkCallback sink)
{
    std::lock_guard<std::mutex> lock(m_sink_mutex);
    size_t sinkId = m_nextSinkId++;
    m_sinks[sinkId] = sink;
    
    // 如果有缓冲的日志，立即发送给新的sink
    if (!m_earlyLogBuffer.empty()) {
        for (const auto& [level, msg] : m_earlyLogBuffer) {
            sink(level, msg);
        }
    }
    
    return sinkId;
}

/**
 * @brief 移除日志接收器
 * @param sinkId 接收器的唯一标识符（由addSink返回）
 * 
 * 线程安全地移除指定的日志接收器，防止访问已销毁的对象
 */
void Logger::removeSink(size_t sinkId)
{
    std::lock_guard<std::mutex> lock(m_sink_mutex);
    m_sinks.erase(sinkId);
}

/**
 * @brief 设置全局日志级别
 * @param level 要设置的日志级别
 * 
 * 设置主日志器的级别，影响所有未单独设置级别的模块
 */
void Logger::setLevel(LogLevel level)
{
    m_logger->set_level(static_cast<spdlog::level::level_enum>(level));
}

/**
 * @brief 设置特定模块的日志级别
 * @param moduleName 模块名称
 * @param level 要设置的日志级别
 * 
 * 为特定模块设置独立的日志级别，允许细粒度控制
 */
void Logger::setModuleLevel(const std::string& moduleName, LogLevel level)
{
    getLogger(moduleName)->set_level(static_cast<spdlog::level::level_enum>(level));
}

/**
 * @brief 获取或创建指定模块的日志器
 * @param moduleName 模块名称
 * @return spdlog日志器的共享指针
 * 
 * 如果指定的模块日志器不存在，会自动创建一个新的。
 * 新创建的日志器会使用与主日志器相同的所有Sink，
 * 并继承主日志器的级别设置。
 * 
 * 使用示例：
 * auto logger = Logger::getInstance().getLogger("NetworkModule");
 * logger->info("网络连接成功");
 */
std::shared_ptr<spdlog::logger> Logger::getLogger(const std::string& moduleName)
{
    if (!m_initialized) {
        // 这是一个保障，确保在日志系统完全初始化前不会创建模块logger
        return m_logger; 
    }
    std::lock_guard<std::mutex> lock(m_logger_mutex); // 使用互斥锁保护logger创建
    auto logger = spdlog::get(moduleName);
    if (!logger) {
        // 使用共享的sinks列表来创建新的模块logger
        logger = std::make_shared<spdlog::async_logger>(moduleName, m_shared_sinks.begin(), m_shared_sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        logger->set_level(m_logger->level()); // 与主logger级别保持一致
        spdlog::register_logger(logger);
    }
    return logger;
}

/**
 * @brief 获取所有已注册的日志器名称
 * @return 日志器名称列表
 * 
 * 用于UI显示所有可用的模块名称，便于用户选择和过滤
 */
std::vector<std::string> Logger::getRegisteredLoggerNames()
{
    std::vector<std::string> names;
    spdlog::apply_all([&](std::shared_ptr<spdlog::logger> l) {
        names.push_back(l->name());
    });
    return names;
}

/**
 * @brief 获取指定模块的日志级别
 * @param moduleName 模块名称
 * @return 模块的日志级别
 * 
 * 如果模块不存在，返回主日志器的级别
 */
LogLevel Logger::getModuleLevel(const std::string& moduleName)
{
    auto logger = spdlog::get(moduleName);
    return logger ? static_cast<LogLevel>(logger->level()) : static_cast<LogLevel>(m_logger->level());
}

/**
 * @brief 将日志消息分发到所有注册的接收器
 * @param level 日志级别
 * @param msg 日志消息
 * 
 * 这个方法处理日志消息的分发逻辑：
 * 1. 如果没有注册的接收器，将消息暂存到早期缓冲区
 * 2. 如果有注册的接收器，先发送缓冲区中的消息，然后发送当前消息
 * 3. 限制缓冲区大小，防止内存泄漏
 */
void Logger::dispatchToSinks(LogLevel level, const std::string& msg)
{
    std::lock_guard<std::mutex> lock(m_sink_mutex);
    
    if (m_sinks.empty()) {
        // 没有接收器时，暂存到缓冲区
        m_earlyLogBuffer.emplace_back(level, msg);
        // 限制缓冲区大小，避免内存泄漏
        if (m_earlyLogBuffer.size() > 1000) {
            m_earlyLogBuffer.erase(m_earlyLogBuffer.begin());
        }
    } else {
        // 有接收器时，先发送缓冲区中的消息
        if (!m_earlyLogBuffer.empty()) {
            for (const auto& [bufferedLevel, bufferedMsg] : m_earlyLogBuffer) {
                for (const auto& [sinkId, sink] : m_sinks) {
                    sink(bufferedLevel, bufferedMsg);
                }
            }
            m_earlyLogBuffer.clear();
        }
        
        // 发送当前消息
        for (const auto& [sinkId, sink] : m_sinks) {
            sink(level, msg);
        }
    }
}



/* --- 内部类 CallbackSink 实现 --- */
/**
 * @brief CallbackSink的核心处理方法
 * @param msg spdlog的日志消息对象
 * 
 * 这个方法：
 * 1. 格式化日志消息
 * 2. 转换为字符串
 * 3. 调用dispatchToSinks分发到所有注册的回调
 */
void Logger::CallbackSink::sink_it_(const spdlog::details::log_msg& msg)
{
    if (!m_outer) {
        return;
    }

    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);
    
    std::string finalMessage(formatted.data(), formatted.size());
    // 注意：这里不再添加设备上下文，保持消息的原始性

    m_outer->dispatchToSinks(static_cast<LogLevel>(msg.level), finalMessage);
}

/* --- 内部类 QtDebugSink 实现 --- */
/**
 * @brief QtDebugSink的核心处理方法
 * @param msg spdlog的日志消息对象
 * 
 * 这个方法：
 * 1. 格式化日志消息
 * 2. 转换为QString
 * 3. 通过信号发送到主线程的Qt调试系统
 * 
 * 关键点：不直接调用qDebug()，而是通过信号槽确保线程安全
 */
void Logger::QtDebugSink::sink_it_(const spdlog::details::log_msg& msg)
{
    // 关键修复：不再直接调用qDebug()，而是通过信号发送给主线程
    if (!m_outer) return;

    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);
    QString text = QString::fromUtf8(formatted.data(), static_cast<int>(formatted.size()));
    
    emit m_outer->logToQtDebug(text);
}

/**
 * @brief Qt调试输出的槽函数
 * @param msg 要输出的消息
 * 
 * 这个槽函数在主线程中执行，确保qDebug调用的线程安全性
 * 处理换行符，避免双重换行问题
 */
void Logger::onLogToQtDebug(QString msg)
{
    // spdlog的格式化程序会自动在末尾加上换行符，而qDebug也会，这会导致双重换行。
    if (msg.endsWith('\n')) {
        msg.chop(1);
    }
    if (msg.endsWith('\r')) {
        msg.chop(1);
    }
    // .noquote()可以移除qDebug为QString自动添加的引号
    qDebug().noquote() << msg;
}

} // namespace TesterFramework 