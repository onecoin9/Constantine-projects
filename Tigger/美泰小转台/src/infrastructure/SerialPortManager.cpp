#include "infrastructure/SerialPortManager.h"
#include "infrastructure/SerialPortWorker.h"
#include "core/Logger.h"

namespace Infrastructure {

SerialPortManager::SerialPortManager(QObject* parent) : QObject(parent) {
    LOG_MODULE_INFO("SerialPortManager", "SerialPortManager created.");
    qRegisterMetaType<QSerialPort::DataBits>("QSerialPort::DataBits");
    qRegisterMetaType<QSerialPort::Parity>("QSerialPort::Parity");
    qRegisterMetaType<QSerialPort::StopBits>("QSerialPort::StopBits");
}

SerialPortManager& SerialPortManager::getInstance()
{
    static SerialPortManager instance;
    return instance;
}

SerialPortManager::~SerialPortManager() {
    LOG_MODULE_INFO("SerialPortManager", "Shutting down all SerialPortManager threads...");
    shutdown();
}

bool SerialPortManager::connectPort(const QString& portName , quint8 portIndex, int baudRate, QSerialPort::DataBits dataBits,
    QSerialPort::Parity parity, QSerialPort::StopBits stopBits)
{
    QMutexLocker locker(&m_mapMutex); // 锁定 map 访问

    if (m_workers.contains(portName)) {
        LOG_ERROR(QString("端口 %1 已连接或正在连接.").arg(portName).toStdString());
        return false;
    }

    SerialPortWorker* worker = new SerialPortWorker();
    QThread* thread = new QThread();

    worker->setThread(thread); // 让 worker 知道它的线程
    worker->moveToThread(thread); // 将 worker 移动到新线程

    // --- 连接信号和槽 ---
    // 1. 线程启动后初始化端口
    connect(thread, &QThread::started, [=]() {
        // 使用 invokeMethod 确保在 worker 线程事件循环中执行初始化
        QMetaObject::invokeMethod(worker, "initializeAndConnectPort", Qt::QueuedConnection,
            Q_ARG(QString, portName),
            Q_ARG(int, baudRate),
            Q_ARG(QSerialPort::DataBits, dataBits),
            Q_ARG(QSerialPort::Parity, parity),
            Q_ARG(QSerialPort::StopBits, stopBits));
        });

    // 2. 转发 Worker 的信号到 Manager (使用 QueuedConnection 保证在 Manager 线程处理)
    connect(worker, &SerialPortWorker::dataReceived, this, [this, portName](const QString& receivedPortName, const QByteArray& data) {
        Q_UNUSED(receivedPortName); // 我们已经知道是哪个端口
        emit dataReceived(portName, data);
        }, Qt::QueuedConnection);

    // 连接错误信号
    connect(worker, &SerialPortWorker::errorOccurred, this, [this, portName](const QString& error) {
        emit portErrorOccurred(portName, error);
        }, Qt::QueuedConnection);

    // 连接断开信号
    connect(worker, &SerialPortWorker::disconnectedSignal, this, [this, portName]() {
        emit portDisconnected(portName);
        }, Qt::QueuedConnection);

    // 3. 线程结束后自动清理 Worker 对象
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);

    // --- 存储并启动 ---
    m_workers.insert(portName, worker);
    m_threads.insert(portName, thread);

    thread->start(); // 启动线程（之后会触发 started 信号）

    LOG_INFO(QString("已启动端口 %1 的连接过程").arg(portName).toStdString());
    emit portConnected(portName); // 发送开始连接的信号

    return true;
}

