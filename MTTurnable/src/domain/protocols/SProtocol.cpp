#include "domain/protocols/SProtocol.h"
#include "infrastructure/TcpChannel.h"
#include "infrastructure/TcpServerChannel.h"
#include "core/Logger.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDataStream>
#include <QTimer>

namespace Domain::Protocols {

// 协议常量
static const uint16_t PFLAG_SA = 0x5341;  // StdMes → AutoApp
static const uint16_t PFLAG_AS = 0x4153;  // AutoApp → StdMes

SProtocol::SProtocol(QObject *parent)
    : ISProtocol(parent)
    , m_clientChannel(nullptr)
    , m_serverChannel(nullptr)
    , m_vautoHost("127.0.0.1")
    , m_cmd3Port(1000)
    , m_commandPort(64100)
    , m_serverPort(64101)
    , m_connectTimeout(5000)
    , m_isInitialized(false)
    , m_currentVersion(0x01)  // 默认标准StdMes
{
}

SProtocol::~SProtocol()
{
    release();
}

bool SProtocol::initialize(const QJsonObject& config)
{
    LOG_MODULE_INFO("SProtocol", "Starting SProtocol initialization");
    
    if (m_isInitialized) {
        LOG_MODULE_INFO("SProtocol", "SProtocol already initialized");
        return true;
    }
    
    m_config = config;
    
    // 从配置读取参数
    m_vautoHost = config.value("vautoHost").toString("127.0.0.1");
    m_cmd3Port = config.value("cmd3Port").toInt(1000);
    m_commandPort = config.value("commandPort").toInt(64100);
    m_serverPort = config.value("serverPort").toInt(64101);
    m_connectTimeout = config.value("connectTimeout").toInt(5000);
    m_currentVersion = config.value("version").toInt(0x01);

    LOG_MODULE_INFO("SProtocol", QString("Configuration - Host: %1, CMD3Port: %2, CommandPort: %3, ServerPort: %4")
                   .arg(m_vautoHost).arg(m_cmd3Port).arg(m_commandPort).arg(m_serverPort).toStdString());

    // 获取单例通道实例
    m_clientChannel = Infrastructure::TcpChannel::getInstance();
    m_serverChannel = Infrastructure::TcpServerChannel::getInstance();

    if (!m_clientChannel || !m_serverChannel) {
        m_lastError = "Failed to get channel instances";
        LOG_MODULE_ERROR("SProtocol", m_lastError.toStdString());
        return false;
    }
    LOG_MODULE_INFO("SProtocol", "Channel instances obtained successfully");

    // 配置服务器通道
    QVariantMap serverParams;
    serverParams["bindAddress"] = m_vautoHost;
    serverParams["port"] = m_serverPort;
    serverParams["maxConnections"] = 1;
    serverParams["connectionTimeout"] = 30000;
    m_serverChannel->setParameters(serverParams);
    LOG_MODULE_INFO("SProtocol", "Server parameters set");

    // 启动服务器监听，如果默认端口失败则尝试其他端口
    LOG_MODULE_INFO("SProtocol", QString("Attempting to start server on %1:%2").arg(m_vautoHost).arg(m_serverPort).toStdString());
    
    bool serverStarted = false;
    int originalPort = m_serverPort;
    int attemptsCount = 0;
    const int maxAttempts = 10;
    
    for (int attempt = 0; attempt < maxAttempts && !serverStarted; attempt++) {
        serverParams["port"] = m_serverPort + attempt;
        m_serverChannel->setParameters(serverParams);
        
        if (m_serverChannel->open()) {
            serverStarted = true;
            if (attempt > 0) {
                LOG_MODULE_INFO("SProtocol", QString("Server started on alternate port %1 (original %2 was busy)")
                               .arg(m_serverPort + attempt).arg(originalPort).toStdString());
                m_serverPort = m_serverPort + attempt; // 更新实际使用的端口
            }
        } else {
            attemptsCount++;
            LOG_MODULE_WARNING("SProtocol", QString("Port %1 is busy, trying port %2")
                              .arg(m_serverPort + attempt).arg(m_serverPort + attempt + 1).toStdString());
        }
    }
    
    if (!serverStarted) {
        m_lastError = QString("Failed to start server on port %1-%2 - all ports are in use or unavailable")
                     .arg(originalPort).arg(originalPort + maxAttempts - 1);
        LOG_MODULE_ERROR("SProtocol", m_lastError.toStdString());
        LOG_MODULE_ERROR("SProtocol", "Please check that no other instances are running or try restarting the application");
        return false;
    }
    LOG_MODULE_INFO("SProtocol", "Server started successfully");

    // 连接服务器通道信号
    connect(m_serverChannel, &Infrastructure::ICommunicationChannel::dataReceived,
            this, &SProtocol::onServerDataReceived);
    connect(m_serverChannel, &Infrastructure::TcpServerChannel::clientConnected,
            this, &SProtocol::onServerClientConnected);
    connect(m_serverChannel, &Infrastructure::TcpServerChannel::clientDisconnected,
            this, &SProtocol::onServerClientDisconnected);
    connect(m_serverChannel, &Infrastructure::ICommunicationChannel::errorOccurred,
            this, &SProtocol::onServerError);
    LOG_MODULE_INFO("SProtocol", "Server signals connected");

    m_isInitialized = true;
    LOG_MODULE_INFO("SProtocol", QString("S Protocol initialized successfully, server listening on %1:%2")
                   .arg(m_vautoHost).arg(m_serverPort).toStdString());
    
    emit connected();
    return true;
}

void SProtocol::release()
{
    if (m_serverChannel) {
        m_serverChannel->close(); // Close the listening server
        disconnect(m_serverChannel, nullptr, this, nullptr);
    }
    
    // Disconnect client channel signals if they were ever connected
    // This part is less critical as client channels are short-lived.
    if (m_clientChannel) {
        // No persistent signals to disconnect for the singleton client channel
    }

    m_isInitialized = false;
    LOG_MODULE_INFO("SProtocol", "S Protocol released");
    emit disconnected();
}

bool SProtocol::isConnected() const
{
    return m_isInitialized && m_serverChannel && m_serverChannel->isListening();
}

QString SProtocol::getLastError() const
{
    return m_lastError;
}

bool SProtocol::sendCmd3Command(const QString& command)
{
    LOG_MODULE_INFO("SProtocol", QString("Sending CMD3 command to %1:%2: %3")
                   .arg(m_vautoHost).arg(m_cmd3Port).arg(command).toStdString());
    
    if (!m_clientChannel) {
        m_lastError = "Client channel not available";
        LOG_MODULE_ERROR("SProtocol", m_lastError.toStdString());
        return false;
    }

    QByteArray responseData;
    bool success = m_clientChannel->sendToPort(command.toUtf8(), m_cmd3Port, m_vautoHost, m_connectTimeout, &responseData);
    if (!success) {
        m_lastError = QString("Failed to send CMD3 command to %1:%2").arg(m_vautoHost).arg(m_cmd3Port);
        LOG_MODULE_WARNING("SProtocol", QString("CMD3 command send failed - vauto service may not be running").toStdString());
    }

    if (!responseData.isEmpty()) {
        QString responseStr = QString::fromUtf8(responseData).trimmed();
        LOG_MODULE_INFO("SProtocol", QString("Received response for CMD3 command: %1").arg(responseStr).toStdString());
        
        bool ok;
        int responseCode = responseStr.toInt(&ok);
        QString description = "未知响应码";
        if (ok) {
            switch(responseCode) {
                case 0: description = "数据传送OK"; break;
                case 1: description = "烧录机未Ready (vauto尚未收到PDU 0x63)"; break;
                case 2: description = "设备自动运行中"; break;
                case 3: description = "前次通信处理进行中"; break;
                case 4: description = "当前模式为非一键烧录模式"; break;
                case 5: description = "任务数据不正确"; break;
                case 6: description = "托盘/卷带指定位置不正确"; break;
                case 7: description = "烧录器未初始化成功(CONN)"; break;
                case 8: description = "Lic验证失败"; break;
                case 99: description = "命令格式错误 (参数不为20个)"; break;
                default: description = "未知的成功或失败代码"; break;
            }
        }
        LOG_MODULE_INFO("SProtocol", QString("  解析: %1").arg(description).toStdString());
    }

    return success;
}

bool SProtocol::sendPduCommand(uint16_t pflag, uint8_t pdu, const QByteArray& data)
{
    QByteArray packet = buildPduPacket(pflag, pdu, data);
    
    // 详细的PDU帧数据日志
    LOG_MODULE_INFO("SProtocol", QString("Sending PDU command: PFLAG=0x%1, PDU=0x%2 to %3:%4")
                   .arg(pflag, 4, 16, QChar('0')).arg(pdu, 2, 16, QChar('0'))
                   .arg(m_vautoHost).arg(m_commandPort).toStdString());
    
    // 输出完整的帧数据
    QString frameHex = formatFrameData(packet);
    LOG_MODULE_INFO("SProtocol", QString("TX Frame[%1 bytes]: %2")
                   .arg(packet.size()).arg(frameHex).toStdString());
    
    // 对于特定PDU提供协议解析
    logPduDetails(packet, true);
    
    if (!m_clientChannel) {
        m_lastError = "Client channel not available";
        LOG_MODULE_ERROR("SProtocol", m_lastError.toStdString());
        return false;
    }

    QByteArray responseData;
    bool success = m_clientChannel->sendToPort(packet, m_commandPort, m_vautoHost, m_connectTimeout, &responseData);
    if (!success) {
        m_lastError = QString("Failed to send PDU command to %1:%2").arg(m_vautoHost).arg(m_commandPort);
        LOG_MODULE_WARNING("SProtocol", QString("PDU command send failed - target may not be running").toStdString());
    }

    if (!responseData.isEmpty()) {
        LOG_MODULE_INFO("SProtocol", "--- Received ACK/Response for PDU command ---");
        logPduDetails(responseData, false);
    }

    return success;
}

bool SProtocol::sendAxisMoveCommand(const QJsonObject& command)
{
    // 构建严格符合协议规范的JSON格式 - 使用紧凑格式
    QJsonObject axisCommand;
    axisCommand["Command"] = "AxisMove";
    axisCommand["AxisSelect"] = command["AxisSelect"].toString();
    axisCommand["SiteIdx"] = command["SiteIdx"].toString();
    axisCommand["TargetAngle"] = command["TargetAngle"].toString();
    
    // 使用紧凑JSON格式，减少数据大小
    QJsonDocument doc(axisCommand);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    
    LOG_MODULE_INFO("SProtocol", QString("Sending AxisMove command to %1:%2 (size: %3 bytes)")
                   .arg(m_vautoHost).arg(m_commandPort).arg(jsonData.size()).toStdString());
    LOG_MODULE_INFO("SProtocol", QString("AxisMove JSON: %1")
                   .arg(QString::fromUtf8(jsonData)).toStdString());
    
    if (!m_clientChannel) {
        m_lastError = "Client channel not available";
        LOG_MODULE_ERROR("SProtocol", m_lastError.toStdString());
        return false;
    }

    QByteArray responseData;
    bool success = m_clientChannel->sendToPort(jsonData, m_commandPort, m_vautoHost, m_connectTimeout, &responseData);
    if (!success) {
        m_lastError = QString("Failed to send AxisMove command to %1:%2").arg(m_vautoHost).arg(m_commandPort);
        LOG_MODULE_WARNING("SProtocol", QString("AxisMove command send failed - vauto may not be running").toStdString());
    } else {
        LOG_MODULE_INFO("SProtocol", QString("AxisMove command sent successfully").toStdString());
    }

    if (!responseData.isEmpty()) {
        LOG_MODULE_INFO("SProtocol", "--- Received Response for AxisMove command ---");
        tryParseJsonCommand(responseData);
    }

    return success;
}

bool SProtocol::sendJsonCommand(const QJsonObject& command)
{
    QJsonDocument doc(command);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    LOG_MODULE_INFO("SProtocol", QString("Sending JSON command: %1")
                   .arg(QString::fromUtf8(jsonData)).toStdString());
    
    if (!m_clientChannel) {
        m_lastError = "Client channel not available";
        LOG_MODULE_ERROR("SProtocol", m_lastError.toStdString());
        return false;
    }

    return m_clientChannel->sendToPort(jsonData, m_commandPort, m_vautoHost, m_connectTimeout);
}

bool SProtocol::sendResponse(const QByteArray& response)
{
    if (!m_serverChannel) {
        m_lastError = "Server channel not available";
        LOG_MODULE_ERROR("SProtocol", m_lastError.toStdString());
        return false;
    }
    
    qint64 bytesWritten = m_serverChannel->send(response);
    
    if (bytesWritten > 0) {
        LOG_MODULE_INFO("SProtocol", QString("Response sent: %1 bytes").arg(bytesWritten).toStdString());
        return true;
    } else {
        m_lastError = "Failed to send response";
        LOG_MODULE_ERROR("SProtocol", m_lastError.toStdString());
        return false;
    }
}

bool SProtocol::sendJsonResponse(const QJsonObject& response)
{
    QJsonDocument doc(response);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    LOG_MODULE_INFO("SProtocol", QString("Sending JSON response: %1")
                   .arg(QString::fromUtf8(jsonData)).toStdString());
    return sendResponse(jsonData);
}

bool SProtocol::sendPduResponse(uint16_t pflag, uint8_t pdu, const QByteArray& data)
{
    QByteArray packet = buildPduPacket(pflag, pdu, data);
    LOG_MODULE_INFO("SProtocol", QString("Sending PDU response: PFLAG=0x%1, PDU=0x%2")
                   .arg(pflag, 4, 16, QChar('0')).arg(pdu, 2, 16, QChar('0')).toStdString());
    return sendResponse(packet);
}

void SProtocol::processReceivedData(const QByteArray& data)
{
    LOG_MODULE_DEBUG("SProtocol", (QString("Processing received data: %1 bytes,data: %2")
                    .arg(data.size()).arg(QString::fromLatin1(data.toHex(' ').toUpper()))).toStdString());

    emit rawDataReceived(data);

    // 尝试解析为JSON命令
    LOG_MODULE_DEBUG("SProtocol", "Attempting to parse as JSON command...");
    if (tryParseJsonCommand(data)) {
        LOG_MODULE_DEBUG("SProtocol", "Successfully parsed as JSON command.");
        return;
    }

    // 尝试解析为PDU命令
    LOG_MODULE_DEBUG("SProtocol", "Attempting to parse as PDU command...");
    if (tryParsePduCommand(data)) {
        LOG_MODULE_DEBUG("SProtocol", "Successfully parsed as PDU command.");
        return;
    }

    LOG_MODULE_WARNING("SProtocol", "Received data does not match any known format");
}

bool SProtocol::tryParseJsonCommand(const QByteArray& data)
{
    QByteArray jsonData = data.trimmed();
    
    // 轴移动命令直接以JSON格式传输，不需要CMD4前缀
        if (!jsonData.startsWith('{')) {
            return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
    
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        LOG_MODULE_WARNING("SProtocol", QString("JSON parse error: %1 at offset %2")
                        .arg(error.errorString()).arg(error.offset).toStdString());
        return false;
    }
    
    QJsonObject command = doc.object();
    LOG_MODULE_INFO("SProtocol", QString("RX <- Received JSON command: %1")
                   .arg(QString::fromUtf8(jsonData)).toStdString());
    
    emit jsonCommandReceived(command);
    
    // 具体的JSON命令处理
    QString cmdName = command["Command"].toString();
    if (cmdName == "AxisMove") {
        // 处理轴移动响应
        handleAxisMoveResponse(command);
    } else if (cmdName == "AxisMoveComplete") {
        // 处理轴移动完成通知
        handleAxisMoveComplete(command);
    } else if (cmdName == "ProductInfo") {
        // 处理productInfo指令
        handleProductInfo(command);
    }
    
    return true;
}

bool SProtocol::tryParsePduCommand(const QByteArray& data)
{
    if (data.size() < 5) { // 最小PDU包长度：PFLAG(2) + PDU(1) + LEN(1) + CRC(1)
        return false;
    }
    
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);
    
