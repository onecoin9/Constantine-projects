#include "logManager.h"
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/file_helper.h>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QCoreApplication>
#include <csignal>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardItem>

// 替换 std::filesystem 的 C++11 兼容实现
namespace spdlog
{
    class custom_rotating_file_sink : public spdlog::sinks::base_sink<std::mutex>
    {
    public:
        custom_rotating_file_sink(const std::string& base_filename,
            std::size_t max_size,
            std::size_t max_files)
            : base_filename_(base_filename),
            max_size_(max_size),
            max_files_(max_files)
        {
            file_helper_.open(gen_filename());
        }

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override
        {
            spdlog::memory_buf_t formatted;
            formatter_->format(msg, formatted);

            if (file_helper_.size() + formatted.size() > max_size_)
            {
                rotate_();
            }

            file_helper_.write(formatted);
        }

        void flush_() override
        {
            file_helper_.flush();
        }

    private:
        std::string gen_filename()
        {
            QDateTime now = QDateTime::currentDateTime();
            QString timeStr = now.toString("yyyyMMddhhmmss");

            // 添加毫秒部分(3位)
            int ms = now.time().msec();
            timeStr += QString("_%1").arg(ms, 3, 10, QLatin1Char('0'));

            return base_filename_ + "_" + timeStr.toStdString() + ".log";
        }

        void rotate_()
        {
            file_helper_.close();
            cleanup_old_files();
            file_helper_.open(gen_filename());
        }

        void cleanup_old_files()
        {
            if (max_files_ == 0) return;

            QFileInfo base_info(QString::fromStdString(base_filename_));
            QDir dir = base_info.absoluteDir();
            QString base_name = base_info.fileName();

            QFileInfoList files = dir.entryInfoList(QStringList() << (base_name + "_*.log"),
                QDir::Files, QDir::Time);

            // 删除最旧的文件
            while (files.size() >= static_cast<int>(max_files_))
            {
                QFile::remove(files.last().absoluteFilePath());
                files.removeLast();
            }
        }

        std::string base_filename_;
        std::size_t max_size_;
        std::size_t max_files_;
        spdlog::details::file_helper file_helper_;
    };

} // namespace

LogManager::LogManager(QObject* parent) : QObject(parent)
{
    // 连接Qt退出信号
    connect(qApp, &QCoreApplication::aboutToQuit, this, &LogManager::onAboutToQuit);
    connect(this, &LogManager::logAdded, this, &LogManager::onLogAdded);


    // 处理异常信号
    static auto handleSignal = [](int)
    {
        LogManager::instance().shutdown();
        std::_Exit(1);
    };

    std::signal(SIGTERM, handleSignal);
    std::signal(SIGSEGV, handleSignal);
    std::signal(SIGINT, handleSignal);
    std::signal(SIGABRT, handleSignal);

    // 初始化表格模型
    m_logModel = new QStandardItemModel(this);
    m_logModel->setColumnCount(7); // 根据您的UI设计调整列数
    m_logModel->setHeaderData(0, Qt::Horizontal, tr("日志序号"));
    m_logModel->setHeaderData(1, Qt::Horizontal, tr("日志名称")); // 假设日志名称是message的一部分或detail
    m_logModel->setHeaderData(2, Qt::Horizontal, tr("日志发生时间"));
    m_logModel->setHeaderData(3, Qt::Horizontal, tr("日志类型"));
    m_logModel->setHeaderData(4, Qt::Horizontal, tr("日志内容"));
    m_logModel->setHeaderData(5, Qt::Horizontal, tr("告警详情"));
    m_logModel->setHeaderData(6, Qt::Horizontal, tr("告警数据"));
}

LogManager::~LogManager()
{
    shutdown();
}

LogManager& LogManager::instance()
{
    static LogManager logInstance; // 修改变量名以避免与类名冲突
    return logInstance;
}

void LogManager::initialize(const QString& logDir, const QString& appName, const QString& dbPath, size_t maxFileSize, size_t maxFiles)
{
    // 文件日志部分的初始化
    if (!m_logger) { // 假设 m_logger 代表文件日志部分已初始化
        QDir().mkpath(logDir);
        std::string base_filename = QDir(logDir).absoluteFilePath(appName).toStdString();
        m_logger = createCustomLogger(base_filename, maxFileSize, maxFiles);
        m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] %v");
        m_logger->set_level(spdlog::level::trace);
        spdlog::register_logger(m_logger);
        spdlog::set_default_logger(m_logger);
    }

    // 初始化数据库 (如果尚未初始化)
    if (!isDBInitialized()) {
        if (!initializeDB(dbPath)) {
            qCritical() << "Failed to initialize database for logging during main initialization.";
        }
    }
    
    // 初始化后刷新一次日志视图
    refreshLogView();
}

void LogManager::shutdown()
{
    if (m_shuttingDown.exchange(true)) return; // 使用 exchange 确保原子性并获取旧值

    emit aboutToShutdown();

    try
    {
        if (m_logger)
        {
            m_logger->flush();
            spdlog::drop(m_logger->name());
            m_logger.reset(); // 显式重置
        }
        spdlog::shutdown(); // spdlog 全局关闭
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        qCritical() << "Log shutdown error:" << ex.what();
    }
    catch (...)
    {
        qCritical() << "Unknown error during log shutdown.";
    }

    if (m_db.isOpen()) {
        m_db.close();
    }
}

