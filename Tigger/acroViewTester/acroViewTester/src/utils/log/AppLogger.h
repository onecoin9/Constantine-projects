#ifndef APPLOGGER_H
#define APPLOGGER_H

#include <QString>
#include "logManager.h"

class AppLogger 
{
public:
    static AppLogger& instance() {
        static AppLogger instance;
        return instance;
    }
    
    void info(const QString& eventName, 
              const QString& detail = "",
              const QString& jsonData = "{}") {
        LogManager::instance().addLogToDB(
            LogManager::Info, eventName, detail, jsonData);
    }
    
    void warn(const QString& eventName,
              const QString& detail = "",
              const QString& jsonData = "{}") {
        LogManager::instance().addLogToDB(
            LogManager::Warn, eventName, detail, jsonData);
    }
    
    void error(const QString& eventName,
               const QString& detail = "", 
               const QString& jsonData = "{}") {
        LogManager::instance().addLogToDB(
            LogManager::Error, eventName, detail, jsonData);
    }
    
    void debug(const QString& eventName,
               const QString& detail = "",
               const QString& jsonData = "{}") {
        LogManager::instance().addLogToDB(
            LogManager::Debug, eventName, detail, jsonData);
    }
    
    // 便捷的JSON格式化方法
    static QString json(const QString& key, const QString& value) {
        return QString("{\"%1\":\"%2\"}").arg(key).arg(value);
    }
    
    static QString json(const QString& key, int value) {
        return QString("{\"%1\":%2}").arg(key).arg(value);
    }
    
private:
    AppLogger() {} // 私有构造函数
    AppLogger(const AppLogger&) = delete;
    AppLogger& operator=(const AppLogger&) = delete;
};

// 定义全局别名函数，使代码更简洁
#define LOG AppLogger::instance()

#endif // APPLOGGER_H