#include "application/DeviceCommandStep.h"
#include "application/WorkflowContext.h"
#include "services/DeviceManager.h"
#include "domain/IDevice.h"
#include "application/WorkflowUtils.h"
#include "core/Logger.h"
#include <QJsonDocument>

namespace Application {

DeviceCommandStep::DeviceCommandStep(const QJsonObject &config, QObject *parent)
    : IWorkflowStep(parent) {
    setConfiguration(config);
}

DeviceCommandStep::~DeviceCommandStep() = default;

bool DeviceCommandStep::execute(std::shared_ptr<WorkflowContext> context) {
    m_status = StepStatus::Running;

    if (!context || !context->getDeviceManager()) {
        LOG_MODULE_ERROR("DeviceCommandStep", "Context or DeviceManager is not available.");
        m_status = StepStatus::Failed;
        return false;
    }

    QJsonObject config = m_config.value("config").toObject();
    
    QString deviceId = config.value("deviceId").toString();
    QString command = config.value("command").toString();
    
    if (deviceId.isEmpty() || command.isEmpty()) {
        LOG_MODULE_ERROR("DeviceCommandStep", "'deviceId' or 'command' is missing in config.");
        m_status = StepStatus::Failed;
        return false;
    }
    
    QJsonObject args = config.value("args").toObject();
    QJsonObject resolvedArgs = WorkflowUtils::replacePlaceholders(args, context);

    auto deviceManager = context->getDeviceManager();
    auto device = deviceManager->getDevice(deviceId);
    if (!device) {
        LOG_MODULE_ERROR("DeviceCommandStep", QString("Device '%1' not found.").arg(deviceId).toStdString());
        m_status = StepStatus::Failed;
        return false;
    }
    
    LOG_MODULE_INFO("DeviceCommandStep", QString("Executing command '%1' on device '%2' with args: %3")
        .arg(command).arg(deviceId).arg(QString(QJsonDocument(resolvedArgs).toJson(QJsonDocument::Compact))).toStdString());

    QJsonObject result = device->executeCommand(command, resolvedArgs);
    m_result = result;

    QString storeResultIn = config.value("storeResultIn").toString();
    if (!storeResultIn.isEmpty()) {
        context->setData(storeResultIn, result);
        LOG_MODULE_INFO("DeviceCommandStep", QString("Stored result in context under key '%1'.").arg(storeResultIn).toStdString());
    }

    m_status = StepStatus::Completed;
    return true;
}

QString DeviceCommandStep::getName() const {
    return m_config.value("name").toString("DeviceCommandStep");
}

QString DeviceCommandStep::getDescription() const {
    return m_config.value("description").toString("Executes a command on a specified device.");
}

IWorkflowStep::StepStatus DeviceCommandStep::getStatus() const {
    return m_status;
}

QJsonObject DeviceCommandStep::getResult() const {
    return m_result;
}

void DeviceCommandStep::setConfiguration(const QJsonObject &config) {
    m_config = config;
}

QJsonObject DeviceCommandStep::getConfiguration() const {
    return m_config;
}

bool DeviceCommandStep::canExecute(std::shared_ptr<WorkflowContext> context) const {
     if (!context || !context->getDeviceManager()) return false;

    QJsonObject config = m_config.value("config").toObject();
    return !config.value("deviceId").toString().isEmpty() && !config.value("command").toString().isEmpty();
}

} // namespace Application
