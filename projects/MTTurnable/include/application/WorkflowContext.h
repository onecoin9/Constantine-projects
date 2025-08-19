#ifndef WORKFLOWCONTEXT_H
#define WORKFLOWCONTEXT_H

#include <QObject>
#include <QJsonObject>
#include <QVariant>
#include <QMap>
#include <memory>

namespace Services {
    class DeviceManager;
    class WorkflowManager;
    class DutManager;
}

namespace Application {

/**
 * @brief 工作流上下文
 * 在工作流的各个步骤之间传递数据
 */
class WorkflowContext : public QObject
{
    Q_OBJECT
public:
    explicit WorkflowContext(QObject *parent = nullptr);
    ~WorkflowContext();

    // 数据管理
    void setData(const QString &key, const QVariant &value);
    QVariant getData(const QString &key) const;
    bool hasData(const QString &key) const;
    void removeData(const QString &key);
    void clearData();
    
    // JSON数据管理
    void setJsonData(const QString &key, const QJsonObject &value);
    QJsonObject getJsonData(const QString &key) const;
    
    // 获取所有数据
    QMap<QString, QVariant> getAllData() const;
    
    // 设备管理器访问
    void setDeviceManager(std::shared_ptr<Services::DeviceManager> deviceManager);
    std::shared_ptr<Services::DeviceManager> getDeviceManager() const;
    
    // 工作流管理器访问
    void setWorkflowManager(std::shared_ptr<Services::WorkflowManager> workflowManager);
    std::shared_ptr<Services::WorkflowManager> getWorkflowManager() const;
    
    // 工作流信息
    void setWorkflowId(const QString &id);
    QString getWorkflowId() const;
    
    void setWorkflowName(const QString &name);
    QString getWorkflowName() const;
    
    // 当前测试信息
    void setCurrentChipId(const QString &chipId);
    QString getCurrentChipId() const;
    
    void setCurrentSiteId(int siteId);
    int getCurrentSiteId() const;
    
    // 测试结果
    void addTestResult(const QString &testName, const QJsonObject &result);
    QJsonObject getTestResult(const QString &testName) const;
    QJsonObject getAllTestResults() const;
      // 错误信息
    void addError(const QString &step, const QString &error);
    QStringList getErrors() const;
    bool hasErrors() const;
    
    // 日志功能
    void log(const QString &message);
    void logWarning(const QString &message);
    void logError(const QString &message);

    void setDutManager(std::shared_ptr<Services::DutManager> dutManager);
    std::shared_ptr<Services::DutManager> getDutManager() const;

signals:
    void dataChanged(const QString &key, const QVariant &value);
    void errorAdded(const QString &step, const QString &error);
    void logMessage(const QString &message);
    void logWarningMessage(const QString &message);
    void logErrorMessage(const QString &message);

private:
    QMap<QString, QVariant> m_data;
    std::shared_ptr<Services::DeviceManager> m_deviceManager;
    std::shared_ptr<Services::WorkflowManager> m_workflowManager;
    std::shared_ptr<Services::DutManager> m_dutManager;
    QString m_workflowId;
    QString m_workflowName;
    QString m_currentChipId;
    int m_currentSiteId;
    QJsonObject m_testResults;
    QMap<QString, QString> m_errors;
};

} // namespace Application

#endif // WORKFLOWCONTEXT_H