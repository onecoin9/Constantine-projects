#include <QSerialPort>
#include <QThread>
#include "infrastructure/SerialPortWorker.h"
#include <QDateTime>
#include "core/Logger.h"

namespace Infrastructure {

SerialPortWorker::SerialPortWorker(QObject* parent)
    : QObject(parent),
    m_serialPort(nullptr),
    m_thread(nullptr)
{
    LOG_MODULE_DEBUG("SerialPortWorker", "SerialPortWorker created.");
    // 队列和互斥锁会自动初始化
}

SerialPortWorker::~SerialPortWorker() {
    LOG_MODULE_DEBUG("SerialPortWorker", QString("SerialPortWorker destroying (port: %1)...").arg(m_portName).toStdString());
    // 串口资源应在 Manager 请求断开时清理，这里作为最后防线
    if (m_serialPort) {
        if (m_serialPort->isOpen()) {
            m_serialPort->close();
        }
        delete m_serialPort;
        m_serialPort = nullptr;
    }
    LOG_MODULE_DEBUG("SerialPortWorker", QString("SerialPortWorker destroyed (port: %1).").arg(m_portName).toStdString());
}

// 这个方法可以被 Manager 在任何线程调用
void SerialPortWorker::enqueueData(const QByteArray& data) {
    QMutexLocker locker(&m_queueMutex); // 自动加锁和解锁
    m_sendQueue.enqueue(data);
}

// --- 以下方法预期在 Worker 自己的线程中执行 ---

// 初始化和连接端口
void SerialPortWorker::initializeAndConnectPort(const QString& portName, int baudRate, QSerialPort::DataBits dataBits,
    QSerialPort::Parity parity, QSerialPort::StopBits stopBits)
{
    Q_ASSERT(QThread::currentThread() == m_thread); // 确认在正确的线程
    if (m_serialPort) {
        emit errorOccurred(portName,QString("端口 %1 已初始化.").arg(portName));
        return;
    }

    m_portName = portName;
    m_serialPort = new QSerialPort(this); // 父对象设为 Worker

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(dataBits);
    m_serialPort->setParity(parity);
    m_serialPort->setStopBits(stopBits);

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        QString errorStr = m_serialPort->errorString();
        emit errorOccurred(portName,QString("打开端口 %1 失败: %2").arg(portName).arg(errorStr));
        LOG_MODULE_ERROR("SerialPortWorker", QString("Failed to open port %1: %2").arg(portName).arg(errorStr).toStdString());
        delete m_serialPort;
        m_serialPort = nullptr;
        return;
    }

    // 关键修复：清空串口缓冲区，防止旧数据干扰
    m_serialPort->clear();

    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPortWorker::readData, Qt::DirectConnection);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialPortWorker::handleError, Qt::DirectConnection);

    LOG_MODULE_DEBUG("SerialPortWorker", QString("Connected to port: %1").arg(portName).toStdString());
    emit logMessage(QString("Worker 已连接到端口: %1").arg(portName));
}

// 断开端口
void SerialPortWorker::disconnectPort() {
    Q_ASSERT(QThread::currentThread() == m_thread || !m_thread); // 允许在清理阶段调用

    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
        LOG_MODULE_DEBUG("SerialPortWorker", QString("Worker disconnected from port %1.").arg(m_portName).toStdString());
        emit logMessage(QString("Worker 已从端口 %1 断开.").arg(m_portName));
    }
    if (m_serialPort) {
        m_serialPort->disconnect(this); //断开所有到此对象的连接
        // 串口对象的删除由析构函数或 Manager 控制
    }

    // 清空可能未发送的数据队列
    {
        QMutexLocker locker(&m_queueMutex);
        m_sendQueue.clear();
    }

    emit disconnectedSignal(); // 发送断开信号
}

// 处理发送队列（在 Worker 线程执行）
void SerialPortWorker::processSendDataQueue() {
    // Q_ASSERT(QThread::currentThread() == m_thread); // 调试时打开
    if (!m_serialPort || !m_serialPort->isOpen()) {
        // 端口未打开，清空队列以防积压
        QMutexLocker locker(&m_queueMutex);
        if (!m_sendQueue.isEmpty()) {
            LOG_MODULE_ERROR("SerialPortWorker", QString("Port %1 is not open, discarding %2 queued messages.").arg(m_portName).arg(m_sendQueue.size()).toStdString());
            m_sendQueue.clear();
        }
        return;
    }

    QQueue<QByteArray> localQueue; // 创建一个局部队列

    // 1. 快速锁定，将主队列数据转移到局部队列，然后解锁
    {
        QMutexLocker locker(&m_queueMutex);
        // 交换比逐个 dequeue 更快
        localQueue.swap(m_sendQueue);
    } // 互斥锁在这里释放

    // 2. 处理局部队列中的数据，此时不再持有锁
    while (!localQueue.isEmpty()) {
        performSendData(localQueue.dequeue());
    }
}

// 内部实际发送函数
void SerialPortWorker::performSendData(const QByteArray& data) {
    if (!m_serialPort || !m_serialPort->isOpen()) {
        if (m_portName.contains("5"))LOG_MODULE_ERROR("SerialPortWorker", QString("Port %1 is not open, cannot send data.").arg(m_portName).toStdString());
        emit errorOccurred(m_portName,QString("端口 %1 未打开 (发送时).").arg(m_portName));
        return;
    }
    qint64 bytesWritten = m_serialPort->write(data);
    //qDebug()<<"线程号"<<QThread::currentThreadId()<<"字节数:"<<data.size()<<"内容:"<<QString(data.toHex(' ').toUpper());
    if (bytesWritten == -1) {
        QString errorStr = m_serialPort->errorString();
        if(m_portName.contains("5"))LOG_MODULE_ERROR("SerialPortWorker", QString("Error writing to port %1: %2").arg(m_portName).arg(errorStr).toStdString());
        emit errorOccurred(m_portName,QString("写入错误 %1: %2").arg(m_portName).arg(errorStr));
    }
    else if (bytesWritten < data.size()) {
        emit errorOccurred(m_portName,QString("部分写入 %1").arg(m_portName));
    }
    else {
        test++;
        // 写入成功
        // m_serialPort->flush(); // 通常不需要，除非需要极低延迟保证，但会阻塞
        // QString hexString = data.toHex().toUpper();
        // LOG_INFO(QString("发送到 %1: %2").arg(m_portName).arg(hexString));
    }
}

// 读取数据（在 Worker 线程执行）
void SerialPortWorker::readData() {
    if (!m_serialPort || !m_serialPort->isOpen()) return;

    if (m_serialPort->bytesAvailable() > 0) {
        QByteArray data = m_serialPort->readAll();
        if (!data.isEmpty()) {
            emit dataReceived(m_portName, data);
        }
    }
}

// 处理串口错误（在 Worker 线程执行）
void SerialPortWorker::handleError(QSerialPort::SerialPortError error) {
    if (error != QSerialPort::NoError) {
        const QString errorString = m_serialPort->errorString();
        emit errorOccurred(m_portName, errorString);

        if (error == QSerialPort::ResourceError) {
            LOG_MODULE_ERROR("SerialPortWorker", QString("Resource error on port %1, assuming disconnected.").arg(m_portName).toStdString());
            if (m_serialPort && m_serialPort->isOpen()) {
                m_serialPort->close(); // 尝试关闭
            }
            emit disconnectedSignal(); // 通知 Manager
        }
    }
}

QThread* SerialPortWorker::getThread() const {
    return m_thread;
}

void SerialPortWorker::setThread(QThread* thread) {
    m_thread = thread;
}

} // namespace Infrastructure
