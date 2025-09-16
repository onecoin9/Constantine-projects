#ifndef SERIALCHANNEL_H
#define SERIALCHANNEL_H

#include "ICommunicationChannel.h"
#include <QSerialPort>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include <memory>

namespace Infrastructure {

class SerialPortManager;  // 前向声明

/**
 * @brief 串口通信通道实现
 * 使用SerialPortManager支持多设备共享串口
 * 支持RS232、RS422、RS485等串口通信
 */
class SerialChannel : public ICommunicationChannel
{
    Q_OBJECT
public:
    enum class SerialMode {
        RS232,
        RS422,
        RS485_HalfDuplex,
        RS485_FullDuplex
    };

    explicit SerialChannel(QObject *parent = nullptr);
    ~SerialChannel() override;

    // ICommunicationChannel 接口实现
    bool open() override;
    void close() override;
    bool isOpen() const override;
    Status getStatus() const override;

    qint64 send(const QByteArray &data) override;
    QByteArray receive(int timeoutMs = 1000) override;
    void clearBuffer() override;

    void setParameters(const QVariantMap &params) override;
    QVariantMap getParameters() const override;

    // 串口特定方法
    void setSerialMode(SerialMode mode);
    SerialMode getSerialMode() const;
    
    // 设置共享的SerialPortManager
    void setSerialPortManager(SerialPortManager* manager);
    
    static QStringList getAvailablePorts();

private slots:
    void handleDataReceived(const QString &portName, const QByteArray &data);
    void handlePortConnected(const QString &portName);
    void handlePortDisconnected(const QString &portName);
    void handlePortError(const QString &portName, const QString &error);

private:
    SerialPortManager* m_serialPortManager;
    SerialMode m_serialMode;
    QByteArray m_readBuffer;
    mutable QMutex m_mutex;
    QWaitCondition m_dataAvailable;
    Status m_status;
    
    // 串口参数
    QString m_portName;
    quint8 m_portIndex;  // 用于SerialPortManager的端口索引
    qint32 m_baudRate;
    QSerialPort::DataBits m_dataBits;
    QSerialPort::Parity m_parity;
    QSerialPort::StopBits m_stopBits;
    QSerialPort::FlowControl m_flowControl;
    
    // 连接管理
    bool m_isConnected;
    
    // RS485半双工控制（如果需要，可以在SerialPortWorker中实现）
    void enableTransmit();
    void enableReceive();
};

} // namespace Infrastructure

#endif // SERIALCHANNEL_H 