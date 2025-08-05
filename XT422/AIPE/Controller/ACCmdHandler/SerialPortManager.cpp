#include "SerialPortManager.h"
#include "SerialPortWorker.h" // 包含 Worker 定义
#include <QThread>
#include <QDebug>
#include <QMutexLocker>
#include <QMetaObject> // 用于 invokeMethod

// 假设的日志宏
#define LOG_INFO(msg) qDebug() << QThread::currentThreadId() << ":" << msg
#define LOG_ERROR(msg) qWarning() << QThread::currentThreadId() << ":" << msg
// #include <AngkLogger.h>
// #define LOG_INFO(msg) ALOG_INFO("%s", "CU", "--", msg.toLocal8Bit().constData())

SerialPortManager::SerialPortManager(QObject* parent) : QObject(parent) {
    LOG_INFO("SerialPortManager 创建.");
    qRegisterMetaType<QSerialPort::DataBits>("QSerialPort::DataBits");
    qRegisterMetaType<QSerialPort::Parity>("QSerialPort::Parity");
    qRegisterMetaType<QSerialPort::StopBits>("QSerialPort::StopBits");
}

SerialPortManager::~SerialPortManager() {
    LOG_INFO("SerialPortManager 销毁中...");
    QMutexLocker locker(&m_mapMutex);

    // 复制 key 列表，因为 disconnectPort 会修改 map
    QStringList portNames = m_workers.keys();
    locker.unlock(); // 解锁，因为 disconnectPort 内部会加锁

    for (const QString& portName : portNames) {
        disconnectPort(portName); // 调用公共的断开方法
    }
    LOG_INFO("SerialPortManager 销毁完成.");
}

bool SerialPortManager::connectPort(const QString& portName , quint8 portIndex, int baudRate, QSerialPort::DataBits dataBits,
    QSerialPort::Parity parity, QSerialPort::StopBits stopBits)
{
    QMutexLocker locker(&m_mapMutex); // 锁定 map 访问

    if (m_workers.contains(portName)) {
        LOG_ERROR(QString("端口 %1 已连接或正在连接.").arg(portName));
        return false;
    }

    SerialPortWorker* worker = new SerialPortWorker();
    QThread* thread = new QThread();

    worker->setThread(thread); // 让 worker 知道它的线程
    worker->moveToThread(thread); // 将 worker 移动到新线程

    // --- 连接信号和槽 ---
    // 1. 线程启动后初始化端口
    connect(thread, &QThread::started, worker, [=]() {
        // 使用 invokeMethod 确保在 worker 线程事件循环中执行初始化
        QMetaObject::invokeMethod(worker, "initializeAndConnectPort", Qt::QueuedConnection,
            Q_ARG(QString, portName),
            Q_ARG(int, baudRate),
            Q_ARG(QSerialPort::DataBits, dataBits),
            Q_ARG(QSerialPort::Parity, parity),
            Q_ARG(QSerialPort::StopBits, stopBits));
        });

    // 2. 转发 Worker 的信号到 Manager (使用 QueuedConnection 保证在 Manager 线程处理)
    connect(worker, &SerialPortWorker::dataReceived, this, [this, portName](const QByteArray& data) {
        emit dataReceived(portName, data);
        }, Qt::QueuedConnection);


    // 3. 线程结束后自动清理 Worker 和 Thread 对象
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    // --- 存储并启动 ---
    m_workers.insert(QString("COM%1").arg(portIndex), worker);
    m_threads.insert(portName, thread);

    thread->start(); // 启动线程（之后会触发 started 信号）

    //LOG_INFO(QString("已启动端口 %1 的连接过程").arg(portName));
    emit portConnected(portName); // 发送开始连接的信号

    return true;
}

void SerialPortManager::disconnectPort(const QString& portName) {
    //LOG_INFO(QString("Manager 尝试断开端口 %1").arg(portName));
    QThread* thread = nullptr;
    SerialPortWorker* worker = nullptr;

    { // 互斥锁作用域
        QMutexLocker locker(&m_mapMutex);
        if (!m_workers.contains(portName)) {
            LOG_ERROR(QString("断开时未找到端口 %1.").arg(portName));
            return;
        }
        worker = m_workers.value(portName);
        thread = m_threads.value(portName);

        // 从 map 中移除，防止新的访问
        m_workers.remove(portName);
        m_threads.remove(portName);
    } // 互斥锁释放

    if (thread && worker) {
        LOG_INFO(QString("请求端口 %1 的 Worker 断开并请求线程退出.").arg(portName));

        // 请求 Worker 在其线程中断开串口连接
        QMetaObject::invokeMethod(worker, "disconnectPort", Qt::QueuedConnection);

        // 请求线程退出事件循环
        thread->quit();

        // 等待线程结束（设置超时）
        if (!thread->wait(5000)) { // 等待最多 5 秒
            LOG_ERROR(QString("端口 %1 的线程未能优雅退出，将强制终止.").arg(portName));
            thread->terminate(); // 强制终止（谨慎使用）
            thread->wait();      // 终止后也要等待
        }
        else {
            LOG_INFO(QString("端口 %1 的线程已优雅退出.").arg(portName));
        }
        // deleteLater 会在 QThread::finished 信号连接中处理
    }
    else {
        LOG_ERROR(QString("断开端口 %1 时发现不一致（线程或 Worker 丢失）.").arg(portName));
    }

    LOG_INFO(QString("端口 %1 的断开过程完成.").arg(portName));
    emit portDisconnected(portName); // 发送实际断开完成的信号
}


// 高并发调用的发送函数
bool SerialPortManager::sendData(const QString& portName, const QByteArray& data) {    // 获取traceId
    QString traceIdStr = property("currentTraceId").toString();
    quint64 traceId = traceIdStr.toULongLong();
    
    SerialPortWorker* worker = nullptr;

    { // 短暂锁定 map 以查找 worker
        QMutexLocker locker(&m_mapMutex);
        worker = m_workers.value(portName, nullptr); // 安全地获取 worker 指针
    } // 互斥锁释放

    if (!worker) {
         LOG_ERROR(QString("端口 %1 未连接，无法发送数据.").arg(portName)); // 可能非常频繁，谨慎打印
         return false; // 端口不存在或正在断开
    }

    // --- 将数据放入 Worker 的队列 ---
    // 注意：这里没有直接访问 worker 的 m_queueMutex 和 m_sendQueue
    // 我们调用 worker 的公共方法 enqueueData，它内部处理了锁
    
    // 将traceId传递给Worker
    worker->setProperty("currentTraceId", QVariant::fromValue(traceId));
    
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
    LOG_INFO(QString("端口 %1 的 Worker 发出断开信号，开始清理.").arg(portName));
    disconnectPort(portName); // 执行标准的断开清理流程
}

void SerialPortManager::handleWorkerError(const QString& portName, const QString& error) {
    LOG_ERROR(QString("端口 %1 的 Worker 报告错误: %2").arg(portName).arg(error));
    emit portErrorOccurred(portName, error);
    // 可以根据错误类型决定是否自动断开
    // if (error.contains("ResourceError")) {
    //     disconnectPort(portName);
    // }
}

void SerialPortManager::handleWorkerLog(const QString& message) {
    LOG_INFO(QString("Worker 日志: %1").arg(message));
    // 可以在这里集中处理日志
}