#ifndef SERIALPORTWORKER_H
#define SERIALPORTWORKER_H

#include <QObject>
#include <QSerialPort>
#include <QThread>
#include <QByteArray>
#include <QQueue>   // 使用 Qt 的队列
#include <QMutex>   // 使用 Qt 的互斥锁
#include <QMetaType>

class SerialPortWorker : public QObject {
    Q_OBJECT

public:
    explicit SerialPortWorker(QObject* parent = nullptr);
    ~SerialPortWorker() override;

    // 不再需要从外部直接访问队列，设为私有
    // QQueue<QByteArray> sendQueue; // 改为私有

    QThread* getThread() const;
    void setThread(QThread* thread);

    // 公共方法，供 Manager 调用（通过 invokeMethod）以添加数据到队列
    // 注意：这个方法本身是线程安全的，因为它内部使用了锁
    void enqueueData(const QByteArray& data);

public slots:
    // 由 Manager 通过 invokeMethod(QueuedConnection) 触发，在 Worker 线程执行
    void processSendDataQueue();
    // 初始化和连接端口（在 Worker 线程执行）
    void initializeAndConnectPort(const QString& portName, int baudRate, QSerialPort::DataBits dataBits,
        QSerialPort::Parity parity, QSerialPort::StopBits stopBits);
    // 断开端口（在 Worker 线程执行）
    void disconnectPort();

signals:
    void dataReceived(const QByteArray& data);
    void errorOccurred(const QString& error);
    void disconnectedSignal(); // 避免与 QObject::disconnected 冲突
    void logMessage(const QString& message); // 用于日志记录

private slots:
    void readData();
    void handleError(QSerialPort::SerialPortError error);

private:
    void performSendData(const QByteArray& data, const QString& traceId = QString()); // 内部实际发送逻辑

    QString m_portName;
    QSerialPort* m_serialPort;
    QThread* m_thread;

    QQueue<QByteArray> m_sendQueue; // 数据发送队列
    QMutex m_queueMutex;           // 保护 m_sendQueue 的互斥锁
    uint64_t test = 0;
};

#endif // SERIALPORTWORKER_H