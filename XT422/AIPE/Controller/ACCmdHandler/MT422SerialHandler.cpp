#include "MT422SerialHandler.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include "CustomMessageHandler.h"
#include "AngkLogger.h"
#include <QMap>
#include "CRC/Crc32_Std.h"
#include<QTimer>

// 静态成员变量定义
MT422SerialHandler* MT422SerialHandler::s_instance = nullptr;
QMutex MT422SerialHandler::s_instanceMutex;

// 单例获取方法实现
MT422SerialHandler& MT422SerialHandler::instance()
{
    // 双重检查锁定模式
    if (s_instance == nullptr) {
        QMutexLocker locker(&s_instanceMutex);
        if (s_instance == nullptr) {
            s_instance = new MT422SerialHandler();
        }
    }
    return *s_instance;
}

// 销毁单例实例
void MT422SerialHandler::destroyInstance()
{
    QMutexLocker locker(&s_instanceMutex);
    if (s_instance != nullptr) {
        delete s_instance;
        s_instance = nullptr;
    }
}

// SerialPortReader implementation
SerialPortReader::SerialPortReader(quint8 portIndex, QString portName,QObject* parent)
    : QThread(parent)
    , m_port(nullptr)
    , m_portIndex(portIndex)
    , m_running(true)
    , m_portName(portName)
{
    m_port = new QSerialPort(this);

    // 配置串口参数
    m_port->setPortName(m_portName);
    m_port->setBaudRate(921600);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::OneStop);
    // 设置读取超时为100ms
    //m_port->setReadBufferSize(1024);
    m_port->open(QIODevice::ReadWrite);
    if (!m_port || !m_port->isOpen()) {
        emit error(QString("Port %1 is not open").arg(m_portIndex));
        ALOG_INFO("Port %d is not open,%s", "CU", "--",
            m_portIndex, m_portName.toStdString().c_str());
        return;
    }
    m_port->moveToThread(this);
    
}
// SerialPortReader.cpp
// SerialPortReader.cpp
void SerialPortReader::handleWriteRequest(quint8 portIndex, const QByteArray& data) {
    if (portIndex != m_portIndex || !m_port || !m_port->isOpen()) {
       // ALOG_ERROR("Invalid port %d or not open", "CU", "--", portIndex);
        return;
    }

    qint64 bytesWritten = m_port->write(data);
    ALOG_INFO("Write on port %d ", "CU", "--",
        portIndex);
    if (bytesWritten == -1) {
        ALOG_ERROR("Write failed on port %d: %s", "CU", "--",
            portIndex, m_port->errorString().toStdString().c_str());
    }
    // 完全异步，不等待写入完成
}
void SerialPortReader::stop()
{
    QMutexLocker locker(&m_mutex);
    m_running = false;
}