    uint16_t pflag;
    uint8_t pdu;
    uint8_t dataLen;
    
    stream >> pflag >> pdu >> dataLen;

    // 验证PFLAG
    if (pflag != PFLAG_SA && pflag != PFLAG_AS) {
        LOG_MODULE_DEBUG("SProtocol", QString("Invalid PFLAG: 0x%1, expected SA(0x5341) or AS(0x4153)")
                        .arg(pflag, 4, 16, QChar('0')).toStdString());
    return false;
}

    // 验证包长度
    int expectedSize = 4 + dataLen + 1; // 头部(4) + 数据 + CRC(1)
    if (data.size() != expectedSize) {
        LOG_MODULE_DEBUG("SProtocol", QString("Invalid packet size: %1, expected %2")
                        .arg(data.size()).arg(expectedSize).toStdString());
        return false;
    }

    // 提取数据部分
    QByteArray pduData = data.mid(4, dataLen);
    
    // CRC验证
    uint8_t expectedCrc = calculateCRC(data.left(data.size() - 1));
    uint8_t actualCrc = static_cast<uint8_t>(data[data.size() - 1]);
    
    LOG_MODULE_DEBUG("SProtocol", QString("CRC validation: Expected=0x%1, Actual=0x%2")
                    .arg(expectedCrc, 2, 16, QChar('0')).arg(actualCrc, 2, 16, QChar('0')).toStdString());
    
