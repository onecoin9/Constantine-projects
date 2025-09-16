#include "domain/HandlerDevice.h"
#include "domain/protocols/SProtocol.h"
#include "core/Logger.h"
#include "services/DutManager.h" // 包含DutManager头文件
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QDateTime>

namespace Domain {

HandlerDevice::HandlerDevice(QObject *parent)
    : IDevice(parent)
    , m_sProtocol(nullptr)
    , m_status(DeviceStatus::Disconnected)
    , m_isSimulation(false)
    , m_heartbeatTimer(nullptr)
    , m_isLinkHealthy(false)
{
}

HandlerDevice::~HandlerDevice()
{
    if (m_sProtocol) {
        m_sProtocol->release();
        m_sProtocol.reset();
    }
    if (m_heartbeatTimer) {
        m_heartbeatTimer->stop();
        delete m_heartbeatTimer;
        m_heartbeatTimer = nullptr;
    }
}

bool HandlerDevice::initialize()
{
    m_status = DeviceStatus::Initializing;
    emit statusChanged(m_status);

    // 创建SProtocol实例
    m_sProtocol = std::make_unique<Protocols::SProtocol>();
    if (!m_sProtocol) {
        LOG_MODULE_ERROR("HandlerDevice", "Failed to create SProtocol instance");
        m_status = DeviceStatus::Error;
        emit statusChanged(m_status);
        return false;
    }
    
    // 准备SProtocol配置
    QJsonObject sProtocolConfig = m_configuration;
    
    // 如果配置中没有协议参数，使用默认值
    if (!sProtocolConfig.contains("vautoHost")) {
        sProtocolConfig["vautoHost"] = "127.0.0.1";
    }
    if (!sProtocolConfig.contains("cmd3Port")) {
        sProtocolConfig["cmd3Port"] = 1000;
    }
    if (!sProtocolConfig.contains("commandPort")) {
        sProtocolConfig["commandPort"] = 64100;
    }
    if (!sProtocolConfig.contains("serverPort")) {
        sProtocolConfig["serverPort"] = 64101;
    }
    if (!sProtocolConfig.contains("connectTimeout")) {
        sProtocolConfig["connectTimeout"] = 5000;
    }
    if (!sProtocolConfig.contains("version")) {
        sProtocolConfig["version"] = 1;
    }
    
    LOG_MODULE_DEBUG("HandlerDevice", QString("Initializing SProtocol with config: %1")
                   .arg(QString(QJsonDocument(sProtocolConfig).toJson(QJsonDocument::Compact))).toStdString());
    
    // 初始化协议（SProtocol内部处理所有通信配置）
    if (!m_sProtocol->initialize(sProtocolConfig)) {
        m_lastError = m_sProtocol->getLastError();
        LOG_MODULE_ERROR("HandlerDevice", QString("Failed to initialize SProtocol: %1").arg(m_lastError).toStdString());
        m_status = DeviceStatus::Error;
        emit statusChanged(m_status);
        return false;
    }
    
    // 连接信号
    connectSignals();
    
    // 【新增】初始化心跳定时器
    if (m_configuration.value("enableHeartbeat").toBool(false)) {
        int interval = m_configuration.value("heartbeatIntervalMs").toInt(5000);
        m_heartbeatTimer = new QTimer(this);
        connect(m_heartbeatTimer, &QTimer::timeout, this, &HandlerDevice::onHeartbeatTimeout);
        m_heartbeatTimer->start(interval);
        LOG_MODULE_INFO("HandlerDevice", QString("Heartbeat enabled with interval: %1 ms").arg(interval).toStdString());
        // 首次立即触发一次心跳
        QTimer::singleShot(100, this, &HandlerDevice::onHeartbeatTimeout);
    }

    LOG_MODULE_INFO("HandlerDevice", "HandlerDevice initialized successfully");
    
    m_status = DeviceStatus::Ready;
    emit statusChanged(m_status);
    return true;
}

bool HandlerDevice::release()
{
    if (m_sProtocol) {
        m_sProtocol->release();
        m_sProtocol.reset();
    }
    
    m_status = DeviceStatus::Disconnected;
    emit statusChanged(m_status);
    return true;
}

QJsonObject HandlerDevice::executeCommand(const QString &command, const QJsonObject &params)
{
    if (!m_sProtocol || (m_status != DeviceStatus::Ready && m_status != DeviceStatus::Connected)) {
        // 心跳命令特殊处理：即使设备未连接，也应尝试发送以探测链路
        if (command != "heartbeat") {
            QString error = QString("Device not ready or protocol unavailable, status: %1").arg(static_cast<int>(m_status));
            LOG_MODULE_WARNING("HandlerDevice", error.toStdString());
            return {{"success", false}, {"error", error}};
        }
    }
    
    LOG_MODULE_INFO("HandlerDevice", QString("Executing command: %1").arg(command).toStdString());
    
    try {
        if (command == "sendCmd3Command"||command == "loadTask") {
            QString taskPath = params.contains("taskPath") ? params["taskPath"].toString() : params["command"].toString();
            
            QString cmd3String = buildCmd3Command(taskPath, params);
            bool success = m_sProtocol->sendCmd3Command(cmd3String);
            return {{"success", success}, {"command", command}};
            
        } else if (command == "sendPduCommand") {
            uint16_t pflag = params["pflag"].toInt();
            uint8_t pdu = params["pdu"].toInt();
            QByteArray data = QByteArray::fromHex(params["data"].toString().toLatin1());
            bool success = m_sProtocol->sendPduCommand(pflag, pdu, data);
            return {{"success", success}, {"command", command}};
            
        } else if (command == "sendAxisMove") {
            // 构建符合协议规范的轴移动命令
            QJsonObject axisCmd;
            axisCmd["AxisSelect"] = params["axisSelect"].toString();  // 轴选择：Axis_Pitch, Axis_Yaw等
            axisCmd["SiteIdx"] = QString::number(params["siteIdx"].toInt());      // 站点索引（字符串格式）
            axisCmd["TargetAngle"] = QString::number(params["targetAngle"].toInt()); // 目标角度（字符串格式）
            
            LOG_MODULE_INFO("HandlerDevice", QString("Building AxisMove: Axis=%1, Site=%2, Target=%3")
                           .arg(axisCmd["AxisSelect"].toString())
                           .arg(axisCmd["SiteIdx"].toString()) 
                           .arg(axisCmd["TargetAngle"].toString()).toStdString());
            
            bool success = m_sProtocol->sendAxisMoveCommand(axisCmd);
            return {{"success", success}, {"command", command}};
            
        } else if (command == "tellDevReady") {
            // 严格按照协议V4.5，从配置动态构建PDU 0x63
            QByteArray pduData = buildPdu63Data(m_configuration);
            if (pduData.isEmpty()) {
                QString errorMsg = "Failed to build PDU 0x63 data from configuration. Check logs for details.";
                LOG_MODULE_ERROR("HandlerDevice", errorMsg.toStdString());
                return {{"success", false}, {"error", errorMsg}};
            }
            
            bool success = m_sProtocol->sendPduCommand(
                Protocols::ISProtocol::PFLAG_SA, 
                Protocols::ISProtocol::PDU_TELLDEVINIT, 
                pduData
            );
            return {{"success", success}, {"command", command}};
            
        } else if (command == "tellDevCommVersion") {
            // 发送PDU 0x61告知版本信息
            QByteArray versionData;
            versionData.append(static_cast<char>(params["version"].toInt()));
            
            bool success = m_sProtocol->sendPduCommand(
                Protocols::ISProtocol::PFLAG_SA,
                Protocols::ISProtocol::PDU_TELLVERSION,
                versionData
            );
            return {{"success", success}, {"command", command}};
            
        } else if (command == "setDoneSite") {
            // 发送PDU 0x67告知烧录结果
            
            // 添加详细的参数调试信息
            LOG_MODULE_DEBUG("HandlerDevice", QString("setDoneSite: Raw params: %1")
                            .arg(QString(QJsonDocument(params).toJson(QJsonDocument::Compact))).toStdString());
            
            // 检查siteIdx参数的类型和值
            QJsonValue siteIdxValue = params["siteIdx"];
            LOG_MODULE_DEBUG("HandlerDevice", QString("setDoneSite: siteIdx value type: %1, isString: %2, isDouble: %3, raw value: '%4'")
                            .arg(static_cast<int>(siteIdxValue.type()))
                            .arg(siteIdxValue.isString())
                            .arg(siteIdxValue.isDouble())
                            .arg(siteIdxValue.toString()).toStdString());
            
            // 使用更安全的参数解析方式
            int siteIdx = 0;
            bool parseSuccess = false;
            
            if (siteIdxValue.isString()) {
                QString siteIdxStr = siteIdxValue.toString().trimmed();
                siteIdx = siteIdxStr.toInt(&parseSuccess);
                LOG_MODULE_DEBUG("HandlerDevice", QString("setDoneSite: String parsing: '%1' -> %2, success: %3")
                                .arg(siteIdxStr).arg(siteIdx).arg(parseSuccess).toStdString());
            } else if (siteIdxValue.isDouble()) {
                siteIdx = siteIdxValue.toInt();
                parseSuccess = true;
                LOG_MODULE_DEBUG("HandlerDevice", QString("setDoneSite: Direct number conversion: %1").arg(siteIdx).toStdString());
            } else {
                // 尝试从任何类型转换为字符串再解析
                QString fallbackStr = siteIdxValue.toVariant().toString().trimmed();
                siteIdx = fallbackStr.toInt(&parseSuccess);
                LOG_MODULE_DEBUG("HandlerDevice", QString("setDoneSite: Fallback conversion: '%1' -> %2, success: %3")
                                .arg(fallbackStr).arg(siteIdx).arg(parseSuccess).toStdString());
            }
            
            if (!parseSuccess || siteIdx <= 0) {
                LOG_MODULE_ERROR("HandlerDevice", QString("setDoneSite: Failed to parse siteIdx or invalid value: %1")
                                .arg(siteIdx).toStdString());
                return {{"success", false}, {"error", QString("Invalid siteIdx value: %1").arg(siteIdxValue.toString())}};
            }
            
            // 用作仅采集测试！！！ 发布版需删除
            QByteArray resultData = Services::DutManager::instance()->getSiteInfoByIndex(siteIdx).currentChipStatus;
            //if (resultData.isEmpty()) {
            //    resultData.append(0x01);
            //    resultData.append(15, 0);
            //}
            LOG_MODULE_INFO("HandlerDevice", QString("exec command - getSiteChipStatus:").append(resultData.toHex()).toStdString());
            
            LOG_MODULE_INFO("HandlerDevice", QString("Sending PDU 0x67 with vauto optimized special site handling. Site=%1, Result=%2")
                           .arg(siteIdx).arg(QString(resultData.toHex())).toStdString());
            
            QByteArray pduData;
            pduData.append(static_cast<char>(siteIdx));
            pduData.append(resultData);
            
            bool success = m_sProtocol->sendPduCommand(
                Protocols::ISProtocol::PFLAG_SA,
                Protocols::ISProtocol::PDU_TELLRESULTS,
                pduData
            );
            
            if (success) {
                LOG_MODULE_INFO("HandlerDevice", QString("PDU 0x67 sent successfully for Site %1")
                               .arg(siteIdx).toStdString());
                // 发射测试结果信号，供CoreEngine更新DutManager
                emit testResultSent(siteIdx, resultData);
            } else {
                LOG_MODULE_ERROR("HandlerDevice", QString("PDU 0x67 send failed for Site %1 - target may not be running or unreachable")
                                .arg(siteIdx).toStdString());
            }
            
            return {{"success", success}, {"command", command}};
            
        } else if (command == "contactCheckResult") {
            // 发送PDU 0x68告知接触检查结果
            int siteIdx = params["siteIdx"].toInt();
            QJsonArray resultArray = params["result"].toArray();
            
            QByteArray pduData;
            pduData.append(static_cast<char>(siteIdx));
            for (const auto& value : resultArray) {
                pduData.append(static_cast<char>(value.toInt()));
            }
            
            bool success = m_sProtocol->sendPduCommand(
                Protocols::ISProtocol::PFLAG_SA,
                Protocols::ISProtocol::PDU_TELLICSTATUS,
                pduData
            );
            return {{"success", success}, {"command", command}};
            
        }else if (command == "tellPrint") {
            // 发送PDU 0x70告知接触镭雕内容
            QJsonObject rootObj;
            rootObj.insert("SiteIdx", params["siteIdx"].toInt());
            rootObj.insert("ContentEn", 1);
            rootObj.insert("Swap", 1);
            rootObj.insert("Manufactor", "XTTurn");

            //FIXME, from MTUID instance?
            QJsonArray contentArray;
            QJsonArray socketArray = params.value("SKT").toArray();
            for (const auto& value : socketArray) {
                QJsonObject socketObj = value.toObject();
                QJsonObject contentItem;
                //FIXME Swap=1, need Change SktIdx to Auto SocketIdx
                contentItem.insert("SktIdx", socketObj["SktIdx"].toInt());
                contentItem.insert("Uid", socketObj["Uid"].toInt());
                contentItem.insert("Text", socketObj["Text"].toInt());
                contentItem.insert("Valid", 1);
                contentArray.append(contentItem);
            }

            rootObj.insert("Content", contentArray);
            QJsonDocument doc(rootObj);
            QByteArray pduData = doc.toJson();
            bool success = m_sProtocol->sendPduCommand(
                Protocols::ISProtocol::PFLAG_SA,
                Protocols::ISProtocol::PDU_TELLPRINT,
                pduData
            );
            return { {"success", success}, {"command", command} };
        }
        else if (command == "remainingCheckResult") {
            // 发送PDU 0x65告知残料检查结果
            int siteIdx = params["siteIdx"].toInt();
            QJsonArray resultArray = params["result"].toArray();
            
            QByteArray pduData;
            pduData.append(static_cast<char>(siteIdx));
            for (const auto& value : resultArray) {
                pduData.append(static_cast<char>(value.toInt()));
            }
            
            bool success = m_sProtocol->sendPduCommand(
                Protocols::ISProtocol::PFLAG_SA,
                Protocols::ISProtocol::PDU_TELLREMAINING,
                pduData
            );
            return {{"success", success}, {"command", command}};
            
        } else if (command == "getSiteMap") {
            // 获取站点映射（从配置读取）
            QJsonObject siteConfig = m_configuration.value("siteConfiguration").toObject();
            QJsonArray sitesArray = siteConfig.value("sites").toArray();
            
            QJsonObject result = {{"success", true}, {"command", "getSiteMap"}};
            for (const auto& value : sitesArray) {
                QJsonObject siteObj = value.toObject();
                int siteIndex = siteObj["siteIndex"].toInt();
                QString siteAlias = siteObj["siteAlias"].toString();
                result[QString::number(siteIndex)] = siteAlias;
            }
            
            // 异步发射信号
            QTimer::singleShot(0, this, [this, result]() {
                emit commandFinished(result);
            });
            
            return result;
            
        } else if (command == "startProductionMonitoring") {
            // 开始生产监控
            LOG_MODULE_INFO("HandlerDevice", "Starting production monitoring");
            return {{"success", true}, {"command", command}};
            
        } else if (command == "updateProgress") {
            // 更新进度
            QString phase = params["phase"].toString();
            int currentStep = params["currentStep"].toInt();
            int totalSteps = params["totalSteps"].toInt();
            LOG_MODULE_INFO("HandlerDevice", QString("Progress update: %1 - %2/%3")
                           .arg(phase).arg(currentStep).arg(totalSteps).toStdString());
            return {{"success", true}, {"command", command}};
            
        } else if (command == "finalizeProduction") {
            // 完成生产
            LOG_MODULE_INFO("HandlerDevice", "Finalizing production");
            return {{"success", true}, {"command", command}};
            
        } else if (command == "heartbeat") {
            // 【新增】处理心跳命令
            bool success = m_sProtocol->sendPduCommand(
                Protocols::ISProtocol::PFLAG_SA,
                Protocols::ISProtocol::PDU_QUERYVERSION, // 使用0xE1作为心跳包
                QByteArray()
            );

            // 【新增】当链路状态发生变化时，发射信号
            if (m_isLinkHealthy != success) {
                m_isLinkHealthy = success;
                emit linkHealthChanged(m_isLinkHealthy);
            }

            // 更新链路状态
            if (success) {
                m_lastSuccessfulHeartbeat = QDateTime::currentDateTime();
                LOG_MODULE_DEBUG("HandlerDevice", "Heartbeat sent successfully, link is healthy.");
            } else {
                LOG_MODULE_WARNING("HandlerDevice", "Heartbeat send failed, link is unhealthy.");
            }
            return {{"success", success}};
        } else {
            LOG_MODULE_WARNING("HandlerDevice", QString("Unknown command: %1").arg(command).toStdString());
            return {{"success", false}, {"error", QString("Unknown command: %1").arg(command)}};
        }

    } catch (const std::exception& e) {
        LOG_MODULE_ERROR("HandlerDevice", QString("Exception in executeCommand: %1").arg(e.what()).toStdString());
        return {{"success", false}, {"error", QString("Exception: %1").arg(e.what())}};
    }
}

void HandlerDevice::setConfiguration(const QJsonObject &config)
{
    m_configuration = config;
    m_isSimulation = config.value("isSimulation").toBool(false);
}

IDevice::DeviceStatus HandlerDevice::getStatus() const
{
    if (m_status == DeviceStatus::Error) {
        return DeviceStatus::Error;
    }

    if (m_sProtocol && m_sProtocol->isConnected()) {
        return DeviceStatus::Connected;
    }
    
    return m_status;
}

QString HandlerDevice::getName() const
{
    QString name = m_configuration.value("deviceId").toString();
    if (name.isEmpty()) {
        name = m_configuration.value("name").toString();
    }
    if (name.isEmpty()) {
        name = "HandlerDevice";
    }
    return name;
}

IDevice::DeviceType HandlerDevice::getType() const
{
    return DeviceType::Handler;
}

QString HandlerDevice::getDescription() const
{
    return "自动化处理设备 - 使用S协议通信";
}

bool HandlerDevice::selfTest()
{
    return m_sProtocol && m_sProtocol->isConnected() && (m_status == DeviceStatus::Ready);
}

void HandlerDevice::setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel> channel)
{
    // SProtocol使用单例通道，此方法仅为接口兼容
    Q_UNUSED(channel);
}