void SerialPortReader::run()
{
    
    // 主线程返回的 port 可能属于主线程，需在子线程重新创建
    //m_port = handler->createAndMovePort(m_portIndex, this); // 新增方法
    
    QByteArray buffer;
    const int maxSize = 1024;
    buffer.reserve(maxSize);

    while (m_running) {
        // 等待数据可读
        //if (m_port->waitForReadyRead(100)) {
            //ALOG_INFO("---------waitForReadyRead--------", "CU", "--");
            QByteArray data = m_port->readAll();
            //if (!data.isEmpty()) {
                //ALOG_INFO("---------data.isNOTEmpty--------", "CU", "--");
                // 构造消息头
                QByteArray message;

                // 1. 添加MT422标识(4字节)
                message.append((char*)&MT422SerialHandler::MT422_TAG_ID, 4);

                // 2. 添加数据长度(4字节)
                quint32 length = data.size();
                message.append((char*)&length, 4);

                // 3. 计算并添加CRC32(4字节)
                CCrc32Std crc32;
                crc32.ReInit();
                crc32.CalcSubRoutine((uint8_t*)data.data(), data.size());
                quint32 checksum = 0;
                crc32.GetChecksum((uint8_t*)&checksum, sizeof(checksum));
                message.append((char*)&checksum, 4);

                // 4. 添加实际数据
                message.append(data);

                // 从配置中获取对应的 strIPHop 和 BPUID
                QString strIPHop;
                uint16_t bpuid = 0;
                quint8 index = 0;
                MT422SerialHandler* handler = qobject_cast<MT422SerialHandler*>(parent());
                if (handler) {
                    handler->getPortMapping(m_portIndex, strIPHop, bpuid, index);
                }

                // 发送完整消息
                emit dataReceived(strIPHop, bpuid, index, message);

                // 输出日志，显示前8个字节的十六进制
                QString hexData = data.left(8).toHex().toUpper();
                QString hexOutput;
                for (int i = 0; i < hexData.length(); i += 2) {
                    hexOutput += hexData.mid(i, 2) + " ";
                }
                ALOG_INFO("---------Received data from port %d --------", "CU", "--",
                    m_portIndex);
            //}
       // }
    }
}
std::shared_ptr<QSerialPort> MT422SerialHandler::createAndMovePort(quint8 portIndex, QThread* targetThread) {
    auto port = std::make_shared<QSerialPort>();
    // 配置串口参数（同原有逻辑）
    port->moveToThread(targetThread);

    // 在目标线程中打开串口
    QMetaObject::invokeMethod(port.get(), [port, portIndex]() {
        if (!port->open(QIODevice::ReadWrite)) {
            ALOG_ERROR("Failed to open port %d in thread", "CU", "--", portIndex);
        }
        }, Qt::BlockingQueuedConnection);

    return port;
}
// MT422SerialHandler implementation
MT422SerialHandler::MT422SerialHandler(QObject* parent)
    : QObject(parent)
{
    // 获取应用程序目录
    QString appDir = QCoreApplication::applicationDirPath();
    //QString configPath = appDir + "/ACCmdHandler/422com.json";
    QString configPath = appDir + "/422com.json";
    // 尝试加载配置文件
    m_serialportmanager = new SerialPortManager;
    if (!loadPortConfiguration(configPath)) {
        ALOG_ERROR("Failed to load port configuration from %s", "CU", "--", configPath.toStdString().c_str());
    }
    else {
        ALOG_INFO("Successfully loaded port configuration from %s", "CU", "--", configPath.toStdString().c_str());
    }
    // 连接信号到CustomMessageHandler的MT422专用处理函数
    connect(this, &MT422SerialHandler::messageReceived,
        CustomMessageHandler::instance(), &CustomMessageHandler::OnMT422Response);
    connect(m_serialportmanager,&SerialPortManager::dataReceived,this,&MT422SerialHandler::receiveMessage);
}

MT422SerialHandler::~MT422SerialHandler()
{
    // 断开信号连接
    disconnect(this, &MT422SerialHandler::messageReceived,
        CustomMessageHandler::instance(), &CustomMessageHandler::OnMT422Response);

    // 停止所有读取线程
    /*for (auto reader : m_portReaders) {
        reader->stop();
        reader->wait();
        delete reader;
    }
    m_portReaders.clear();*/

    closeAllPorts();
}

bool MT422SerialHandler::loadPortConfiguration(const QString& configFile)
{
    QFile file(configFile);
    if (!file.open(QIODevice::ReadOnly)) {
        ALOG_ERROR("Failed to open configuration file: %s", "CU", "--", configFile.toStdString().c_str());
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        ALOG_ERROR("Failed to parse JSON configuration: %s", "CU", "--", parseError.errorString().toStdString().c_str());
        return false;
    }

    QJsonObject root = doc.object();
    int baudRate = root["baudRate"].toInt(); 
    jsonConfigBaudrate = (uint32_t)baudRate;
    ALOG_INFO("Success load jsonConfigbaudRate:%d !","CU","--", jsonConfigBaudrate);
    QJsonObject portsConfig = root["ports"].toObject();

    // 清除旧的配置
    m_portConfig.clear();
    m_strIPHopBpuIndexToPortMap.clear();

    // 遍历所有端口配置
    int i = 0;
    for (auto it = portsConfig.begin(); it != portsConfig.end(); ++it) {
        
        QJsonObject portInfo = it.value().toObject();
        quint8 portIndex = it.key().toInt();
        QString portName = portInfo["port"].toString();
        QString strIPHop = portInfo["strIPHop"].toString();
        uint16_t bpu = portInfo["bpu"].toInt();
        quint8 index = portInfo["index"].toInt();

        // 保存端口配置
        portInfo["baudRate"] = baudRate;  // 添加统一的波特率
        m_portConfig[QString::number(portIndex)] = portInfo;

        // 建立映射关系
        QString key = QString("%1_%2_%3").arg(strIPHop).arg(bpu).arg(index);
        m_strIPHopBpuIndexToPortMap[key] = portIndex;
        // 配置串口
        i++;
        if (!configurePort(i,portIndex, portName, baudRate)) {
            ALOG_WARN("Failed to configure port %d (%s)", "CU", "--", portIndex, portName.toStdString().c_str());
            continue;
        }

        ALOG_INFO("Mapped port %d to IPHop: %s, BPU: %d, Index: %d", "CU", "--",
            portIndex, strIPHop.toStdString().c_str(), bpu, index);
    }

    return true;
}

