#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

class LogDatabase : public QObject
{
    Q_OBJECT

public:
    static LogDatabase& instance();

    bool initialize(const QString& dbPath = "logs.db");
    bool insertLog(const QString& event, 
                  const QString& level, 
                  const QString& message,
                  const QString& detail = QString(),
                  const QString& data = QString());
    QSqlQuery queryLogs(const QString& filter = QString());
    bool clearLogs();
    QString lastError() const;

private:
    LogDatabase(QObject* parent = nullptr);
    ~LogDatabase();
    LogDatabase(const LogDatabase&) = delete;
    LogDatabase& operator=(const LogDatabase&) = delete;

    bool createTables();
    
private:
    QSqlDatabase m_db;
    QString m_lastError;
};