#pragma once

#include "domain/IDevice.h"
#include "domain/protocols/ISProtocol.h"
#include "infrastructure/ICommunicationChannel.h"
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include <QMutex>
#include <QTimer> // 添加QTimer头文件
#include <QDateTime> // 添加QDateTime头文件

// 前向声明
namespace Domain::Protocols {
    class SProtocol;
}

namespace Services {
    class DutManager;
}

namespace Domain {

/**
 * @brief 自动化处理设备类
 * 
 * 该类使用S协议与自动化设备通信，采用分层架构设计
 * 包含：SProtocol(协议层) + TcpChannel(通信层) + SProtocolFramer(帧处理层)
 */
class HandlerDevice : public IDevice
{
    Q_OBJECT
public:
    explicit HandlerDevice(QObject *parent = nullptr);
    ~HandlerDevice() override;
    
    // IDevice 接口实现
    bool initialize() override;
    bool release() override;
    DeviceStatus getStatus() const override;
    QString getName() const override;
    DeviceType getType() const override;
    QString getDescription() const override;
    QJsonObject executeCommand(const QString &command, const QJsonObject &params) override;
    bool selfTest() override;
    
    void setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel> channel) override;
    std::shared_ptr<Infrastructure::ICommunicationChannel> getCommunicationChannel() const override;
    void setConfiguration(const QJsonObject &config) override;
    QJsonObject getConfiguration() const override;
    
    // Handler 特定功能
    //bool loadTask(const QString &taskData);
    bool tellDevReady(const QJsonObject &readyInfo);
    bool tellDevCommVersion(int version);
    bool querySiteEnable();
    bool setDoneSite(int siteIdx, const QByteArray &result);
    QJsonObject getSiteMap() const;
    bool sendAxisMoveCommand(const QString &axisSelect, int siteIdx, int targetAngle);
    bool sendContactCheckResult(int siteIdx, int adapterCnt, const QByteArray &result);
    bool sendRemainingCheckResult(int siteIdx, int adapterCnt, const QByteArray &result);
    bool sendRawFrame(const QByteArray &frame);
    
    // 获取S协议实例
    Domain::Protocols::ISProtocol* getSProtocol() const;
    
    // 【新增】心跳相关
    bool isLinkHealthy() const;

    // 删除setDutManager方法，因为现在使用单例模式

signals:
    // 【新增】链路健康状态变化信号
    void linkHealthChanged(bool isHealthy);

    // 芯片放置信号
    void chipPlaced(int siteIndex, uint32_t slotEn, const QString &siteSn);
    
    // 接触检查请求
    void contactCheckRequested(int siteIdx, const QByteArray &sktEn);
    
    // 残料检查请求
    void remainingCheckRequested(int siteIdx, const QByteArray &sktEn);
    
    // 编程结果反馈
    void programResultReceived(bool success, int errCode, const QString &errMsg);
    
    // 轴移动请求
    void axisMovementRequested(const QString &axisSelect, int siteIdx, int targetAngle);
    
    // 轴移动完成
    void axisMovementCompleted(const QString& axisSelect, int siteIdx, int currentAngle, int result, const QString& errMsg);
    
    /**
     * @brief 当发送测试结果时发射
     * @param siteIndex 站点索引
     * @param resultData 结果数据（每个字节表示一个socket的结果）
     */
    void testResultSent(int siteIndex, const QByteArray& resultData);
    
    // 原始数据帧
    void rawFrameSent(const QByteArray &frame);
    void rawFrameReceived(const QByteArray &frame);
    
    // 调试消息
    void debugMessage(const QString &message);
    
    // Handler通知：已将芯片放置到指定工位
    void chipsPlaced(const QString& siteId, const QStringList& dutIds);
    // Handler通知：转台移动完成
    void turntableMoveCompleted(const QString& siteId);
    // Handler通知：准备好开始测试/激活流程
    void readyForProcess(const QString& processType); // e.g., "activation", "test"
    
    // 产品信息信号
    void productInfoReceived(const QJsonArray& rotateInfo);

private slots:
    // 处理来自S协议的信号
    void onPduCommandReceived(uint16_t pflag, uint8_t pdu, const QByteArray& data);
    void onJsonCommandReceived(const QJsonObject& command);
    void onVersionQueryReceived();
    void onChipPlacementReceived(int siteIdx, const QByteArray& chipData);
    void onICStatusCheckRequested(int siteIdx, const QByteArray& checkData);
    void onRemainingCheckRequested(int siteIdx, const QByteArray& checkData);
    void onSiteEnableReceived(const QByteArray& enableData);
    void onAxisMovementRequested(const QString& axisSelect, int siteIdx, int targetAngle);
    void onAxisMovementCompleted(const QString& axisSelect, int siteIdx, int currentAngle, int result);
    void onProductInfoReceived(const QJsonArray& rotateInfo);
    void onProtocolConnected();
    void onProtocolDisconnected();
    void onProtocolError(const QString& error);

    // 【新增】心跳定时器槽
    void onHeartbeatTimeout();

private:
    // 辅助方法
    bool initializeProtocol();
    void connectSignals();
    void setupCommunicationChannel();
    QByteArray buildPdu63Data(const QJsonObject& readyInfo);
    QString buildCmd3Command(const QString& taskPath, const QJsonObject& params);
    
    // 成员变量
    std::unique_ptr<Domain::Protocols::SProtocol> m_sProtocol;
    mutable QMutex m_mutex;
    QString m_lastError;
    int m_lastErrorCode;
    bool m_isConnected;
    
    // 【新增】心跳相关成员
    QTimer* m_heartbeatTimer;
    bool m_isLinkHealthy;
    QDateTime m_lastSuccessfulHeartbeat;

    // 配置信息
    QJsonObject m_configuration;
    DeviceStatus m_status;
    bool m_isSimulation;
    // 删除m_dutManager成员变量
};

} // namespace Domain
