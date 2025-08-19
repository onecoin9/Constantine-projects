#include "infrastructure/SerialChannel.h"
#include "infrastructure/SerialPortManager.h"
#include <QSerialPortInfo>
#include <QMutexLocker>
#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include <QVariant>
#include <QDateTime>
#include "core/Logger.h"

namespace Infrastructure {

SerialChannel::SerialChannel(QObject *parent)
    : ICommunicationChannel(parent)
    , m_serialPortManager(nullptr)
    , m_serialMode(SerialMode::RS232)
    , m_status(Status::Disconnected)
    , m_portIndex(0)
    , m_baudRate(9600)
    , m_dataBits(QSerialPort::Data8)
    , m_parity(QSerialPort::NoParity)
    , m_stopBits(QSerialPort::OneStop)
    , m_flowControl(QSerialPort::NoFlowControl)
    , m_isConnected(false)
{
}

SerialChannel::~SerialChannel()
{
    if (isOpen()) {
        close();
    }
}

void SerialChannel::setSerialPortManager(SerialPortManager* manager)
{
    QMutexLocker locker(&m_mutex);
    
    // 断开旧的连接
    if (m_serialPortManager) {
        disconnect(m_serialPortManager, nullptr, this, nullptr);
    }
    
    m_serialPortManager = manager;
    
    // 连接新的信号
    if (m_serialPortManager) {
        connect(m_serialPortManager, &SerialPortManager::dataReceived,
                this, &SerialChannel::handleDataReceived);
        connect(m_serialPortManager, &SerialPortManager::portConnected,
                this, &SerialChannel::handlePortConnected);
        connect(m_serialPortManager, &SerialPortManager::portDisconnected,
                this, &SerialChannel::handlePortDisconnected);
        connect(m_serialPortManager, &SerialPortManager::portErrorOccurred,
                this, &SerialChannel::handlePortError);
    }
}

bool SerialChannel::open()
{
    if (isOpen()) {
        return true;
    }

    if (!m_serialPortManager) {
        LOG_MODULE_ERROR("SerialChannel", "SerialPortManager is not set.");
        return false;
    }

    if (m_portName.isEmpty()) {
        LOG_MODULE_ERROR("SerialChannel", "Serial port name is not set.");
        return false;
    }

    bool result = m_serialPortManager->connectPort(
        m_portName,
        m_portIndex,
        m_baudRate,
        m_dataBits,
        m_parity,
        m_stopBits
    );

    if (result) {
        m_status = Status::Connected; // Or Connecting, depending on desired state machine
        m_isConnected = true;
        emit statusChanged(m_status);
        emit connected();
    } else {
        m_status = Status::Error;
        emit errorOccurred("无法打开串口");
        emit statusChanged(m_status);
    }
    LOG_MODULE_INFO("SerialChannel", QString("Opening serial port '%1', result: %2").arg(m_portName).arg(result).toStdString());
    return result;
}

void SerialChannel::close()
{
    if (!isOpen()) {
        return;
    }
    if (m_serialPortManager) {
        m_serialPortManager->disconnectPort(m_portName);
    }
    m_readBuffer.clear();
    m_status = Status::Disconnected;
    m_isConnected = false;
    emit statusChanged(m_status);
    emit disconnected();
}

bool SerialChannel::isOpen() const
{
    QMutexLocker locker(&m_mutex);
    return m_isConnected;
}

ICommunicationChannel::Status SerialChannel::getStatus() const
{
    QMutexLocker locker(&m_mutex);
    return m_status;
}

qint64 SerialChannel::send(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);

    if (!m_serialPortManager || !m_isConnected) {
        emit errorOccurred("串口未连接");
        return -1;
    }
    
    // RS485半双工模式下的特殊处理可以在SerialPortWorker中实现
    
    bool result = m_serialPortManager->sendData(m_portName, data);
    
    if (!result) {
        emit errorOccurred("发送数据失败");
        return -1;
    }
    
    return data.size();
}

QByteArray SerialChannel::receive(int timeoutMs)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isConnected) {
        emit errorOccurred("串口未连接");
        return QByteArray();
    }
    
    // 先返回缓冲区中的数据
    if (!m_readBuffer.isEmpty()) {
        QByteArray data = m_readBuffer;
        m_readBuffer.clear();
        return data;
    }
    
    // 等待新数据
    if (timeoutMs > 0) {
        m_dataAvailable.wait(&m_mutex, timeoutMs);
        
        if (!m_readBuffer.isEmpty()) {
            QByteArray data = m_readBuffer;
            m_readBuffer.clear();
            return data;
        }
    }
    
    return QByteArray();
}

void SerialChannel::clearBuffer()
{
    QMutexLocker locker(&m_mutex);
    m_readBuffer.clear();
}

void SerialChannel::setParameters(const QVariantMap &params)
{
    m_portName = params.value("portName").toString();
    // 使用toInt()进行安全转换
    m_baudRate = params.value("baudRate").toInt();

    LOG_MODULE_DEBUG("SerialChannel", QString("Parameters set for port '%1' with baud rate %2.")
        .arg(m_portName).arg(m_baudRate).toStdString());
}

QVariantMap SerialChannel::getParameters() const
{
    QVariantMap params;
    params["portName"] = m_portName;
    params["portIndex"] = m_portIndex;
    params["baudRate"] = m_baudRate;
    params["dataBits"] = static_cast<int>(m_dataBits);
    params["parity"] = static_cast<int>(m_parity);
    params["stopBits"] = static_cast<int>(m_stopBits);
    params["flowControl"] = static_cast<int>(m_flowControl);
    return params;
}

void SerialChannel::setSerialMode(SerialMode mode)
{
    QMutexLocker locker(&m_mutex);
    m_serialMode = mode;
}

SerialChannel::SerialMode SerialChannel::getSerialMode() const
{
    QMutexLocker locker(&m_mutex);
    return m_serialMode;
}

QStringList SerialChannel::getAvailablePorts()
{
    QStringList portNames;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        portNames << info.portName();
    }
    return portNames;
}

void SerialChannel::handleDataReceived(const QString &portName, const QByteArray &data)
{
    if (portName == m_portName) {
        emit dataReceived(data);
    }
}

void SerialChannel::handlePortConnected(const QString &portName)
{
    if (portName != m_portName) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    m_isConnected = true;
    m_status = Status::Connected;
    
    LOG_MODULE_INFO("SerialChannel", QString("Port '%1' connected.").arg(portName).toStdString());
}

void SerialChannel::handlePortDisconnected(const QString &portName)
{
    if (portName != m_portName) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    m_isConnected = false;
    m_status = Status::Disconnected;
    
    LOG_MODULE_INFO("SerialChannel", QString("Port '%1' disconnected.").arg(portName).toStdString());
}

void SerialChannel::handlePortError(const QString &portName, const QString &error)
{
    if (portName != m_portName) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    m_status = Status::Error;
    
    emit errorOccurred(error);
    emit statusChanged(m_status);
    
    LOG_MODULE_ERROR("SerialChannel", QString("Error on port '%1': %2").arg(portName).arg(error).toStdString());
}

void SerialChannel::enableTransmit()
{
    // RS485半双工发送使能
    // 这个功能现在应该在SerialPortWorker中实现
    // 可以通过SerialPortManager发送特殊命令
}

void SerialChannel::enableReceive()
{
    // RS485半双工接收使能
    // 这个功能现在应该在SerialPortWorker中实现
    // 可以通过SerialPortManager发送特殊命令
}

} // namespace Infrastructure 