    if (expectedCrc != actualCrc) {
        LOG_MODULE_WARNING("SProtocol", QString("PDU CRC check failed! Expected=0x%1, Actual=0x%2, Data=%3")
                          .arg(expectedCrc, 2, 16, QChar('0'))
                          .arg(actualCrc, 2, 16, QChar('0'))
                          .arg(formatFrameData(data)).toStdString());
        // CRC错误时也应该继续尝试处理，让上层决定如何应对
    }

    LOG_MODULE_INFO("SProtocol", QString("RX <- Received PDU command: PFLAG=0x%1, PDU=0x%2, DataLen=%3")
                   .arg(pflag, 4, 16, QChar('0')).arg(pdu, 2, 16, QChar('0')).arg(dataLen).toStdString());
    
    // 输出完整的接收帧数据
    QString frameHex = formatFrameData(data);
    LOG_MODULE_INFO("SProtocol", QString("RX Frame[%1 bytes]: %2")
                   .arg(data.size()).arg(frameHex).toStdString());
    
    // 对于特定PDU提供协议解析
    logPduDetails(data, false);
    
    emit pduCommandReceived(pflag, pdu, pduData);
    
    // 具体的PDU命令处理
    if (pflag == PFLAG_AS) { // AutoApp发送的命令
        switch (pdu) {
            case PDU_QUERYVERSION:
                handleQueryVersion(pduData);
                break;
            case PDU_TELLCHIPSEN:
                handleTellChipSen(pduData);
                break;
            case PDU_QUERYICSTATUS:
                handleQueryICStatus(pduData);
                break;
            case PDU_QUERYREMAINING:
                handleQueryRemaining(pduData);
                break;
            case PDU_TELLSITEEN:
                handleTellSiteEn(pduData);
                break;
            default:
                LOG_MODULE_WARNING("SProtocol", QString("Unknown PDU command: 0x%1").arg(pdu, 2, 16, QChar('0')).toStdString());
                break;
        }
    }
    