std::shared_ptr<Infrastructure::ICommunicationChannel> HandlerDevice::getCommunicationChannel() const
{
    // 返回nullptr，因为通道由SProtocol内部管理
    return nullptr;
}

QJsonObject HandlerDevice::getConfiguration() const
{
    return m_configuration;
}

Protocols::ISProtocol* HandlerDevice::getSProtocol() const
{
    return m_sProtocol.get();
}

bool HandlerDevice::isLinkHealthy() const
{
    // 如果禁用了心跳，则认为链路总是健康的（兼容旧逻辑）
    if (!m_heartbeatTimer || !m_heartbeatTimer->isActive()) {
        return true;
    }
    return m_isLinkHealthy;
}

void HandlerDevice::connectSignals()
{
    if (!m_sProtocol) {
        return;
    }
    
    // 连接基础数据接收信号
    connect(m_sProtocol.get(), &Protocols::ISProtocol::pduCommandReceived,
            this, &HandlerDevice::onPduCommandReceived);
    
    connect(m_sProtocol.get(), &Protocols::ISProtocol::jsonCommandReceived,
            this, &HandlerDevice::onJsonCommandReceived);
    
    // 连接PDU业务信号
    connect(m_sProtocol.get(), &Protocols::ISProtocol::versionQueryReceived,
            this, &HandlerDevice::onVersionQueryReceived);
    
    connect(m_sProtocol.get(), &Protocols::ISProtocol::chipPlacementReceived,
            this, &HandlerDevice::onChipPlacementReceived);
    
    connect(m_sProtocol.get(), &Protocols::ISProtocol::icStatusCheckRequested,
            this, &HandlerDevice::onICStatusCheckRequested);
    
    connect(m_sProtocol.get(), &Protocols::ISProtocol::remainingCheckRequested,
            this, &HandlerDevice::onRemainingCheckRequested);
    
    connect(m_sProtocol.get(), &Protocols::ISProtocol::siteEnableReceived,
            this, &HandlerDevice::onSiteEnableReceived);
    
    // 连接轴移动JSON业务信号
    connect(m_sProtocol.get(), &Protocols::ISProtocol::axisMovementRequested,
            this, &HandlerDevice::onAxisMovementRequested);
    
    connect(m_sProtocol.get(), &Protocols::ISProtocol::axisMovementCompleted,
            this, &HandlerDevice::onAxisMovementCompleted);
    
    // 连接产品信息JSON业务信号
    connect(m_sProtocol.get(), &Protocols::ISProtocol::productInfoReceived,
            this, &HandlerDevice::onProductInfoReceived);

    // 连接状态信号
    connect(m_sProtocol.get(), &Protocols::ISProtocol::connected,
            this, &HandlerDevice::onProtocolConnected);
    
    connect(m_sProtocol.get(), &Protocols::ISProtocol::disconnected,
            this, &HandlerDevice::onProtocolDisconnected);
    
    connect(m_sProtocol.get(), &Protocols::ISProtocol::errorOccurred,
            this, &HandlerDevice::onProtocolError);
    
    // 连接原始数据信号（用于调试）
    connect(m_sProtocol.get(), &Protocols::ISProtocol::rawDataReceived,
            this, [this](const QByteArray& data) {
                emit rawFrameReceived(data);
            });
    
    LOG_MODULE_INFO("HandlerDevice", "SProtocol signals connected");
}