void MT422SerialHandler::handleE(const QString& portName, const QByteArray& data) {
    handleReceivedData("192.168.70.109:0", 1, 3, data);
}
bool MT422SerialHandler::configurePort(quint8 portNum,quint8 portIndex, const QString& portName, int baudRate)
{
    if (portNum > MAX_PORTS) {
        ALOG_ERROR("PortNum:%d Port index %d exceeds maximum allowed ports (%d)", "CU", "--",portNum, portIndex, MAX_PORTS);
        return false;
    }
    m_serialportmanager->connectPort(portName,portIndex,baudRate, QSerialPort::Data8, QSerialPort::NoParity, QSerialPort::OneStop);
    ALOG_INFO("Successfully configured port %d (%s)", "CU", "--", portIndex, portName.toStdString().c_str());
    return true;
}

void MT422SerialHandler::closePort(quint8 portIndex)
{
    // 停止读取线程
    /*auto readerIt = m_portReaders.find(portIndex);
    if (readerIt != m_portReaders.end()) {
        readerIt.value()->stop();
        readerIt.value()->wait();
        delete readerIt.value();
        m_portReaders.remove(portIndex);
    }*/

    // 关闭串口
    auto it = m_serialPorts.find(portIndex);
    if (it != m_serialPorts.end()) {
        if (it.value()->isOpen()) {
            it.value()->close();
        }
        m_serialPorts.remove(portIndex);
        ALOG_DEBUG("Closed port %d", "CU", "--", portIndex);
    }
}

void MT422SerialHandler::closeAllPorts()
{
    for (auto it = m_serialPorts.begin(); it != m_serialPorts.end(); ++it) {
        if (it.value()->isOpen()) {
            it.value()->close();
        }
    }
    m_serialPorts.clear();
    ALOG_INFO("Closed all ports", "CU", "--");
}

bool MT422SerialHandler::isPortOpen(quint8 portIndex) const
{
    auto it = m_serialPorts.find(portIndex);
    return it != m_serialPorts.end() && it.value()->isOpen();
}