    return true;
}

QByteArray SProtocol::buildPduPacket(uint16_t pflag, uint8_t pdu, const QByteArray& data)
{
    QByteArray packet;
    
    // 【协议标准化修复】手动构建包，确保字节序正确
    // PFLAG(2字节) - 按协议规定的字节序
    packet.append(static_cast<char>((pflag >> 8) & 0xFF));  // 高字节
    packet.append(static_cast<char>(pflag & 0xFF));         // 低字节
    
    // PDU(1字节)
    packet.append(static_cast<char>(pdu));
    
    // PLEN(1字节) - 数据长度
    packet.append(static_cast<char>(data.size()));
    
    // PDATA(N字节) - 实际数据
    packet.append(data);
    
    // 【关键修复】CRC计算：从PFLAG到PDATA结束的所有字节累加和（4+N字节）
    uint8_t crc = calculateCRC(packet);  // packet现在包含PFLAG+PDU+PLEN+PDATA
    packet.append(static_cast<char>(crc));
    
    // 验证构建的包格式
    LOG_MODULE_DEBUG("SProtocol", QString("Built PDU packet: PFLAG=0x%1, PDU=0x%2, PLEN=%3, DataLen=%4, CRC=0x%5")
                    .arg(pflag, 4, 16, QChar('0'))
                    .arg(pdu, 2, 16, QChar('0'))
                    .arg(data.size())
                    .arg(data.size())
                    .arg(crc, 2, 16, QChar('0')).toStdString());
    
    return packet;
}

