#ifndef SPROTOCOL_H
#define SPROTOCOL_H

#include "ISProtocol.h"
#include <QObject>
#include <QJsonObject>
#include <QTimer>

namespace Infrastructure {
    class TcpChannel;
    class TcpServerChannel;
}

namespace Domain::Protocols {

/**
 * @brief S协议实现
 * 通信架构：
 * - CMD3命令：端口1000发送（短连接）- 使用TcpChannel单例
 * - 其他命令：端口64100发送（短连接）- 使用TcpChannel单例
 * - 响应接收：端口64101监听（服务器一直开启）- 使用TcpServerChannel单例
 */
class SProtocol : public ISProtocol
{
    Q_OBJECT

public:
    explicit SProtocol(QObject *parent = nullptr);
    ~SProtocol() override;

    // ISProtocol接口实现
    bool initialize(const QJsonObject& config) override;
    void release() override;
    bool isConnected() const override;
    QString getLastError() const override;

    // 基础发送接口
    bool sendCmd3Command(const QString& command) override;
    bool sendPduCommand(uint16_t pflag, uint8_t pdu, const QByteArray& data) override;
    bool sendAxisMoveCommand(const QJsonObject& command) override;
    bool sendJsonCommand(const QJsonObject& command) override;

    // 响应发送接口
    bool sendResponse(const QByteArray& response) override;
    bool sendJsonResponse(const QJsonObject& response) override;
    bool sendPduResponse(uint16_t pflag, uint8_t pdu, const QByteArray& data) override;

    // 响应发送辅助方法
    void sendAck(uint8_t pdu, uint8_t errorCode = 0);             // 发送ACK响应
private slots:
    // 服务器通道事件处理
    void onServerDataReceived(const QByteArray& data);
    void onServerClientConnected(const QString& clientInfo);
    void onServerClientDisconnected(const QString& clientInfo);
    void onServerError(const QString& error);

private:
    // 通信相关
    void processReceivedData(const QByteArray& data);
    bool tryParseJsonCommand(const QByteArray& data);
    bool tryParsePduCommand(const QByteArray& data);
    QByteArray buildPduPacket(uint16_t pflag, uint8_t pdu, const QByteArray& data);
    uint8_t calculateCRC(const QByteArray& data);

    // PDU业务处理方法 - 根据协议文档
    void handleQueryVersion(const QByteArray& data);               // 处理0xE1
    void handleTellChipSen(const QByteArray& data);                // 处理0xE6
    void handleQueryICStatus(const QByteArray& data);              // 处理0xE8
    void handleQueryRemaining(const QByteArray& data);             // 处理0xE5
    void handleTellSiteEn(const QByteArray& data);                 // 处理0xE4
    void handleTellScanInfo(const QByteArray& data);               // 处理0xE9
    
    // JSON业务处理方法
    void handleAxisMoveResponse(const QJsonObject& response);      // 处理AxisMove响应
    void handleAxisMoveCommand(const QJsonObject& command);        // 处理AxisMove
    void handleAxisMoveComplete(const QJsonObject& command);       // 处理AxisMoveComplete
    void handleProductInfo(const QJsonObject& command);            // 处理ProductInfo指令
    
    
    // 调试日志辅助方法
    QString formatFrameData(const QByteArray& data);              // 格式化帧数据为十六进制字符串
    void logPduDetails(const QByteArray& packet, bool isSending);

    // 通道实例（单例）
    Infrastructure::TcpChannel* m_clientChannel;        // 短连接发送通道
    Infrastructure::TcpServerChannel* m_serverChannel;  // 监听接收通道

    // 配置参数
    QJsonObject m_config;
    QString m_vautoHost;
    int m_cmd3Port;
    int m_commandPort;
    int m_serverPort;
    int m_connectTimeout;

    // 状态
    QString m_lastError;
    bool m_isInitialized;
    int m_currentVersion;
};

} // namespace Domain::Protocols

#endif // SPROTOCOL_H 