// 【新增】心跳超时处理
void HandlerDevice::onHeartbeatTimeout()
{
    LOG_MODULE_DEBUG("HandlerDevice", "Heartbeat timer triggered. Sending heartbeat command...");
    executeCommand("heartbeat", QJsonObject());
}

// 槽函数实现
void HandlerDevice::onPduCommandReceived(uint16_t pflag, uint8_t pdu, const QByteArray& data)
{
    LOG_MODULE_DEBUG("HandlerDevice", QString("PDU command received: PFLAG=0x%1, PDU=0x%2, DataLen=%3")
                    .arg(pflag, 4, 16, QChar('0')).arg(pdu, 2, 16, QChar('0')).arg(data.size()).toStdString());
    
    emit rawFrameReceived(data); // 转发给UI用于调试显示
}

void HandlerDevice::onJsonCommandReceived(const QJsonObject& command)
{
    LOG_MODULE_DEBUG("HandlerDevice", QString("JSON command received: %1")
                    .arg(QString(QJsonDocument(command).toJson(QJsonDocument::Compact))).toStdString());
    
    QByteArray jsonData = QJsonDocument(command).toJson(QJsonDocument::Compact);
    emit rawFrameReceived(jsonData); // 转发给UI用于调试显示
}

void HandlerDevice::onVersionQueryReceived()
{
    LOG_MODULE_INFO("HandlerDevice", "Version query received from AutoApp");
    emit debugMessage("收到版本查询请求(0xE1)");
    
    // 自动回复版本信息已在SProtocol中处理
}