uint8_t SProtocol::calculateCRC(const QByteArray& data)
{
    uint8_t crc = 0;
    for (char byte : data) {
        crc += static_cast<uint8_t>(byte);
    }
    return crc;
}

// PDU业务处理方法实现

void SProtocol::handleQueryVersion(const QByteArray& data)
{
    Q_UNUSED(data);
    LOG_MODULE_INFO("SProtocol", "Received version query (0xE1)");
    emit versionQueryReceived();
    
    // 发送ACK
    sendAck(PDU_QUERYVERSION);
    
    // 发送版本信息 (PDU 0x61)
    QByteArray versionData;
    versionData.append(static_cast<char>(m_currentVersion));
    sendPduResponse(PFLAG_SA, PDU_TELLVERSION, versionData);
}

void SProtocol::handleTellChipSen(const QByteArray& data)
{
    if (data.size() < 1) {
        LOG_MODULE_ERROR("SProtocol", "Invalid TellChipSen data size");
        sendAck(PDU_TELLCHIPSEN, 0x01); // 错误
        return;
    }
    
        uint8_t siteIdx = static_cast<uint8_t>(data[0]);
    QByteArray chipData = data.mid(1);
    
    LOG_MODULE_INFO("SProtocol", QString("Received chip placement (0xE6): Site=%1, DataLen=%2")
                   .arg(siteIdx).arg(chipData.size()).toStdString());
    
    emit chipPlacementReceived(siteIdx, chipData);
    
    // 发送ACK
    sendAck(PDU_TELLCHIPSEN);
}

