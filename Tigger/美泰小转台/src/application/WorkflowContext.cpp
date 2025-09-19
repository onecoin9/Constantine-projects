#include "application/WorkflowContext.h"
#include <QJsonDocument>

// Forward declaration to avoid circular dependencies
#include "services/DutManager.h"

namespace Application {

WorkflowContext::WorkflowContext(QObject *parent)
    : QObject(parent)
    , m_currentSiteId(-1)
{
    // 自动设置时间戳，供工作流步骤使用
    setData("timestamp", QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
}

WorkflowContext::~WorkflowContext()
{
}

void WorkflowContext::setData(const QString &key, const QVariant &value)
{
    m_data[key] = value;
    emit dataChanged(key, value);
}

QVariant WorkflowContext::getData(const QString &key) const
{
    return m_data.value(key);
}

bool WorkflowContext::hasData(const QString &key) const
{
    return m_data.contains(key);
}

void WorkflowContext::removeData(const QString &key)
{
    m_data.remove(key);
}

void WorkflowContext::clearData()
{
    m_data.clear();
}

void WorkflowContext::setJsonData(const QString &key, const QJsonObject &value)
{
    QJsonDocument doc(value);
    setData(key, doc.toJson(QJsonDocument::Compact));
}

QJsonObject WorkflowContext::getJsonData(const QString &key) const
{
    QVariant data = getData(key);
    if (data.isNull()) {
        return QJsonObject();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(data.toByteArray());
    return doc.object();
}

QMap<QString, QVariant> WorkflowContext::getAllData() const
{
    return m_data;
}

void WorkflowContext::setDeviceManager(std::shared_ptr<Services::DeviceManager> deviceManager)
{
    m_deviceManager = deviceManager;
}

std::shared_ptr<Services::DeviceManager> WorkflowContext::getDeviceManager() const
{
    return m_deviceManager;
}

void WorkflowContext::setWorkflowManager(std::shared_ptr<Services::WorkflowManager> workflowManager)
{
    m_workflowManager = workflowManager;
}

std::shared_ptr<Services::WorkflowManager> WorkflowContext::getWorkflowManager() const
{
    return m_workflowManager;
}

void WorkflowContext::setWorkflowId(const QString &id)
{
    m_workflowId = id;
}

QString WorkflowContext::getWorkflowId() const
{
    return m_workflowId;
}

void WorkflowContext::setWorkflowName(const QString &name)
{
    m_workflowName = name;
}

QString WorkflowContext::getWorkflowName() const
{
    return m_workflowName;
}

void WorkflowContext::setCurrentChipId(const QString &chipId)
{
    m_currentChipId = chipId;
}

QString WorkflowContext::getCurrentChipId() const
{
    return m_currentChipId;
}

void WorkflowContext::setCurrentSiteId(int siteId)
{
    m_currentSiteId = siteId;
}

int WorkflowContext::getCurrentSiteId() const
{
    return m_currentSiteId;
}

void WorkflowContext::addTestResult(const QString &testName, const QJsonObject &result)
{
    m_testResults[testName] = result;
}

QJsonObject WorkflowContext::getTestResult(const QString &testName) const
{
    return m_testResults[testName].toObject();
}

QJsonObject WorkflowContext::getAllTestResults() const
{
    return m_testResults;
}

void WorkflowContext::addError(const QString &step, const QString &error)
{
    m_errors[step] = error;
    emit errorAdded(step, error);
}

QStringList WorkflowContext::getErrors() const
{
    QStringList errors;
    for (auto it = m_errors.begin(); it != m_errors.end(); ++it) {
        errors << QString("%1: %2").arg(it.key()).arg(it.value());
    }
    return errors;
}

bool WorkflowContext::hasErrors() const
{
    return !m_errors.isEmpty();
}

void WorkflowContext::log(const QString &message)
{
    emit logMessage(message);
}

void WorkflowContext::logWarning(const QString &message)
{
    emit logWarningMessage(message);
}

void WorkflowContext::logError(const QString &message)
{
    emit logErrorMessage(message);
}

void WorkflowContext::setDutManager(Services::DutManager* dutManager)
{
    m_dutManager = dutManager;
}

Services::DutManager* WorkflowContext::getDutManager() const
{
    return m_dutManager;
}

} // namespace Application