bool MT422SerialHandler::sendMessage(const QString& strIPHop, const uint16_t BPUID, quint8 index, const QByteArray& message)
{
    // 获取从上一阶段传递的traceId
    quint64 traceId = 0;
    QVariant traceVar = property("currentTraceId");
    if (traceVar.isValid()) {
        traceId = traceVar.toULongLong();
    }
    
    if (message.isEmpty()) {
        ALOG_ERROR("Empty message received", "CU", "--");
        return false;
    }
    
    // 查找匹配的端口配置
    QString key = QString("%1:0_%2_%3").arg(strIPHop).arg(BPUID).arg(index);
    if (!m_strIPHopBpuIndexToPortMap.contains(key)) {
        return false;
    }
    
    quint8 portIndex = m_strIPHopBpuIndexToPortMap[key];
    QString portName = QString("COM%1").arg(portIndex);
    

    
    QByteArray data = message.mid(0);
    // 日志输出前8个字节的十六进制
    QString hexData = data.left(8).toHex().toUpper();
    QString hexOutput;
    for (int i = 0; i < hexData.length(); i += 2) {
        hexOutput += hexData.mid(i, 2) + " ";
    } 
    
    bool success = true;
    //到这为止是没有性能瓶颈的
    
    // 将traceId传递给SerialPortManager
    m_serialportmanager->setProperty("currentTraceId", QVariant::fromValue(traceId));
    
    m_serialportmanager->sendData(portName, data);
    //emit messageSent(success);
    return success;
}
bool MT422SerialHandler::receiveMessage(const QString& portName, const QByteArray& data) {

    QString strIPHop, bpu, index;
    
    // 改进的端口索引解析
    quint8 portIndex = extractPortIndex(portName);
    findKeysByPortIndex(portIndex, strIPHop, bpu, index);
    uint16_t bpuInt = bpu.toUInt();
    if (!strIPHop.isEmpty())
    {
        QStringList parts = strIPHop.split(':');

        QString ip = parts[0];     // "192.168.70.109"
        QString hop = parts[1];    // "0"

        // 组装tempData格式: 0x03 | 长度(小端) | 串口号 | 数据长度 | 数据内容
        QByteArray tempData;
        
        // 1. 添加固定命令码 0x03
        tempData.append(static_cast<char>(0x03));
        
        // 2. 计算总长度 = 串口号(1) + 数据长度(1) + 数据内容长度
        uint16_t totalLength = 1 + 1 + data.size();
        
        // 3. 添加长度（小端格式）
        tempData.append(static_cast<char>(totalLength & 0xFF));         // 低字节
        tempData.append(static_cast<char>((totalLength >> 8) & 0xFF));  // 高字节
        
        // 4. 添加串口号（从index转换而来）
        quint8 comPortIndex = index.toUInt();
        tempData.append(static_cast<char>(comPortIndex));
        
        // 5. 添加数据长度
        tempData.append(static_cast<char>(data.size()));
        
        // 6. 添加数据内容
        tempData.append(data);
        
        // 打印调试信息
        QString hexString = tempData.toHex().toUpper();


        // 根据bpuInt的值计算sktEn
        uint32_t sktEn = 0;
        if (bpuInt >= 0 && bpuInt <= 15) {  // 确保bpuInt在有效范围内（0-15）
            // 每个bpuInt值对应设置两个连续的位
            // bpuInt=0 -> 位0,1置1
            // bpuInt=1 -> 位2,3置1
            // bpuInt=2 -> 位4,5置1
            // 以此类推...
            int bitPosition = bpuInt * 2;  // 计算起始位位置
            sktEn = (0x3 << bitPosition);  // 0x3 = 0b11，左移到对应位置
        }

        AngKMessageHandler::instance().Command_RemoteDoPTCmd(ip.toStdString(), 0, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_DoCustom
            ,sktEn, bpuInt, tempData);
    }
    else
    {
        
    }

    return true;
}
void MT422SerialHandler::findKeysByPortIndex(quint8 portIndex, QString& outStrIPHop, QString& outBpu, QString& outIndex)
{
    // 遍历 QMap
    QMap<QString, quint8>::const_iterator it;
    for (it = m_strIPHopBpuIndexToPortMap.constBegin(); it != m_strIPHopBpuIndexToPortMap.constEnd(); ++it)
    {
        if (it.value() == portIndex)
        {
            // 找到匹配的 portIndex，解析键
            QString key = it.key();
            QStringList parts = key.split('_');

            if (parts.size() == 3)
            {
                outStrIPHop = parts[0];
                outBpu = parts[1];
                outIndex = parts[2];
                return; // 找到第一个匹配项就返回
            }
        }
    }

    // 如果没有找到，可以设置默认值或抛出异常
    outStrIPHop.clear();
    outBpu.clear();
    outIndex.clear();
}
// 新增辅助函数来提取端口索引
quint8 MT422SerialHandler::extractPortIndex(const QString& portName) {
    // 使用正则表达式匹配COM后面的数字
    QRegularExpression regex("COM(\\d+)");
    QRegularExpressionMatch match = regex.match(portName);

    if (match.hasMatch()) {
        return match.captured(1).toInt();
    }

    // 如果正则匹配失败，回退到原来的方法（适用于简单的COMx格式）
    if (portName.startsWith("COM") && portName.length() > 3) {
        return portName.mid(3).toInt();
    }

    return 0;
}
void MT422SerialHandler::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }

    QSerialPort* port = qobject_cast<QSerialPort*>(sender());
    QString errorMessage = QString("Serial port error on %1: %2")
        .arg(port ? port->portName() : "unknown")
        .arg(port ? port->errorString() : "unknown error");

    ALOG_ERROR("%s", "CU", "--", errorMessage.toStdString().c_str());
    emit errorOccurred(errorMessage);
}