void SProtocol::handleQueryICStatus(const QByteArray& data)
{
    if (data.size() < 1) {
        LOG_MODULE_ERROR("SProtocol", "Invalid QueryICStatus data size");
        sendAck(PDU_QUERYICSTATUS, 0x01);
        return;
    }
    
    uint8_t siteIdx = static_cast<uint8_t>(data[0]);
    QByteArray checkData = data.mid(1);
    
    LOG_MODULE_INFO("SProtocol", QString("Received IC status check request (0xE8): Site=%1")
                   .arg(siteIdx).toStdString());
    
    emit icStatusCheckRequested(siteIdx, checkData);
    
    // 发送ACK
    sendAck(PDU_QUERYICSTATUS);
}

void SProtocol::handleQueryRemaining(const QByteArray& data)
{
    if (data.size() < 1) {
        LOG_MODULE_ERROR("SProtocol", "Invalid QueryRemaining data size");
        sendAck(PDU_QUERYREMAINING, 0x01);
        return;
    }
    
    uint8_t siteIdx = static_cast<uint8_t>(data[0]);
    QByteArray checkData = data.mid(1);
    
    LOG_MODULE_INFO("SProtocol", QString("Received remaining check request (0xE5): Site=%1")
                   .arg(siteIdx).toStdString());
    
    emit remainingCheckRequested(siteIdx, checkData);
    
    // 发送ACK
    sendAck(PDU_QUERYREMAINING);
}

void SProtocol::handleTellSiteEn(const QByteArray& data)
{
    LOG_MODULE_INFO("SProtocol", QString("Received site enable (0xE4): DataLen=%1")
                   .arg(data.size()).toStdString());
    
    emit siteEnableReceived(data);
    
    // 发送ACK
    sendAck(PDU_TELLSITEEN);
}

// JSON业务处理方法实现

void SProtocol::handleAxisMoveResponse(const QJsonObject& response)
{
    int result = response["Result"].toString().toInt();
    QString errMsg = response["ErrMsg"].toString();
    QString errMsg1;
    switch (result) {
           case 0: errMsg1 = ""; break;
           case 1: errMsg1 = "转台回原点未完成"; break;
           case 2: errMsg1 = "转台移动目标位置超出软极限"; break;
           case 3: errMsg1 = "转台轴控制错误"; break;
           case 4: errMsg1 = "转台未进入测试状态,不可移动"; break;
           case 5: errMsg1 = "转台运动中,不可移动"; break;
           default: errMsg1 = QString("未知的轴移动错误,代码: %1").arg(result); break;
       }

    LOG_MODULE_INFO("SProtocol", QString("Received AxisMove response: Result=%1, ErrMsg=%2,, ResultInfo=%3")
                   .arg(result).arg(errMsg).arg(errMsg1).toStdString());
    
    // 轴移动响应不触发具体的业务逻辑，主要用于确认命令接收状态
    if (result != 0) {
        LOG_MODULE_WARNING("SProtocol", QString("AxisMove command failed with result=%1, error=%2")
                          .arg(result).arg(errMsg).toStdString());
    }
}

