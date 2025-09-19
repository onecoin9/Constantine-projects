#ifndef JSONRPCSERVER_H
#define JSONRPCSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QMap>
#include <functional>
#include <QtConcurrent/QtConcurrent>
#include "AngKScanManager.h"
#include "DeviceModel.h"
#include "AngKGlobalInstance.h"
#include <QSet>
#include "AngKMessageHandler.h"
#include "../TaskManager/TaskManagerSingleton.h"
#include <QString>
#include "AngKProjDataset.h"
#include "nlohmann/json.hpp"


/*
 * JsonRpcServer:
 *
 * 这是一个基于 TCP 的 JSON-RPC 2.0 服务器，为简化设计，此类仅支持单个客户端连接。
 * 它采用长连接通讯。当执行耗时的命令时，服务器会先返回一个任务接受应答，
 * 任务完成后，服务器会通过现有长连接主动向客户端发送通知消息。
 *
 * 此类设计为单件模式，方便在应用程序的其他模块中统一调用。
 *
 * --- 客户端数据包构造指南 (Client Data Packet Construction Guide) ---
 *
 * 要与此服务器成功通信，客户端必须发送遵循以下结构的数据包：
 *
 * 1. 二进制头部 (32字节)
 *   ────────────────────────────────────────────────────────────────────────────────
 *   | 字段            | 长度 (字节) | 值 (均为大端序 Big-Endian)                      |
 *   ├─────────────────┼─────────────┼─────────────────────────────────────────────────┤
 *   | MagicNumber     | 4           | 固定为 0x4150524F (ASCII: "APRO")               |
 *   | Header Version  | 2           | 固定为 1                                        |
 *   | Payload Length  | 4           | 后续JSON消息体的字节长度                        |
 *   | Reserved        | 22          | 保留字段，当前应全部填充为0                     |
 *   ────────────────────────────────────────────────────────────────────────────────
 *
 * 2. JSON-RPC 2.0 消息体 (Payload)
 *    这部分是UTF-8编码的JSON字符串，其长度必须与头部中的 "Payload Length" 字段完全匹配。
 *    JSON对象必须遵循JSON-RPC 2.0规范。
 *
 * --- JSON消息体示例 ---
 *
 *    例如，调用 "LoadProject" 方法:
 *    {
 *        "jsonrpc": "2.0",
 *        "method": "LoadProject",
 *        "params": {
 *            "path": "C:/projects/my_project",
 *            "taskFileName": "task.json"
 *        }
 *    }
 * 服务器收到消息后，会执行相应操作。如果操作需要返回结果（例如获取信息或异步任务完成），
 * 服务器会通过一个新的通知（包含特定的方法名和数据）将结果发送给客户端。
 *
 */
class JsonRpcServer : public QTcpServer
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(JsonRpcServer)

public:
    // 获取单件实例
    bool RUNING_STATUS = false;
    static JsonRpcServer* Instance(QObject *parent = nullptr);
    virtual ~JsonRpcServer();
    // 启动和停止服务器
    bool Start(quint16 port);
    void Stop(); 
protected:
    // 处理新的客户端连接
    void incomingConnection(qintptr socketDescriptor) override;
public slots:
    // 接收来自CustomMessageHandler的原始设备消息并转发
    void OnForwardDeviceMessage(const QString& strIPHop, uint16_t BPUID, const QByteArray& message);
    // 接收来自其他模块的异步执行结果，并通知客户端
    void SendNotification(const QString& method, const nlohmann::json& params);
    void SendErrorNotification(int code, const std::string& message);
private slots:
    // 读取客户端数据
    void OnReadyRead();
    // 处理客户端断开连接
    void OnSocketDisconnected();
    // 当扫描到设备时，通知客户端
signals:
    // UI交互信号
    void SiteScanAndConnect();
    void CloseSiteScanAndConnectUI();
    void sgnLoadProjectFile2UI(QString filepath);
    void sgnLoadProjectBegin();
private:
    explicit JsonRpcServer(QObject *parent = nullptr);
    // 注册所有支持的RPC方法处理器
    void RegisterHandlers();
    // 消息处理流程
    void ProcessPacket(const QByteArray &payload);
    void ProcessJsonRpcRequest(const nlohmann::json &doc); 
    
    // 消息发送
    void SendMessageToClient(const nlohmann::json &doc); 
    void TranslateSubCmd2String(ChipOperCfgSubCmdID subCmd, std::string& subCmdStr);
    
    // RPC方法处理器类型定义
    using RpcHandler = std::function<void(const nlohmann::json&)>;

    // 各RPC方法的具体实现
    void HandleDoCustom(const nlohmann::json &params);
    void HandleSiteScanAndConnect(const nlohmann::json &params);
    void HandleLoadProject(const nlohmann::json &params);
    void HandleGetProjectInfo(const nlohmann::json &params);
    void HandleDoJob(const nlohmann::json &params);
    void HandleGetProjectInfoExt(const nlohmann::json &params);
    void HandleGetSKTInfo(const nlohmann::json& params);
    void HandleSendUUID(const nlohmann::json& params);
    // 成员变量
    QTcpSocket* m_clientSocket = nullptr; // 唯一的客户端连接
    QByteArray m_buffer;                  // 用于接收数据的缓冲区
    bool m_isScanSubscribed = false;      // 客户端是否订阅了扫描通知
    QHash<QString, RpcHandler> m_handlers; // RPC方法处理器映射
    
    static JsonRpcServer* m_instance;      // 单例实例
};

// 全局单例访问函数
JsonRpcServer* GetGlobalJsonRPCServerApp();

#endif // JSONRPCSERVER_H