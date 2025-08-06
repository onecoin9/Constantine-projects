#include "LogDatabase.h"

LogDatabase& LogDatabase::instance()
{
    static LogDatabase instance;
    return instance;
}

LogDatabase::LogDatabase(QObject* parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

LogDatabase::~LogDatabase()
{
    if (m_db.isOpen())
        m_db.close();
}

bool LogDatabase::initialize(const QString& dbPath)
{
    m_db.setDatabaseName(dbPath);
    
    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    return createTables();
}

bool LogDatabase::createTables()
{
    QSqlQuery query;
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS logs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "event TEXT NOT NULL,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "level TEXT NOT NULL,"
        "message TEXT NOT NULL,"
        "detail TEXT,"
        "data TEXT"
        ")"
    );

    if (!success) {
        m_lastError = query.lastError().text();
    }
    return success;
}

bool LogDatabase::insertLog(const QString& event, 
                          const QString& level, 
                          const QString& message,
                          const QString& detail,
                          const QString& data)
{
    QSqlQuery query;
    query.prepare(
        "INSERT INTO logs (event, level, message, detail, data) "
        "VALUES (:event, :level, :message, :detail, :data)"
    );
    
    query.bindValue(":event", event);
    query.bindValue(":level", level);
    query.bindValue(":message", message);
    query.bindValue(":detail", detail);
    query.bindValue(":data", data);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

QSqlQuery LogDatabase::queryLogs(const QString& filter)
{
    QSqlQuery query;
    QString queryStr = "SELECT * FROM logs";
    
    if (!filter.isEmpty()) {
        queryStr += " WHERE " + filter;
    }
    queryStr += " ORDER BY timestamp DESC";

    if (!query.exec(queryStr)) {
        m_lastError = query.lastError().text();
    }
    return query;
}

bool LogDatabase::clearLogs()
{
    QSqlQuery query;
    if (!query.exec("DELETE FROM logs")) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

QString LogDatabase::lastError() const
{
    return m_lastError;
}

void acroViewTester::setupDataUI()
{
    // 初始化数据库
    if (!LogDatabase::instance().initialize()) {
        QMessageBox::critical(this, "Error", 
            "Failed to initialize database: " + LogDatabase::instance().lastError());
        return;
    }

    // 加载已有日志
    loadLogsFromDatabase();
}

void acroViewTester::loadLogsFromDatabase()
{
    dataModel->removeRows(0, dataModel->rowCount());
    
    QSqlQuery query = LogDatabase::instance().queryLogs();
    while (query.next()) {
        QList<QStandardItem*> row;
        row << new QStandardItem(query.value("id").toString())
            << new QStandardItem(query.value("event").toString())
            << new QStandardItem(query.value("timestamp").toDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            << new QStandardItem(query.value("level").toString())
            << new QStandardItem(query.value("message").toString())
            << new QStandardItem(query.value("detail").toString())
            << new QStandardItem(query.value("data").toString());

        for (auto* item : row) {
            item->setTextAlignment(Qt::AlignCenter);
        }
        dataModel->appendRow(row);
    }
}

void acroViewTester::addLog(const QString& event, 
                          const QString& level, 
                          const QString& message,
                          const QString& detail,
                          const QString& data)
{
    if (LogDatabase::instance().insertLog(event, level, message, detail, data)) {
        loadLogsFromDatabase();  // 重新加载显示
    } else {
        QMessageBox::warning(this, "Warning", 
            "Failed to insert log: " + LogDatabase::instance().lastError());
    }
}