void LogManager::onAboutToQuit()
{
    shutdown();
}

std::shared_ptr<spdlog::logger> LogManager::createCustomLogger(const std::string& base_filename,
    size_t max_size,
    size_t max_files)
{
    auto sink = std::make_shared<spdlog::custom_rotating_file_sink>(base_filename, max_size, max_files);
    auto logger = std::make_shared<spdlog::logger>("qt_logger", sink); // 给logger一个名字
    return logger;
}

bool LogManager::initializeDB(const QString& dbPath)
{
    if (m_dbInitialized.load()) return true;

    QFileInfo dbFileInfo(dbPath);
    QDir dir;
    // 确保数据库文件所在的目录存在
    if (!dir.mkpath(dbFileInfo.absolutePath())) {
        qCritical() << "Failed to create directory for database:" << dbFileInfo.absolutePath();
        // 可以选择返回 false 或采取其他错误处理措施
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", "LogConnection");
    m_db.setDatabaseName(dbPath); // 现在目录应该存在了

    if (!m_db.open()) {
        qCritical() << "Failed to open log database:" << m_db.lastError().text() << "Path:" << dbPath;
        return false;
    }
    // 打印数据库文件的绝对路径
    qInfo() << "Log database opened successfully. Absolute path:" << QFileInfo(m_db.databaseName()).absoluteFilePath();
    if (createTables()) {
        m_dbInitialized.store(true);
        return true;
    }
    return false;
}

bool LogManager::createTables()
{
    QSqlQuery query(m_db);
    QString createTableSql =
        "CREATE TABLE IF NOT EXISTS log_entries ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "event_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "log_level TEXT NOT NULL,"
        "message TEXT,"
        "detail TEXT,"
        "data TEXT"
        ")";

    if (!query.exec(createTableSql)) {
        qCritical() << "Failed to create log_entries table:" << query.lastError().text();
        return false;
    }
    qInfo() << "log_entries table checked/created successfully.";
    return true;
}

void LogManager::addLogToDB(LogLevel level, const QString& message, const QString& detail, const QString& data)
{
    if (!m_db.isOpen()) {
        qWarning() << "Database not open. Cannot add log to DB.";
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO log_entries (log_level, message, detail, data, event_time) "
                  "VALUES (:log_level, :message, :detail, :data, :event_time)");

    query.bindValue(":log_level", logLevelToString(level));
    query.bindValue(":message", message);
    query.bindValue(":detail", detail);
    query.bindValue(":data", data);
    query.bindValue(":event_time", QDateTime::currentDateTime().toString(Qt::ISODateWithMs)); // 使用ISO格式并包含毫秒


    if (!query.exec()) {
        qCritical() << "Failed to add log entry to DB:" << query.lastError().text();
    } else {
        emit logAdded(); // 成功添加后发出信号
    }
}

void LogManager::refreshLogView()
{
    if (!m_logModel) return;

    m_logModel->removeRows(0, m_logModel->rowCount()); // 清空模型

    if (!m_db.isOpen()) {
        qWarning() << "Database not open. Cannot refresh log view.";
        return;
    }

    QSqlQuery query("SELECT id, event_time, log_level, message, detail, data FROM log_entries ORDER BY id ASC", m_db); // 按ID升序或时间升序
    while (query.next()) {
        QList<QStandardItem*> rowItems;
        rowItems.append(new QStandardItem(query.value("id").toString()));
        // 假设“日志名称”是message的一部分，或者你需要从其他地方获取
        // 为了演示，我们暂时将message也作为日志名称
        rowItems.append(new QStandardItem(query.value("message").toString())); 
        rowItems.append(new QStandardItem(QDateTime::fromString(query.value("event_time").toString(), Qt::ISODateWithMs).toString("yyyy-MM-dd hh:mm:ss.zzz")));
        rowItems.append(new QStandardItem(query.value("log_level").toString()));
        rowItems.append(new QStandardItem(query.value("message").toString()));
        rowItems.append(new QStandardItem(query.value("detail").toString()));
        rowItems.append(new QStandardItem(query.value("data").toString()));
        
        for(QStandardItem* item : rowItems) {
            if(item) item->setTextAlignment(Qt::AlignCenter);
        }
        m_logModel->appendRow(rowItems);
    }
    emit logDataChanged();
}

QString LogManager::logLevelToString(LogLevel level) {
    switch (level) {
        case Trace: return "Trace";
        case Debug: return "Debug";
        case Info:  return "Info";
        case Warn:  return "Warn";
        case Error: return "Error";
        case Critical: return "Critical";
        default: return "Unknown";
    }
}

void LogManager::onLogAdded()
{
    // 当数据库中有新日志添加时，刷新UI视图
    // 注意：如果日志非常频繁，直接刷新可能会影响性能
    // 可以考虑使用QTimer延迟刷新，或者只刷新最后几条
    refreshLogView();
}