void HandlerDevice::onChipPlacementReceived(int siteIdx, const QByteArray& chipData)
{
    LOG_MODULE_INFO("HandlerDevice", QString("Chip placement received: Site=%1, DataLen=%2")
                    .arg(siteIdx).arg(chipData.size()).toStdString());
    
    // 解析chipData - VAuto发送的是每个socket的使能状态
    // 对于16个socket的站点，chipData应该有16个字节
    uint32_t slotEn = 0;
    
    // 根据协议，每个字节表示一个socket的状态
    // 0x0F表示前4个socket有芯片
    for (int i = 0; i < chipData.size() && i < 16; i++) {
        if (static_cast<uint8_t>(chipData[i]) != 0) {
            slotEn |= (1 << i);
        }
    }
    
    // 如果chipData的第一个字节是0x0F，表示前4个socket有芯片
    if (!chipData.isEmpty() && static_cast<uint8_t>(chipData[0]) == 0x0F) {
        slotEn = 0x0F; // Socket 1-4有芯片
    }
    
    // 根据站点配置获取站点SN
    QString siteSn;
    QJsonObject siteConfig = m_configuration.value("siteConfiguration").toObject();
    QJsonArray sites = siteConfig.value("sites").toArray();
    for (const auto& siteValue : sites) {
        QJsonObject site = siteValue.toObject();
        if (site["siteIndex"].toInt() == siteIdx) {
            siteSn = site["siteSN"].toString();
            break;
        }
    }
    
    if (siteSn.isEmpty()) {
        siteSn = QString("Site%1").arg(siteIdx, 2, 10, QChar('0'));
    }
    
    LOG_MODULE_INFO("HandlerDevice", QString("Chip placed at site %1 (SN: %2), SlotEn=0x%3")
                    .arg(siteIdx).arg(siteSn).arg(slotEn, 0, 16).toStdString());
    
    emit chipPlaced(siteIdx, slotEn, siteSn);
}

