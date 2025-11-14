#include "application/DeviceCommandStep.h"
#include "application/WorkflowContext.h"
#include "services/DeviceManager.h"
#include "services/DutManager.h"
#include "domain/IDevice.h"
#include "application/WorkflowUtils.h"
#include "core/Logger.h"
#include <QJsonDocument>
#include <QJsonArray>

namespace Application {

DeviceCommandStep::DeviceCommandStep(const QJsonObject &config, QObject *parent)
    : IWorkflowStep(parent) {
    setConfiguration(config);
}

DeviceCommandStep::~DeviceCommandStep() = default;
QJsonObject DeviceCommandStep::completeParams(const QString& command, QJsonObject& params)
{
    QJsonObject retParams = params;
    //if (command == "sendUid") {
    //    // 获取使能的dut的ip及stkEn
    //    auto dutManager = context->getDutManager();
    //    QJsonArray sktEnMapArray;
    //    auto siteInfoMap = dutManager->getAllSiteInfo();
    //    for (auto siteInfo : siteInfoMap) {
    //        if (siteInfo.isScanned) {
    //            QJsonObject sktEnMapObj;
    //            sktEnMapObj["strIP"] = siteInfo.ip;
    //            sktEnMapObj["sktEn"] = static_cast<int>(siteInfo.socketEnable);
    //            sktEnMapArray.append(sktEnMapObj);
    //            break;
    //        }
    //    }
    //    retParams["sktEnMapArray"] = sktEnMapArray;
    //    qDebug() << "retParams" << QJsonDocument(retParams).toJson(QJsonDocument::Compact);
    //}
    return retParams;
}
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

    //QString turnResult = resolvedArgs["result"].toString();
    //QByteArray resArray;
    //for (int i = 0; i < turnResult.length(); i++) {
    //    resArray.append(turnResult.at(i).toLatin1() - '0');
    //}
    //Services::DutManager::instance()->updateSiteChipStatus(context->getData("siteIp").toString(), resArray);

    //LOG_MODULE_INFO("DeviceCommandStep", QString("exec command(%1) - updateSiteChipStatus:").arg(command).append(resArray.toHex()).toStdString());
    LOG_MODULE_INFO("DeviceCommandStep", QString("Executing command '%1' on device '%2' with args: %3")
        .arg(command).arg(deviceId).arg(QString(QJsonDocument(resolvedArgs).toJson(QJsonDocument::Compact))).toStdString());

    //// 部分参数需要实时获取软件数据，无法从工作流文件中获取，暂时放这处理
    //if ((command == "doJob" && config.value("args").toObject()["command"].toString() == "Program")
    //    || (command == "doCustom")) {
        // 获取ip及stkEn
        resolvedArgs["strIp"] = context->getData("siteIp").toString();
        resolvedArgs["sktEn"] = context->getData("dutMask").toInt();
    //}
        
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