void SerialPortManager::disconnectPort(const QString& portName) {
    LOG_INFO(QString("Manager 尝试断开端口 %1").arg(portName).toStdString());
    QThread* thread = nullptr;
    SerialPortWorker* worker = nullptr;

    { // 互斥锁作用域
        QMutexLocker locker(&m_mapMutex);
        if (!m_workers.contains(portName)) {
            LOG_INFO(QString("断开时未找到端口 %1 (可能已断开).").arg(portName).toStdString());
            return;
        }
        worker = m_workers.take(portName);
        thread = m_threads.take(portName);
    } // 互斥锁释放

    if (thread) {
        LOG_INFO(QString("请求端口 %1 的 Worker 断开并请求线程退出.").arg(portName).toStdString());

        // Request Worker in its thread to close the serial port
        QMetaObject::invokeMethod(worker, "disconnectPort", Qt::QueuedConnection);

        // Request thread to exit event loop
        thread->quit();

        // Wait for thread to finish (with a timeout)
        if (!thread->wait(3000)) { // Wait for 3 seconds
            LOG_ERROR(QString("端口 %1 的线程未能优雅退出，将强制终止.").arg(portName).toStdString());
            thread->terminate(); // Force terminate (use with caution)
            thread->wait();      // Wait after termination
        }
        else {
            LOG_INFO(QString("端口 %1 的线程已优雅退出.").arg(portName).toStdString());
        }

        // The worker is deleted via the QObject::deleteLater slot 
        // connected to the thread's finished() signal.
        // We can now safely delete the thread object.
        delete thread;
    }
    else {
        LOG_ERROR(QString("断开端口 %1 时发现不一致（线程或 Worker 丢失）.").arg(portName).toStdString());
    }

    LOG_INFO(QString("端口 %1 的断开过程完成.").arg(portName).toStdString());
    emit portDisconnected(portName); // Signal that disconnection is complete
}


// 高并发调用的发送函数
bool SerialPortManager::sendData(const QString& portName, const QByteArray& data) {
    SerialPortWorker* worker = nullptr;

    { // 短暂锁定 map 以查找 worker
        QMutexLocker locker(&m_mapMutex);
        worker = m_workers.value(portName, nullptr); // 安全地获取 worker 指针
    } // 互斥锁释放

    if (!worker) {
         LOG_ERROR(QString("端口 %1 未连接，无法发送数据.").arg(portName).toStdString()); // 可能非常频繁，谨慎打印
        //ALOG_INFO("端口未连接:%s ", "CU", "--", portName.toLocal8Bit().data());
        return false; // 端口不存在或正在断开
    }

    // --- 将数据放入 Worker 的队列 ---
    // 注意：这里没有直接访问 worker 的 m_queueMutex 和 m_sendQueue
    // 我们调用 worker 的公共方法 enqueueData，它内部处理了锁
    worker->enqueueData(data); // 这个方法是线程安全的

    // --- 通知 Worker 处理队列 ---
    // 使用 invokeMethod 发送一个事件到 Worker 的事件循环
    // 让 Worker 线程知道需要检查并处理它的发送队列了
    QMetaObject::invokeMethod(worker, "processSendDataQueue", Qt::QueuedConnection);

    // 可选的高频日志（可能影响性能）
    // QString hexString = data.toHex();
    // ALOG_INFO("已将数据排队给端口 :%s %s", "CU", "--", portName.toLocal8Bit().data(), hexString.toLocal8Bit().data());

    return true; // 数据已成功放入队列（或尝试放入）
}


// --- 私有槽函数 (在 Manager 线程中执行) ---

void SerialPortManager::handleWorkerDisconnection(const QString& portName) {
    LOG_INFO(QString("端口 %1 的 Worker 发出断开信号，开始清理.").arg(portName).toStdString());
    disconnectPort(portName); // 执行标准的断开清理流程
}

void SerialPortManager::handleWorkerError(const QString& portName, const QString& error) {
    LOG_ERROR(QString("端口 %1 的 Worker 报告错误: %2").arg(portName).arg(error).toStdString());
    emit portErrorOccurred(portName, error);
    // 可以根据错误类型决定是否自动断开
    // if (error.contains("ResourceError")) {
    //     disconnectPort(portName);
    // }
}

void SerialPortManager::handleWorkerLog(const QString& message) {
    LOG_INFO(QString("Worker 日志: %1").arg(message).toStdString());
    // 可以在这里集中处理日志
}

void SerialPortManager::shutdown()
{
    {
        QMutexLocker locker(&m_mutex);
        if (m_isShutdown) {
            return;
        }
        m_isShutdown = true;
    }

    for (const QString &portName : m_workers.keys()) {
        if (m_threads.contains(portName) && m_threads[portName]->isRunning()) {
            m_threads[portName]->quit();
            m_threads[portName]->wait(3000);
        }
    }
    m_workers.clear();
    m_threads.clear();
    LOG_MODULE_INFO("SerialPortManager", "All SerialPortManager threads shut down.");
}

} // namespace Infrastructure