void HandlerDevice::onICStatusCheckRequested(int siteIdx, const QByteArray& checkData)
{
    LOG_MODULE_INFO("HandlerDevice", QString("IC status check requested: Site=%1, DataLen=%2")
                    .arg(siteIdx).arg(checkData.size()).toStdString());
    
    emit contactCheckRequested(siteIdx, checkData);
}

void HandlerDevice::onRemainingCheckRequested(int siteIdx, const QByteArray& checkData)
{
    LOG_MODULE_INFO("HandlerDevice", QString("Remaining check requested: Site=%1, DataLen=%2")
                    .arg(siteIdx).arg(checkData.size()).toStdString());
    
    emit remainingCheckRequested(siteIdx, checkData);
}

void HandlerDevice::onSiteEnableReceived(const QByteArray& enableData)
{
    LOG_MODULE_INFO("HandlerDevice", QString("Site enable received: DataLen=%1")
                    .arg(enableData.size()).toStdString());
    
    emit debugMessage(QString("收到站点使能配置: %1").arg(QString(enableData.toHex(' ').toUpper())));
}

void HandlerDevice::onAxisMovementRequested(const QString& axisSelect, int siteIdx, int targetAngle)
{
    LOG_MODULE_INFO("HandlerDevice", QString("Axis movement requested: %1, Site=%2, Target=%3°")
                    .arg(axisSelect).arg(siteIdx).arg(targetAngle).toStdString());
    
    emit axisMovementRequested(axisSelect, siteIdx, targetAngle);
}

