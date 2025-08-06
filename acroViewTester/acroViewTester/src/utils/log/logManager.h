#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <memory>
#include <spdlog/spdlog.h>
#include <QSqlDatabase>
#include <QStandardItemModel>

class LogManager : public QObject
{
    Q_OBJECT
public:
    static LogManager& instance();

    // 初始化函数
    void initialize(const QString& logDir = "logs",
        const QString& appName = "app",
        const QString& dbPath = "logs.db",
        size_t maxFileSize = 1024 * 1024,
        size_t maxFiles = 10);
    void shutdown();

    // 日志级别枚举
    enum LogLevel {
        Trace = spdlog::level::trace,
        Debug = spdlog::level::debug,
        Info = spdlog::level::info,
        Warn = spdlog::level::warn,
        Error = spdlog::level::err,
        Critical = spdlog::level::critical
    };

    // 数据库操作
    bool initializeDB(const QString& dbPath);
    void addLogToDB(LogLevel level, const QString& message, 
                   const QString& detail = QString(), 
                   const QString& data = QString());
    
    // UI相关
    QStandardItemModel* getLogModel() { return m_logModel; }
    void refreshLogView();

    // 日志记录函数
    template<typename... Args>
    static void log(spdlog::level::level_enum level, const QString& message, Args... args)
    {
        if (instance().m_logger)
        {
            instance().m_logger->log(level, message.toStdString().c_str(), args...);
            instance().addLogToDB(static_cast<LogLevel>(level), message);
        }
    }

    // 便捷方法
    static void trace(const QString& message) { log(spdlog::level::trace, message); }
    static void debug(const QString& message) { log(spdlog::level::debug, message); }
    static void info(const QString& message) { log(spdlog::level::info, message); }
    static void warn(const QString& message) { log(spdlog::level::warn, message); }
    static void error(const QString& message) { log(spdlog::level::err, message); }
    static void critical(const QString& message) { log(spdlog::level::critical, message); }

    bool isDBInitialized() const { return m_dbInitialized; } // 新增方法

private:
    LogManager(QObject* parent = nullptr);
    ~LogManager();

    std::shared_ptr<spdlog::logger> createCustomLogger(const std::string& base_filename,
        size_t max_size,
        size_t max_files);
    
    bool createTables();
    QString logLevelToString(LogLevel level);

    std::shared_ptr<spdlog::logger> m_logger;
    std::atomic<bool> m_shuttingDown{ false };
    std::atomic<bool> m_dbInitialized{ false }; // 新增状态成员
    QSqlDatabase m_db;
    QStandardItemModel* m_logModel;

signals:
    void aboutToShutdown();
    void logAdded();    // 新增日志时发出信号
    void logDataChanged();
private slots:
    void onAboutToQuit();
    void onLogAdded();  // 处理新增日志的槽函数

};

// 日志宏定义
#define LOG_TRACE(...)    LogManager::log(spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(...)    LogManager::log(spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(...)     LogManager::log(spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(...)     LogManager::log(spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(...)    LogManager::log(spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(...) LogManager::log(spdlog::level::critical, __VA_ARGS__)

#endif // LOGMANAGER_H