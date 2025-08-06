#include "UdpCommunicator.h"
#include <QNetworkDatagram>
#include <QDebug>

UdpCommunicator::UdpCommunicator(QObject *parent)
    : QObject(parent),
      m_udpSocket(new QUdpSocket(this)),
      m_targetServerPort(0),
      m_sequenceCounter(0)
{
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &UdpCommunicator::onReadyRead);
    // QUdpSocket::errorOccurred 信号在较新Qt版本中可用，旧版本可能使用 error()
    // connect(m_udpSocket, &QUdpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError socketError){
    //     Q_UNUSED(socketError);
    //     emit bindError(m_udpSocket->errorString());
    // });
}

UdpCommunicator::~UdpCommunicator()
{
    if (m_udpSocket->state() != QAbstractSocket::UnconnectedState) {
        m_udpSocket->close();
    }
}

bool UdpCommunicator::bind(quint16 localPort, QAbstractSocket::BindMode mode)
{
    if (m_udpSocket->bind(QHostAddress::AnyIPv4, localPort, mode)) {
        qInfo() << "UDP socket bound to port" << m_udpSocket->localPort();
        return true;
    } else {
        qWarning() << "UDP socket bind failed:" << m_udpSocket->errorString();
        emit bindError(m_udpSocket->errorString());
        return false;
    }
}

void UdpCommunicator::setTargetServer(const QHostAddress& address, quint16 port)
{
    m_targetServerAddress = address;
    m_targetServerPort = port;
    qInfo() << "UDP target server set to" << address.toString() << ":" << port;
}

qint64 UdpCommunicator::sendPacket(const QByteArray& packet)
{
    if (!m_targetServerAddress.isNull() && m_targetServerPort != 0) {
        qint64 bytesSent = m_udpSocket->writeDatagram(packet, m_targetServerAddress, m_targetServerPort);
        if (bytesSent == -1) {
            qWarning() << "UDP send error:" << m_udpSocket->errorString();
            emit sendError(m_udpSocket->errorString());
        } else {
            qDebug() << "UDP packet sent (" << bytesSent << "bytes) to"
                     << m_targetServerAddress.toString() << ":" << m_targetServerPort;
        }
        return bytesSent;
    } else {
        qWarning() << "UDP target server address or port not set. Cannot send packet.";
        emit sendError("Target server not set.");
        return -1;
    }
}

void UdpCommunicator::sendDeviceControl(const QString& thermalMode, double targetTemp, int stableTime, double rampRate)
{
    m_sequenceCounter++;
    QByteArray packet = CommandBuilder::buildDeviceControlCommand(m_sequenceCounter, thermalMode, targetTemp, stableTime, rampRate);
    sendPacket(packet);
    qDebug() << "Sent DeviceControl command via UDP, sequence:" << m_sequenceCounter;
}

void UdpCommunicator::sendFunctionTestConfig(
    bool enabledTest, bool powerOn, double targetVoltage, int voltageStableTime,
    const QVariant& targetCurrent, const QVariant& targetDriveFrequency,
    const QVariant& targetDriveForce, const QVariant& targetQuadrature,
    const QVariant& targetAmplitude, const QVariant& targetClock
) {
    m_sequenceCounter++;
    QByteArray packet = CommandBuilder::buildFunctionTestCommand(
        m_sequenceCounter, enabledTest, powerOn, targetVoltage, voltageStableTime,
        targetCurrent, targetDriveFrequency, targetDriveForce,
        targetQuadrature, targetAmplitude, targetClock
    );
    sendPacket(packet);
    qDebug() << "Sent FunctionTestConfig command via UDP, sequence:" << m_sequenceCounter;
}

void UdpCommunicator::sendDataAcquisitionConfig(bool enable, int duration, int interval)
{
    m_sequenceCounter++;
    QByteArray packet = CommandBuilder::buildDataAcquisitionConfigCommand(m_sequenceCounter, enable, duration, interval);
    sendPacket(packet);
    qDebug() << "Sent DataAcquisitionConfig command via UDP, sequence:" << m_sequenceCounter;
}

void UdpCommunicator::onReadyRead()
{
    while (m_udpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        // 或者对于旧版Qt:
        // QByteArray buffer;
        // buffer.resize(m_udpSocket->pendingDatagramSize());
        // QHostAddress senderAddress;
        // quint16 senderPort;
        // m_udpSocket->readDatagram(buffer.data(), buffer.size(), &senderAddress, &senderPort);
        // processDatagram(buffer, senderAddress, senderPort);

        processDatagram(datagram.data(), datagram.senderAddress(), datagram.senderPort());
    }
}

void UdpCommunicator::processDatagram(const QByteArray& datagram, const QHostAddress& senderAddress, quint16 senderPort)
{
    qDebug() << "UDP datagram received from" << senderAddress.toString() << ":" << senderPort << "Size:" << datagram.size();

    ProtocolHeader header;
    QByteArray jsonDataPayload;

    if (!ProtocolUtils::parseHeader(datagram, header, jsonDataPayload)) {
        qWarning() << "UDP: Failed to parse packet header from" << senderAddress.toString();
        return;
    }

    QJsonObject jsonData = ProtocolUtils::parseJsonData(jsonDataPayload);
    if (jsonData.isEmpty()) {
        qWarning() << "UDP: Failed to parse JSON data or JSON is empty from" << senderAddress.toString();
        return;
    }

    qDebug() << "UDP Parsed CmdID:" << Qt::hex << header.cmdID << "CmdDataSize:" << header.cmdDataSize;
    qDebug() << "UDP JSON Data:" << QJsonDocument(jsonData).toJson(QJsonDocument::Indented);

    switch (header.cmdID) {
        case 0x01: { // 设备控制响应
            int code, timestamp;
            if (CommandBuilder::parseDeviceControlResponse(jsonData, code, timestamp)) {
                qInfo() << "UDP Device Control Response - Code:" << code << "Timestamp:" << timestamp;
                emit deviceControlResponseReceived(code, timestamp, senderAddress, senderPort);
            } else {
                qWarning() << "UDP: Failed to parse Device Control Response.";
            }
            break;
        }
        case 0x02: { // 功能测试响应
            int code, timestamp;
            if (CommandBuilder::parseFunctionTestResponse(jsonData, code, timestamp)) {
                qInfo() << "UDP Function Test Response - Code:" << code << "Timestamp:" << timestamp;
                emit functionTestResponseReceived(code, timestamp, senderAddress, senderPort);
            } else {
                qWarning() << "UDP: Failed to parse Function Test Response.";
            }
            break;
        }
        case 0x03: { // 数据采集数据 (服务端主动上报)
            CommandBuilder::SensorData sensorData;
            if (CommandBuilder::parseDataAcquisitionData(jsonData, sensorData)) {
                qInfo() << "UDP Sensor Data Received - Temp:" << sensorData.temperature
                        << "Voltage:" << sensorData.voltage << "Current:" << sensorData.current;
                emit sensorDataReceived(sensorData, senderAddress, senderPort);
            } else {
                qWarning() << "UDP: Failed to parse Sensor Data.";
            }
            break;
        }
        default:
            qWarning() << "UDP: Unknown CmdID received:" << Qt::hex << header.cmdID;
            emit unknownPacketReceived(header.cmdID, jsonData, senderAddress, senderPort);
            break;
    }
}