void SProtocol::handleAxisMoveCommand(const QJsonObject& command)
{
    QString axisSelect = command["AxisSelect"].toString();
    int siteIdx = command["SiteIdx"].toString().toInt();
    int targetAngle = command["TargetAngle"].toString().toInt();
    
    LOG_MODULE_INFO("SProtocol", QString("Received AxisMove command: Axis=%1, Site=%2, Target=%3")
                   .arg(axisSelect).arg(siteIdx).arg(targetAngle).toStdString());
    
    emit axisMovementRequested(axisSelect, siteIdx, targetAngle);
}

void SProtocol::handleAxisMoveComplete(const QJsonObject& command)
{
    QString axisSelect = command["AxisSelect"].toString();
    int siteIdx = command["SiteIdx"].toString().toInt();
    int currentAngle = command["CurrentAngle"].toString().toInt();
    int result = command["Result"].toString().toInt();
    
    LOG_MODULE_INFO("SProtocol", QString("Received AxisMoveComplete: Axis=%1, Site=%2, Current=%3, Result=%4")
                   .arg(axisSelect).arg(siteIdx).arg(currentAngle).arg(result).toStdString());
    
    emit axisMovementCompleted(axisSelect, siteIdx, currentAngle, result);

    // 【协议修复】根据协议，收到AxisMoveComplete后需要回复ACK
    QJsonObject response;
    response["Command"] = "AxisMoveComplete";
    response["Result"] = "0";
    response["ErrMsg"] = "";
    LOG_MODULE_INFO("SProtocol", QString("TX -> Sending AxisMoveComplete ACK: %1").arg(QString(QJsonDocument(response).toJson(QJsonDocument::Compact))).toStdString());
    sendJsonResponse(response);
}
void SProtocol::handleProductInfo(const QJsonObject &command)
{
    QJsonArray rotateInfo = command["RotateInfo"].toArray();

    LOG_MODULE_INFO("SProtocol", QString("Received ProductInfo: RotateInfo contains %1 items")
                   .arg(rotateInfo.size()).toStdString());

    emit productInfoReceived(rotateInfo);

    // 根据协议，收到ProductInfo后需要回复ACK
    QJsonObject response;
    response["Command"] = "ProductInfo";
    response["Result"] = "0";
    response["ErrMsg"] = "";
    LOG_MODULE_INFO("SProtocol", QString("TX -> Sending ProductInfo ACK: %1").arg(QString(QJsonDocument(response).toJson(QJsonDocument::Compact))).toStdString());
    sendJsonResponse(response);
}

void SProtocol::sendAck(uint8_t pdu, uint8_t errorCode)
{
    QByteArray ackData;
    ackData.append(static_cast<char>(errorCode));
    
    // 通过服务器端口发送ACK响应
    sendPduResponse(PFLAG_SA, pdu, ackData);
    
    LOG_MODULE_DEBUG("SProtocol", QString("Sent ACK for PDU 0x%1, ErrorCode=%2")
                    .arg(pdu, 2, 16, QChar('0')).arg(errorCode).toStdString());
}

// 服务器通道事件处理

void SProtocol::onServerDataReceived(const QByteArray& data)
{
    processReceivedData(data);
}

void SProtocol::onServerClientConnected(const QString& clientInfo)
{
    LOG_MODULE_INFO("SProtocol", QString("Client connected: %1").arg(clientInfo).toStdString());
    emit connected();
}

void SProtocol::onServerClientDisconnected(const QString& clientInfo)
{
    LOG_MODULE_INFO("SProtocol", QString("Client disconnected: %1").arg(clientInfo).toStdString());
    // 不发射disconnected信号，因为服务器仍在监听
}

