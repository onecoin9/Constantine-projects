#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort> // 包含 QSerialPort 枚举
#include <QMap>
#include <QMutex>     // 保护 workers map

class SerialPortWorker;
class QThread;

class SerialPortManager : public QObject {
    Q_OBJECT

public:
    explicit SerialPortManager(QObject* parent = nullptr);
    ~SerialPortManager() override;

    bool connectPort(const QString& portName,quint8 portIndex, int baudRate, QSerialPort::DataBits dataBits,
        QSerialPort::Parity parity, QSerialPort::StopBits stopBits);
    void disconnectPort(const QString& portName);

    // 发送数据：将数据放入目标 Worker 的队列并通知 Worker
    // 返回值可以忽略，或者表示是否找到了对应的 Worker
    bool sendData(const QString& portName, const QByteArray& data);

signals:
    // 从 Worker 转发的信号
    void dataReceived(const QString& portName, const QByteArray& data);
    void portErrorOccurred(const QString& portName, const QString& error); // 端口错误
    void portDisconnected(const QString& portName); // 端口断开
    void portConnected(const QString& portName);    // 端口连接（开始连接过程）

private slots:
    // 处理 Worker 发出的断开信号
    void handleWorkerDisconnection(const QString& portName);
    // 处理 Worker 发出的错误信号
    void handleWorkerError(const QString& portName, const QString& error);
    // 处理 Worker 的日志消息（可选）
    void handleWorkerLog(const QString& message);

private:
    // 存储 Worker 指针，访问受 m_mapMutex 保护
    QMap<QString, SerialPortWorker*> m_workers;
    // 存储线程指针，访问受 m_mapMutex 保护
    QMap<QString, QThread*> m_threads;
    // 保护 m_workers 和 m_threads 的互斥锁
    QMutex m_mapMutex;
};

#endif // SERIALPORTMANAGER_H