void HandlerDevice::onAxisMovementCompleted(const QString& axisSelect, int siteIdx, int currentAngle, int result)
{
    LOG_MODULE_INFO("HandlerDevice", QString("Axis movement completed: %1, Site=%2, Current=%3°, Result=%4")
                    .arg(axisSelect).arg(siteIdx).arg(currentAngle).arg(result).toStdString());
    
    QString errMsg = (result == 0) ? "" : "轴移动执行失败";
    emit axisMovementCompleted(axisSelect, siteIdx, currentAngle, result, errMsg);
    
    // 发射命令完成信号
    QJsonObject commandResult;
    commandResult["success"] = (result == 0);
    commandResult["command"] = "sendAxisMove";
    commandResult["axisSelect"] = axisSelect;
    commandResult["siteIdx"] = siteIdx;
    commandResult["currentAngle"] = currentAngle;
    commandResult["result"] = result;
    
    QTimer::singleShot(0, this, [this, commandResult]() {
        emit commandFinished(commandResult);
    });
}

void HandlerDevice::onProductInfoReceived(const QJsonArray& rotateInfo)
{
    LOG_MODULE_INFO("HandlerDevice", QString("Product info received with %1 items.")
                    .arg(rotateInfo.size()).toStdString());

    // if (!m_dutManager) {
    //     LOG_MODULE_WARNING("HandlerDevice", "DutManager not set, cannot update DUT info.");
    //     emit productInfoReceived(rotateInfo);
    //     return;
    // }

    // for (const QJsonValue& value : rotateInfo) {
    //     QJsonObject item = value.toObject();
    //     QString rotateIdxStr = item["RotateIdx"].toString(); // "1-1", "1-2" ... "1-16"
    //     QString fromUnitIdxStr = item["FromUnitIdx"].toString(); // 来源站点号, "0"表示空
    //     QString fromSktIdxStr = item["FromSktIdx"].toString();   // 来源插座号, "0"表示空

    //     // 解析转台DUT编号, e.g., "1-16" -> 16
    //     int dutSlot = rotateIdxStr.split('-').last().toInt();
    //     QString dutId = QString("DUT_Site9-Socket%1").arg(dutSlot); // 假设9号站点的DUT ID格式

    //     if (fromUnitIdxStr == "0" || fromSktIdxStr == "0") {
    //         // 当前转台插座为空
    //         m_dutManager->setState(dutId, Domain::Dut::State::Idle);
    //         m_dutManager->setTestData(dutId, {{"source", "Empty"}});
    //         LOG_MODULE_INFO("HandlerDevice", QString("Turntable DUT %1 is empty.").arg(dutId).toStdString());
    //     } else {
    //         // 当前转台插座有产品
    //         int sourceSiteIndex = fromUnitIdxStr.toInt();
    //         int sourceSocketIndex = fromSktIdxStr.toInt();
            
    //         // 查找源DUT
    //         Services::DutManager::SiteInfo sourceSiteInfo = m_dutManager->getSiteInfoByIndex(sourceSiteIndex);
    //         if (sourceSiteInfo.siteAlias.isEmpty()) {
    //             LOG_MODULE_WARNING("HandlerDevice", QString("Source site with index %1 not found for turntable DUT %2")
    //                                .arg(sourceSiteIndex).arg(dutId).toStdString());
    //             continue;
    //         }

    //         QString sourceDutId = QString("DUT_%1-Socket%2").arg(sourceSiteInfo.siteAlias).arg(sourceSocketIndex);
    //         auto sourceDut = m_dutManager->getDut(sourceDutId);
            
    //         QVariantMap testData;
    //         testData["sourceSite"] = sourceSiteInfo.siteAlias;
    //         testData["sourceSocket"] = sourceSocketIndex;
    //         if (sourceDut) {
    //             testData["sourceDutData"] = sourceDut->getTestData();
    //             m_dutManager->setState(sourceDutId, Domain::Dut::State::AtTestStation); // 将源DUT标记为已移动到测试站
    //         }
    //         qDebug()<<"77777";
    //         m_dutManager->setState(dutId, Domain::Dut::State::AtTestStation);
    //         m_dutManager->setTestData(dutId, testData);

    //         LOG_MODULE_INFO("HandlerDevice", QString("Turntable DUT %1 loaded from Site %2, Socket %3")
    //                        .arg(dutId).arg(sourceSiteIndex).arg(sourceSocketIndex).toStdString());
    //     }
    // }

    emit productInfoReceived(rotateInfo);
}