void SProtocol::onServerError(const QString& error)
{
    m_lastError = error;
    LOG_MODULE_ERROR("SProtocol", QString("Server error: %1").arg(error).toStdString());
    emit errorOccurred(error);
}

// 调试日志辅助方法实现
void SProtocol::logPduDetails(const QByteArray& packet, bool isSending)
{
    if (packet.size() < 5) return;

    QString direction = isSending ? "TX" : "RX";
    uint16_t pflag = (static_cast<uint8_t>(packet[0]) << 8) | static_cast<uint8_t>(packet[1]);
    uint8_t pdu = static_cast<uint8_t>(packet[2]);
    uint8_t plen = static_cast<uint8_t>(packet[3]);
    QByteArray pdata = packet.mid(4, plen);
    uint8_t pcrc = static_cast<uint8_t>(packet.back());

    QString pflagName = (pflag == PFLAG_SA) ? "SA(StdMes->AutoApp)" : "AS(AutoApp->StdMes)";

    LOG_MODULE_INFO("SProtocol", QString("--- PDU Detailed Analysis [%1] ---").arg(direction).toStdString());
    LOG_MODULE_INFO("SProtocol", QString("  PFLAG: 0x%1 (%2)").arg(QString::number(pflag, 16).toUpper()).arg(pflagName).toStdString());
    LOG_MODULE_INFO("SProtocol", QString("  PDU  : 0x%1").arg(QString::number(pdu, 16).toUpper()).toStdString());
    LOG_MODULE_INFO("SProtocol", QString("  PLEN : 0x%1 (%2 bytes)").arg(QString::number(plen, 16).toUpper()).arg(plen).toStdString());
    
    if (pdu == PDU_TELLDEVINIT) { // PDU 0x63
        if (isSending) { // TX 0x63 Request from this app
            LOG_MODULE_INFO("SProtocol", QString("  功能: StdMes告知AutoApp初始化情况").toStdString());
            if (pdata.size() >= 2) {
                uint8_t siteCnt = static_cast<uint8_t>(pdata[0]);
                uint8_t sktCnt = static_cast<uint8_t>(pdata[1]);
                LOG_MODULE_INFO("SProtocol", QString("  PDATA: SiteCnt=%1, SKTCnt=%2").arg(siteCnt).arg(sktCnt).toStdString());
                QByteArray bitmap = pdata.mid(2);
                int sktBytesPerSite = (sktCnt > 0) ? (sktCnt / 8) : 0;
                for (int i = 0; i < siteCnt; ++i) {
                    if (sktBytesPerSite > 0 && (i * sktBytesPerSite < bitmap.size())) {
                        QByteArray siteBitmap = bitmap.mid(i * sktBytesPerSite, sktBytesPerSite);
                         LOG_MODULE_INFO("SProtocol", QString("    - Site %1 Enable: %2").arg(i + 1).arg(formatFrameData(siteBitmap)).toStdString());
                    }
                }
            }
        } else { // RX 0x63 ACK from vauto
            LOG_MODULE_INFO("SProtocol", QString("  功能: AutoApp对0x63的ACK应答").toStdString());
            if (pdata.size() >= 1) {
                uint8_t errCode = static_cast<uint8_t>(pdata[0]);
                QString errMsg = (errCode == 0) ? "成功接收" : "接收失败";
                LOG_MODULE_INFO("SProtocol", QString("  PDATA: ErrCode=0x%1 (%2)").arg(QString::number(errCode, 16).toUpper()).arg(errMsg).toStdString());
            }
        }
    } else {
        LOG_MODULE_INFO("SProtocol", QString("  PDATA: %1").arg(formatFrameData(pdata)).toStdString());
    }
    
    LOG_MODULE_INFO("SProtocol", QString("  PCRC : 0x%1").arg(QString::number(pcrc, 16).toUpper()).toStdString());
    LOG_MODULE_INFO("SProtocol", QString("--- End of Analysis ---").toStdString());
}


QString SProtocol::formatFrameData(const QByteArray& data)
{
    QString result;
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0) result += " ";
        result += QString("%1").arg(static_cast<uint8_t>(data[i]), 2, 16, QChar('0')).toUpper();
    }
    return result;
}

} // namespace Domain::Protocols 
