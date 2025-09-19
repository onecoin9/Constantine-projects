#ifndef ISPROTOCOL_H
#define ISPROTOCOL_H

#include <QObject>
#include <QByteArray>
#include <QJsonObject>
#include <string>
#include <memory>

namespace Infrastructure {
    class ICommunicationChannel;
}

namespace Domain {
namespace Protocols {

/**
 * @brief S协议接口
 * 通信架构：
 * - CMD3命令：端口1000发送（短连接）
 * - 其他命令：端口64100发送（短连接）
 * - 响应接收：端口64101监听（服务器一直开启）
 */
class ISProtocol : public QObject
{
    Q_OBJECT
public:
    explicit ISProtocol(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ISProtocol() = default;
    
    // 协议常量定义
    enum ProtocolFlag {
        PFLAG_SA = 0x5341,  // StdMes → AutoApp
        PFLAG_AS = 0x4153   // AutoApp → StdMes
    };
    
    // PDU命令定义
    enum PDUCommand {
        // StdMes发送的命令 (PFLAG = SA)
        PDU_TELLVERSION = 0x61,      // 告知版本信息
        PDU_TELLDEVINIT = 0x63,      // 告知初始化情况  
        PDU_TELLRESULTS = 0x67,      // 告知芯片烧录结果
        PDU_TELLICSTATUS = 0x68,     // 告知接触检查结果
        PDU_TELLREMAINING = 0x65,    // 告知残料检查结果
        PDU_TELLPRINT = 0x70,    // 告知镭雕内容
        
        // AutoApp发送的命令 (PFLAG = AS)
        PDU_QUERYVERSION = 0xE1,     // 请求版本信息
        PDU_TELLCHIPSEN = 0xE6,      // 告诉芯片放置情况
        PDU_QUERYICSTATUS = 0xE8,    // 请求接触检查
        PDU_QUERYREMAINING = 0xE5,   // 请求残料检查
        PDU_TELLSITEEN = 0xE4        // 告知站点使能
    };
    
    // 初始化和配置
    virtual bool initialize(const QJsonObject& config) = 0;
    virtual void release() = 0;
    virtual bool isConnected() const = 0;
    virtual QString getLastError() const = 0;
    
    // 基础发送接口
    virtual bool sendCmd3Command(const QString& command) = 0;                    // 通过1000端口
    virtual bool sendPduCommand(uint16_t pflag, uint8_t pdu, const QByteArray& data) = 0;  // 通过64100端口
    virtual bool sendAxisMoveCommand(const QJsonObject& command) = 0;           // 通过64100端口
    virtual bool sendJsonCommand(const QJsonObject& command) = 0;               // 通过64100端口
    
    // 响应发送接口（通过64101服务器端口）
    virtual bool sendResponse(const QByteArray& response) = 0;
    virtual bool sendJsonResponse(const QJsonObject& response) = 0;
    virtual bool sendPduResponse(uint16_t pflag, uint8_t pdu, const QByteArray& data) = 0;

signals:
    // 基础数据接收信号
    void pduCommandReceived(uint16_t pflag, uint8_t pdu, const QByteArray& data);
    void jsonCommandReceived(const QJsonObject& command);
    void rawDataReceived(const QByteArray& data);
    
    // PDU业务信号 - 根据协议文档定义
    void versionQueryReceived();                                               // 收到0xE1
    void chipPlacementReceived(int siteIdx, const QByteArray& chipData);      // 收到0xE6
    void icStatusCheckRequested(int siteIdx, const QByteArray& checkData);    // 收到0xE8
    void remainingCheckRequested(int siteIdx, const QByteArray& checkData);   // 收到0xE5
    void siteEnableReceived(const QByteArray& enableData);                    // 收到0xE4
    
    // 轴移动JSON业务信号
    void axisMovementRequested(const QString& axisSelect, int siteIdx, int targetAngle);   // 收到AxisMove
    void axisMovementCompleted(const QString& axisSelect, int siteIdx, int currentAngle, int result);  // 收到AxisMoveComplete
    
    // 产品信息JSON业务信号
    void productInfoReceived(const QJsonArray& rotateInfo); // 收到ProductInfo

    // 状态信号
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
};

} // namespace Protocols
} // namespace Domain

#endif // ISPROTOCOL_H 