void HandlerDevice::onProtocolConnected()
{
    LOG_MODULE_INFO("HandlerDevice", "Protocol connected");
    m_status = DeviceStatus::Connected;
    emit statusChanged(m_status);
}

void HandlerDevice::onProtocolDisconnected()
{
    LOG_MODULE_INFO("HandlerDevice", "Protocol disconnected");
    m_status = DeviceStatus::Ready; // 服务器仍在监听
    emit statusChanged(m_status);
}

void HandlerDevice::onProtocolError(const QString& error)
{
    LOG_MODULE_ERROR("HandlerDevice", QString("Protocol error: %1").arg(error).toStdString());
    m_lastError = error;
    m_status = DeviceStatus::Error;
    emit statusChanged(m_status);
    emit errorOccurred(error);
}

QByteArray HandlerDevice::buildPdu63Data(const QJsonObject& config)
{
    // 使用成员变量中的站点配置
    QJsonDocument doc(config);
    QByteArray jsonStr = doc.toJson(QJsonDocument::Indented); // Indented 表示格式化缩进

    // 输出到调试日志（控制台或日志文件）
    LOG_MODULE_INFO("HandlerDevice", jsonStr.toStdString());
    QJsonObject siteConfig = config.value("siteConfiguration").toObject();
    if (siteConfig.isEmpty()) {
        LOG_MODULE_ERROR("HandlerDevice", "buildPdu63Data: 'siteConfiguration' not found or is not an object.");
        return QByteArray();
    }

    QJsonObject globalSettings = siteConfig.value("globalSocketSettings").toObject();
    if (globalSettings.isEmpty()) {
        LOG_MODULE_ERROR("HandlerDevice", "buildPdu63Data: 'globalSocketSettings' not found or is not an object.");
        return QByteArray();
    }

    int sktCnt = globalSettings.value("maxSocketsPerSite").toInt(16);
    if (sktCnt <= 0 || sktCnt % 8 != 0) {
        LOG_MODULE_ERROR("HandlerDevice", QString("buildPdu63Data: Invalid 'maxSocketsPerSite' value: %1. Must be a multiple of 8.").arg(sktCnt).toStdString());
        return QByteArray();
    }

    QJsonArray sites = siteConfig.value("sites").toArray();
    if (sites.isEmpty()) {
        LOG_MODULE_ERROR("HandlerDevice", "buildPdu63Data: 'sites' array not found or is empty.");
        return QByteArray();
    }

    QMap<int, QJsonObject> activeSitesMap;
    int maxSiteIndex = 0;
    for (const QJsonValue& val : sites) {
        QJsonObject siteObj = val.toObject();
        if (siteObj.value("siteEnvInit").toBool(false)) {
            int siteIndex = siteObj["siteIndex"].toInt();
            activeSitesMap[siteIndex] = siteObj;
            if (siteIndex > maxSiteIndex) {
                maxSiteIndex = siteIndex;
            }
        }
    }

    if (activeSitesMap.isEmpty()) {
        LOG_MODULE_WARNING("HandlerDevice", "buildPdu63Data: No sites found with 'siteEnvInit' set to true. Sending empty PDU63.");
    }
    
    int siteCnt = maxSiteIndex > 0 ? maxSiteIndex : 0;
    int sktBytesPerSite = sktCnt / 8;
    int siteEnBitmapSize = siteCnt * sktBytesPerSite;
    QByteArray siteEnBitmap(siteEnBitmapSize, 0);

    for (int i = 1; i <= siteCnt; ++i) {
        quint64 socketEnable = 0; // Default to disabled
        
        // 从DutManager单例获取socketEnable
        Services::DutManager::SiteInfo siteInfo = Services::DutManager::instance()->getSiteInfoByIndex(i);
        //if (siteInfo.siteIndex > 0) {  // 有效的站点
        if (0) {
            socketEnable = siteInfo.socketEnable;
            LOG_MODULE_INFO("HandlerDevice", QString("Using socketEnable from DutManager for site %1: 0x%2")
                .arg(i).arg(socketEnable, 0, 16).toStdString());
        } else {
            // 如果在DutManager中找不到站点信息，回退到使用JSON配置
            if (activeSitesMap.contains(i)) {
                const QJsonObject& siteObj = activeSitesMap[i];
                QString socketEnableStr = siteObj["socketEnable"].toString("0x00");
                bool ok;
                socketEnable = socketEnableStr.toULongLong(&ok, 0);
                if (!ok) {
                    LOG_MODULE_WARNING("HandlerDevice", QString("buildPdu63Data: Invalid 'socketEnable' format for site %1: '%2'. Defaulting to 0.")
                        .arg(i).arg(socketEnableStr).toStdString());
                    socketEnable = 0;
                }
                LOG_MODULE_WARNING("HandlerDevice", QString("Site %1 not found in DutManager, using JSON config socketEnable: 0x%2")
                    .arg(i).arg(socketEnable, 0, 16).toStdString());
            } else {
                LOG_MODULE_WARNING("HandlerDevice", QString("Site %1 not found in DutManager or JSON config, using default socketEnable: 0")
                    .arg(i).toStdString());
            }
        }

        int offset = (i - 1) * sktBytesPerSite;
        for (int byteIdx = 0; byteIdx < sktBytesPerSite; ++byteIdx) {
            if (offset + byteIdx < siteEnBitmap.size()) {
                siteEnBitmap[offset + byteIdx] = static_cast<char>((socketEnable >> (8 * (sktBytesPerSite - 1 - byteIdx))) & 0xFF);
            }
        }
    }

    QByteArray pduData;
    pduData.append(static_cast<char>(siteCnt));
    pduData.append(static_cast<char>(sktCnt));
    pduData.append(siteEnBitmap);

    LOG_MODULE_INFO("HandlerDevice", QString("Built PDU 0x63 data dynamically (contiguous): SiteCnt=%1, SKTCnt=%2, SiteEnBitmapSize=%3 bytes")
        .arg(siteCnt).arg(sktCnt).arg(siteEnBitmapSize).toStdString());
    LOG_MODULE_DEBUG("HandlerDevice", QString("PDU 0x63 PDATA: %1").arg(QString(pduData.toHex(' ').toUpper())).toStdString());

    return pduData;
}

