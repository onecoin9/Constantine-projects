#ifndef MT422SERIALHANDLER_H
#define MT422SERIALHANDLER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <memory>
#include <QJsonObject>
#include <QThread>
#include <QMutex>
#include "SerialPortManager.h"
// 串口读取线程类
class SerialPortReader : public QThread
{
    Q_OBJECT
public:
    explicit SerialPortReader(quint8 portIndex,QString portName, QObject *parent = nullptr);
    void stop();
public slots:
    void handleWriteRequest(quint8 portIndex, const QByteArray& data); // 子线程执行写入（异步）
    
signals:
    void dataReceived(const QString& strIPHop, const uint16_t BPUID, const quint8 index,const QByteArray &data);
    void error(const QString &errorMessage);

protected:
    void run() override;

private:
    QSerialPort* m_port;
    quint8 m_portIndex;
    bool m_running;
    QString m_portName;
    QMutex m_mutex;
};

class MT422SerialHandler : public QObject
{
    Q_OBJECT
    
private:
    // 单例模式：构造函数和析构函数设为私有
    explicit MT422SerialHandler(QObject *parent = nullptr);
    ~MT422SerialHandler();
    
    // 禁止拷贝构造和赋值操作
    MT422SerialHandler(const MT422SerialHandler&) = delete;
    MT422SerialHandler& operator=(const MT422SerialHandler&) = delete;

public:
    // 获取单例实例的静态方法
    static MT422SerialHandler& instance();
    
    // 销毁单例实例（可选）
    static void destroyInstance();

    // 从JSON文件加载串口配置
    bool loadPortConfiguration(const QString &configFile);
    std::shared_ptr<QSerialPort> createAndMovePort(quint8 portIndex, QThread* targetThread);
    
    // 配置单个串口
    bool configurePort(quint8 portNum,quint8 portIndex, const QString &portName, int baudRate);
    
    // 关闭指定串口
    void closePort(quint8 portIndex);

    // 获取串口实例（如果不存在则创建）
    std::shared_ptr<QSerialPort> getPort(quint8 portIndex);
    
    // 关闭所有串口
    void closeAllPorts();
    
    // 检查指定串口是否打开
    bool isPortOpen(quint8 portIndex) const;
    
    // 发送消息（会根据消息第一个字节选择串口）
    bool sendMessage(const QString& strIPHop, const uint16_t BPUID, const quint8 index,const QByteArray &message);
    bool receiveMessage(const QString& portName, const QByteArray& data);
    static const quint32 MT422_TAG_ID = 0x4D543432; // "MT422"

    void getPortMapping(quint8 portIndex, QString& strIPHop, uint16_t& bpuid ,quint8& index) const;

signals:
    void messageReceived(const QString& strIPHop, const uint16_t BPUID, quint8 index,const QByteArray &message);
    void messageSent(bool success);
    void errorOccurred(const QString &errorMessage);
    void writeRequested(quint8 portIndex, const QByteArray& data);  // 新增信号
    void sendTo(const QByteArray& data);
    
private slots:
    void handleError(QSerialPort::SerialPortError error);
    void handleE(const QString& portName, const QByteArray& data);
    // 新增：处理串口接收到的数据
    void handleReceivedData(const QString& strIPHop, const uint16_t BPUID, quint8 index, const QByteArray &data);
   
    quint8 getPortIndex(const QString& strIPHop, uint16_t bpu, quint8 index);
public slots:
    void slotTaskDownLoadStatus2(std::string devIP, uint32_t HopNum);
private:
    // 新增的辅助函数声明
    quint8 extractPortIndex(const QString& portName);
    // 格式化消息
    QByteArray formatMessage(const QByteArray &message) const;
    
    // 计算CRC16
    quint16 calculateCRC16(const QByteArray &data) const;
    // 计算CRC32
    quint32 calculateCRC32(const QByteArray& data) const;
    
    // 写入数据到指定串口
    bool writeToPort(quint8 portIndex, const QByteArray &data);
    void findKeysByPortIndex(quint8 portIndex, QString& outStrIPHop, QString& outBpu, QString& outIndex);
   

    // 验证接收到的消息格式
    bool validateReceivedMessage(const QByteArray &data, QByteArray &payload) const;
    
    // 组装自定义数据包（按小端格式）
    QByteArray assembleCustomData(uint8_t cmdId, uint16_t dataLength, const QByteArray& payloadData) const;

private:
    // 静态单例实例指针
    static MT422SerialHandler* s_instance;
    static QMutex s_instanceMutex;
    
    static const int MAX_PORTS = 80;  // 最大支持的串口数量
    
    // 存储串口索引到串口实例的映射
    QMap<quint8, QSerialPort*> m_serialPorts;
    
    // 存储串口配置信息
    QMap<QString, QJsonObject> m_portConfig;    // 端口配置信息，key是端口索引字符串

    uint32_t jsonConfigBaudrate = 921600;

    // 存储串口读取线程
    QMap<quint8, QSerialPort*> m_ports;

    QMap<QString, quint8> m_ipHopToPortMap;     // strIPHop到端口索引的映射
    QMap<uint16_t, quint8> m_bpuToPortMap;     // BPU到端口索引的映射
     // 使用 QMap 存储 strIPHop + BPU + Index 到端口索引的映射
    QMap<QString, quint8> m_strIPHopBpuIndexToPortMap;
    SerialPortManager* m_serialportmanager;
    uint64_t test = 0;
};

#endif // MT422SERIALHANDLER_H