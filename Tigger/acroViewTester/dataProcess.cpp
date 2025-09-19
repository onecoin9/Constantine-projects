#include "acroViewTester.h"
#include "LogDatabase.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QStandardItemModel>

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