QString HandlerDevice::buildCmd3Command(const QString& taskPath, const QJsonObject& params)
{
    QStringList cmdParams;
    
    // 参数1: 生产数量 (从params或默认值)
    int productionQuantity = params.value("productionQuantity").toInt(100);
    cmdParams << QString::number(productionQuantity);
    
    // 参数2: 任务文件路径
    cmdParams << taskPath;
    
    // 参数3-5: 供给托盘坐标信息
    QJsonObject supplyTray = params.value("supplyTray").toObject();
    int supplyConfig = supplyTray.value("config").toInt(0);  // 0=默认, 1=手动, 2=自动
    int supplyX = supplyTray.value("x").toInt(0);
    int supplyY = supplyTray.value("y").toInt(0);
    cmdParams << QString::number(supplyConfig) << QString::number(supplyX) << QString::number(supplyY);
    
    // 参数6-8: 生产托盘坐标信息
    QJsonObject productionTray = params.value("productionTray").toObject();
    int productionConfig = productionTray.value("config").toInt(0);
    int productionX = productionTray.value("x").toInt(0);
    int productionY = productionTray.value("y").toInt(0);
    cmdParams << QString::number(productionConfig) << QString::number(productionX) << QString::number(productionY);
    
    // 参数9-11: 拒料托盘坐标信息
    QJsonObject rejectTray = params.value("rejectTray").toObject();
    int rejectConfig = rejectTray.value("config").toInt(0);
    int rejectX = rejectTray.value("x").toInt(0);
    int rejectY = rejectTray.value("y").toInt(0);
    cmdParams << QString::number(rejectConfig) << QString::number(rejectX) << QString::number(rejectY);
    
    // 参数12: 卷带起始位置
    int reelStartPos = params.value("reelStartPosition").toInt(1);
    cmdParams << QString::number(reelStartPos);
    
    // 参数13-19: 7个空参数（保持兼容性）
    for (int i = 0; i < 7; i++) {
        cmdParams << "";
    }
    
    // 构建CMD3字符串：命令码"3" + 逗号分隔的参数
    QString cmd3String = "3," + cmdParams.join(",");
    
    LOG_MODULE_INFO("HandlerDevice", QString("Built CMD3 command: %1").arg(cmd3String).toStdString());
    
    return cmd3String;
}

} // namespace Domain 