QByteArray MT422SerialHandler::formatMessage(const QByteArray& message) const
{
    QByteArray formattedMessage;

    // Add MT422 tag (4 bytes)
    formattedMessage.append((char*)&MT422_TAG_ID, 4);

    // Add message length (2 bytes)
    quint16 length = message.length();
    formattedMessage.append((char*)&length, 2);

    // Add message payload
    formattedMessage.append(message);

    // Calculate and add CRC16 (2 bytes)
    quint16 crc = calculateCRC16(formattedMessage);
    formattedMessage.append((char*)&crc, 2);

    return formattedMessage;
}

quint16 MT422SerialHandler::calculateCRC16(const QByteArray& data) const
{
    quint16 crc = 0xFFFF;
    for (char byte : data) {
        crc ^= (quint8)byte;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            }
            else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

/*bool MT422SerialHandler::writeToPort(quint8 portIndex, const QByteArray& data)
{
    auto port = getPort(portIndex);
    if (!port || !port->isOpen()) {
        ALOG_ERROR("Port %d is not open", "CU", "--", portIndex);
        return false;
    }
    
    qint64 bytesWritten = port->write(data);
    if (bytesWritten != data.size()) {
        ALOG_ERROR("Failed to write all data to port %d: wrote %lld of %d bytes", "CU", "--",
            portIndex, bytesWritten, data.size());
        return false;
    }

    if (!port->waitForBytesWritten(1000)) {
        ALOG_ERROR("Write timeout on port %d", "CU", "--", portIndex);
        return false;
    }

    ALOG_DEBUG("Successfully wrote %lld bytes to port %d", "CU", "--", bytesWritten, portIndex);
    return true;
}
*/
// MT422SerialHandler.cpp
bool MT422SerialHandler::writeToPort(quint8 portIndex, const QByteArray& data) {
    //auto reader = m_portReaders.value(portIndex, nullptr);
    //if (!reader) {
    //    ALOG_ERROR("Port %d reader not found", "CU", "--", portIndex);
    //    return false;
    //}

    // // 触发信号（子线程通过Qt::QueuedConnection异步处理）
    //emit writeRequested(portIndex, data);
    return true;  // 直接返回，不保证实际写入完成
}
std::shared_ptr<QSerialPort> MT422SerialHandler::getPort(quint8 portIndex)
{
    //auto it = m_serialPorts.find(portIndex);
    //if (it != m_serialPorts.end()) {
    //    return it.value();
    //}

    //// 如果端口不存在但有配置，尝试创建
    //if (m_portConfig.contains(QString::number(portIndex))) {
    //    QJsonObject portInfo = m_portConfig[QString::number(portIndex)];
    //    QString portName = portInfo["port"].toString();
    //    int baudRate = portInfo["baudRate"].toInt(921600);

    //    if (configurePort(portIndex, portName, baudRate)) {
    //        return m_serialPorts[portIndex];
    //    }
    //}

    return nullptr;
}

void MT422SerialHandler::handleReceivedData(const QString& strIPHop,
    const uint16_t BPUID, quint8 index, const QByteArray& data)
{
    QByteArray payload = "kkkkkk";
   // if (!validateReceivedMessage(data, payload)) {
    //    ALOG_WARN("Received invalid message from strIPHop %s", "CU", "--", strIPHop);
   //     return;
   // }

    ALOG_DEBUG("Received valid message from strIPHop: %s, BPUID: %d, index: %d, length: %d",
        "CU", "--", strIPHop.toStdString().c_str(), BPUID, index, payload.size());
    
    emit messageReceived(strIPHop, BPUID, index, payload);
}

bool MT422SerialHandler::validateReceivedMessage(const QByteArray& data, QByteArray& payload) const
{
    if (data.size() < 8) { // 至少需要标识(4) + 长度(2) + CRC(2)
        return false;
    }

    // 验证标识
    quint32 tagId;
    memcpy(&tagId, data.data(), 4);
    if (tagId != MT422_TAG_ID) {
        return false;
    }

    // 获取长度
    quint16 length;
    memcpy(&length, data.data() + 4, 2);

    // 验证总长度
    if (data.size() != 4 + 2 + length + 2) {
        return false;
    }

    // 验证CRC
    QByteArray messageWithoutCrc = data.left(data.size() - 2);
    quint16 calculatedCrc = calculateCRC16(messageWithoutCrc);

    quint16 receivedCrc;
    memcpy(&receivedCrc, data.data() + data.size() - 2, 2);

    if (calculatedCrc != receivedCrc) {
        return false;
    }

    // 提取payload
    payload = data.mid(6, length);
    return true;
}

// 修改 calculateCRC32 函数
quint32 MT422SerialHandler::calculateCRC32(const QByteArray& data) const
{
    CCrc32Std crc32;
    crc32.ReInit();  // 初始化 CRC

    // 计算 CRC32
    crc32.CalcSubRoutine((uint8_t*)data.data(), data.size());

    // 获取校验和
    quint32 checksum = 0;
    crc32.GetChecksum((uint8_t*)&checksum, sizeof(checksum));

    return checksum;
}

void MT422SerialHandler::getPortMapping(quint8 portIndex, QString& strIPHop, uint16_t& bpuid, quint8& index) const
{
    QString portKey = QString::number(portIndex);
    if (m_portConfig.contains(portKey)) {
        QJsonObject portInfo = m_portConfig[portKey];
        strIPHop = portInfo["strIPHop"].toString();
        bpuid = portInfo["bpu"].toInt();
        index = portInfo["index"].toInt();
    }
}

// 新增函数，通过strIPHop、bpu和index查找对应的com口
quint8 MT422SerialHandler::getPortIndex(const QString& strIPHop, uint16_t bpu, quint8 index)
{
    QString key = QString("%1_%2_%3").arg(strIPHop).arg(bpu).arg(index);
    if (m_strIPHopBpuIndexToPortMap.contains(key)) {
        return m_strIPHopBpuIndexToPortMap[key];
    }
    return 0; // 找不到时返回0
}
// 组装自定义数据包（按小端格式）
QByteArray MT422SerialHandler::assembleCustomData(uint8_t cmdId, uint16_t dataLength, const QByteArray& payloadData) const
{
    QByteArray result;
    
    // 1. 添加命令ID (1字节)
    result.append(static_cast<char>(cmdId));
    
    // 2. 添加数据长度 (2字节，小端格式)
    result.append(static_cast<char>(dataLength & 0xFF));        // 低字节
    result.append(static_cast<char>((dataLength >> 8) & 0xFF)); // 高字节
    
    // 3. 添加载荷数据
    result.append(payloadData);
    
    return result;
}

void MT422SerialHandler::slotTaskDownLoadStatus2(std::string devIP, uint32_t HopNum)
{
    
    
    // 定义自定义数据参数
    uint8_t docustomDataCmdId = 0x05;
    
    // 准备载荷数据（将波特率按小端格式添加到载荷中）
    QByteArray payloadData;
    payloadData.append(static_cast<char>(jsonConfigBaudrate & 0xFF));         // 低字节
    payloadData.append(static_cast<char>((jsonConfigBaudrate >> 8) & 0xFF));  // 次低字节
    payloadData.append(static_cast<char>((jsonConfigBaudrate >> 16) & 0xFF)); // 次高字节
    payloadData.append(static_cast<char>((jsonConfigBaudrate >> 24) & 0xFF)); // 高字节
    
    uint16_t payloadDataLength = static_cast<uint16_t>(payloadData.size());
    
    // 使用辅助函数组装数据
    QByteArray tempData = assembleCustomData(docustomDataCmdId, payloadDataLength, payloadData);
    
    // 打印tempData的16进制内容
    QString hexString = tempData.toHex().toUpper();
    QString formattedHex;
    for (int i = 0; i < hexString.length(); i += 2) {
        if (i > 0) formattedHex += " ";
        formattedHex += hexString.mid(i, 2);
    }
    
    // 根据bpuInt的值计算sktEn
    for (uint16_t bpuInt = 0; bpuInt <= 7; bpuInt++) {
        uint32_t sktEn = 0;
        if (bpuInt >= 0 && bpuInt <= 7) {  // 确保bpuInt在有效范围内（0-7）
            // 每个bpuInt值对应设置两个连续的位
            // bpuInt=0 -> 位0,1置1
            // bpuInt=1 -> 位2,3置1
            // bpuInt=2 -> 位4,5置1
            // 以此类推...
            int bitPosition = bpuInt * 2;  // 计算起始位位置
            sktEn = (0x3 << bitPosition);  // 0x3 = 0b11，左移到对应位置
        }

        AngKMessageHandler::instance().Command_RemoteDoPTCmd(devIP, 0, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_DoCustom
            , sktEn, bpuInt, tempData);
        }
}