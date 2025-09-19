#include "JsonRpcServer.h"
#include "AngkLogger.h"
#include <QtEndian>
#include <QTimer>
#include <QMessageBox>
#include <QUuid>
#include <QCoreApplication>
#include <QNetworkAddressEntry>
#include <QtConcurrent/QtConcurrent>
#include <QThread>
#include "nlohmann/json.hpp"

// --- 静态常量和全局变量 ---

// 单件实例初始化
JsonRpcServer* JsonRpcServer::m_instance = nullptr;

// 协议头部定义
static const quint32 MAGIC_NUMBER = 0x4150524F; // "APRO"
static const quint16 HEADER_VERSION = 1;         // 协议版本号
static const int HEADER_LENGTH = 32;             // 头部总长度

// JSON-RPC 版本
static const std::string JSONRPC_VERSION_STD = "2.0";

// --- 辅助函数 ---

// 安全地从 JSON 对象中获取字符串
static bool getJsonString(const nlohmann::json& data, const char* key, std::string& outVal) {
    if (data.contains(key) && data.at(key).is_string()) {
        outVal = data.at(key).get<std::string>();
        return !outVal.empty();
    }
    return false;
}

// 安全地从 JSON 对象中获取数值
template<typename T>
static bool getJsonNumber(const nlohmann::json& data, const char* key, T& outVal) {
    if (data.contains(key) && data.at(key).is_number()) {
        outVal = data.at(key).get<T>();
        return true;
    }
    return false;
}

// --- 单例模式实现 ---

// 全局访问函数，获取 JsonRpcServer 单例
JsonRpcServer* GetGlobalJsonRPCServerApp() {
    return JsonRpcServer::Instance(QCoreApplication::instance());
}

// 获取单例实例，如果不存在则创建
JsonRpcServer* JsonRpcServer::Instance(QObject *parent)
{
    if (!m_instance) {
        m_instance = new JsonRpcServer(parent);
    }
    return m_instance;
}

// --- 构造与析构 ---

JsonRpcServer::JsonRpcServer(QObject *parent)
    : QTcpServer(parent)
{
    RegisterHandlers(); // 初始化RPC方法处理器
    qRegisterMetaType<nlohmann::json>("nlohmann::json");
    
}

JsonRpcServer::~JsonRpcServer()
{
    RUNING_STATUS = false;
    Stop();
}

// --- 服务器控制 ---

// 启动服务器，开始监听指定端口
bool JsonRpcServer::Start(quint16 port)
{
    if (RUNING_STATUS) {
        ALOG_WARN("服务器已在运行中，端口：%u", "CU", "--", serverPort());
        return true;
    }
    bool res = this->listen(QHostAddress::Any, port);
    if (!res) {
        ALOG_FATAL("服务器启动失败，无法监听端口 %u: %s", "CU", "--", port, errorString().toStdString().c_str());
    } else {
        RUNING_STATUS = true;
        ALOG_INFO("服务器已启动，正在监听端口 %u", "CU", "--", port);
        // 连接信号，用于设备扫描发现
        connect(&AngKScanManager::instance(), &AngKScanManager::SendNotification, this, &JsonRpcServer::SendNotification);
    }
    return res;
}

// 停止服务器，断开所有连接
void JsonRpcServer::Stop()
{
    if (m_clientSocket) {
        ALOG_INFO("正在断开客户端连接: %s:%u", "CU", "--",
                  m_clientSocket->peerAddress().toString().toStdString().c_str(),
                  m_clientSocket->peerPort());
        m_clientSocket->disconnectFromHost();
    }
    this->close();
    ALOG_INFO("服务器已停止", "CU", "--");
}

// --- 连接处理 ---

// 处理新的客户端连接请求
void JsonRpcServer::incomingConnection(qintptr socketDescriptor)
{
    if (m_clientSocket && m_clientSocket->isOpen()) {
        ALOG_WARN("拒绝新的客户端连接，因为已有客户端连接", "CU", "--");
        // 临时创建一个socket来关闭它
        QTcpSocket tempSocket;
        tempSocket.setSocketDescriptor(socketDescriptor);
        tempSocket.disconnectFromHost();
        return;
    }

    m_clientSocket = new QTcpSocket(this);
    if (!m_clientSocket->setSocketDescriptor(socketDescriptor)) {
        ALOG_ERROR("为新连接设置套接字描述符失败", "CU", "--");
        delete m_clientSocket;
        m_clientSocket = nullptr;
        return;
    }

    m_buffer.clear();
    m_isScanSubscribed = false;

    connect(m_clientSocket, &QTcpSocket::readyRead, this, &JsonRpcServer::OnReadyRead);
    connect(m_clientSocket, &QTcpSocket::disconnected, this, &JsonRpcServer::OnSocketDisconnected);

    ALOG_INFO("接受来自客户端 %s:%u 的连接", "CU", "--",
              m_clientSocket->peerAddress().toString().toStdString().c_str(),
              m_clientSocket->peerPort());
}

// 处理客户端断开连接
void JsonRpcServer::OnSocketDisconnected()
{
    if (m_clientSocket) {
        ALOG_INFO("客户端 %s:%u 已断开连接", "CU", "--",
                  m_clientSocket->peerAddress().toString().toStdString().c_str(),
                  m_clientSocket->peerPort());
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
        m_buffer.clear();
        m_isScanSubscribed = false;
    }
}

// --- 数据收发 ---

// 当有数据可读时被调用
void JsonRpcServer::OnReadyRead()
{
    if (!m_clientSocket) return;

    m_buffer.append(m_clientSocket->readAll());
    ALOG_INFO("从客户端读取数据，当前缓冲区大小: %d", "CU", "--", m_buffer.size());

    while (true) {
        if (m_buffer.size() < HEADER_LENGTH) {
            ALOG_DEBUG("缓冲区数据不足以解析头部，等待更多数据", "CU", "--");
            break; // 数据不够一个完整的头部
        }

        // 解析头部
        QByteArray header = m_buffer.left(HEADER_LENGTH);
        quint32 receivedMagic = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(header.constData()));
        quint16 version = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(header.constData() + 4));
        quint32 payloadLength = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(header.constData() + 6));

        // 验证头部信息
        if (receivedMagic != MAGIC_NUMBER) {
            ALOG_ERROR("无效的Magic Number (0x%X)，断开客户端连接", "CU", "--", receivedMagic);
            m_clientSocket->disconnectFromHost();
            return;
        }
        if (version != HEADER_VERSION) {
            ALOG_ERROR("不支持的协议版本 (%u)，期望版本 %u，断开客户端连接", "CU", "--", version, HEADER_VERSION);
            m_clientSocket->disconnectFromHost();
            return;
        }
        
        const quint32 MAX_PAYLOAD_LENGTH = 16 * 1024 * 1024; // 16MB
        if (payloadLength > MAX_PAYLOAD_LENGTH) {
            ALOG_ERROR("数据包长度 (%u) 超过最大限制 (%u)，断开客户端连接", "CU", "--", payloadLength, MAX_PAYLOAD_LENGTH);
            m_clientSocket->disconnectFromHost();
            return;
        }

        // 检查数据包是否完整
        int totalPacketSize = HEADER_LENGTH + payloadLength;
        if (m_buffer.size() < totalPacketSize) {
            ALOG_DEBUG("数据包不完整，等待更多数据。需要 %d, 当前 %d", "CU", "--", totalPacketSize, m_buffer.size());
            break; // 数据不完整
        }

        // 提取并处理数据包
        QByteArray payload = m_buffer.mid(HEADER_LENGTH, payloadLength);
        ProcessPacket(payload);

        // 从缓冲区移除已处理的数据
        m_buffer.remove(0, totalPacketSize);
        ALOG_INFO("成功处理一个数据包，剩余缓冲区大小: %d", "CU", "--", m_buffer.size());
    }
}

// 发送消息到客户端
void JsonRpcServer::SendMessageToClient(const nlohmann::json &doc)
{
    if (!m_clientSocket || m_clientSocket->state() != QAbstractSocket::ConnectedState) {
        ALOG_WARN("无法发送消息：客户端未连接", "CU", "--");
        return;
    }

    std::string payload_str = doc.dump();
    QByteArray payload = QByteArray::fromStdString(payload_str);
    
    QByteArray packet;
    packet.reserve(HEADER_LENGTH + payload.size());
    
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << MAGIC_NUMBER;
    stream << HEADER_VERSION;
    stream << static_cast<quint32>(payload.size());
    packet.append(QByteArray(22, '\0')); // 22字节的保留字段
    packet.append(payload);

    qint64 bytesWritten = m_clientSocket->write(packet);
    if (bytesWritten != packet.size()) {
        ALOG_ERROR("发送数据到客户端失败，可能连接已断开", "CU", "--");
    } else {
        ALOG_DEBUG("成功发送数据包，大小: %d", "CU", "--", packet.size());
    }
}

// --- RPC 消息处理 ---

// 解析数据包并分发
void JsonRpcServer::ProcessPacket(const QByteArray &payload)
{
    nlohmann::json doc;
    try {
        doc = nlohmann::json::parse(payload.constData(), payload.constData() + payload.size());
    } catch (const nlohmann::json::parse_error& e) {
        ALOG_ERROR("JSON 解析失败: %s", "CU", "--", e.what());
        SendErrorNotification(-32700, std::string("解析错误: ") + e.what());
        return;
    }
    ProcessJsonRpcRequest(doc);
}

// 处理 JSON-RPC 请求
void JsonRpcServer::ProcessJsonRpcRequest(const nlohmann::json &doc)
{
    ALOG_DEBUG("接收到请求: %s", "CU", "--", doc.dump().c_str());
    if (!doc.is_object()) {
        SendErrorNotification(-32600, "无效的请求：不是一个JSON对象");
        return;
    }
    if (doc.value("jsonrpc", "") != JSONRPC_VERSION_STD) {
        SendErrorNotification(-32600, "无效的请求：jsonrpc版本必须是 '2.0'");
        return;
    }
    std::string method_str;
    if (!getJsonString(doc, "method", method_str)) {
        SendErrorNotification(-32601, "无效的请求：缺少 'method' 字段或字段非字符串");
        return;
    }
    // 提取 params
    nlohmann::json params_val = doc.value("params", nlohmann::json::object());
    // 查找并调用处理器
    RpcHandler handler = m_handlers.value(QString::fromStdString(method_str));
    if (handler) {
        try {
            handler(params_val);
        } catch (const std::exception& e) {
            ALOG_ERROR("处理方法 '%s' 时发生异常: %s", "CU", "--", method_str.c_str(), e.what());
            SendErrorNotification(-32000, std::string("服务器内部错误: ") + e.what());
        }
    } else {
        ALOG_WARN("未找到方法: %s", "CU", "--", method_str.c_str());
        SendErrorNotification(-32601, "方法未找到");
    }
}

void JsonRpcServer::SendErrorNotification(int code, const std::string &message)
{
    nlohmann::json params;
    params["code"] = code;
    params["message"] = message;
    SendNotification("Error", params);
}
// 发送通知（无ID的请求）
void JsonRpcServer::SendNotification(const QString &method, const nlohmann::json &result)
{
    nlohmann::json notification;
    notification["jsonrpc"] = JSONRPC_VERSION_STD;
    notification["method"] = method.toStdString();
    nlohmann::json temp_result = result;
    
    if (method.toStdString() == "setLoadProjectResult" && temp_result["result"]){
        auto map_ptr = TaskManagerSingleton::getInstance().getAllProjInfo();
        if (map_ptr) {
            ALOG_INFO("Project Info (Key and First String in Pair):", "CU", "--");
            for (auto it = map_ptr->constBegin(); it != map_ptr->constEnd(); ++it) {
                const QString& key_qstr = it.key();
                const QPair<QString, ACProjManager*>& pair_value = it.value();
                const QString& first_string_in_pair_qstr = pair_value.first;
                std::string key_std = key_qstr.toStdString();
                std::string first_string_std = first_string_in_pair_qstr.toStdString();
                ALOG_INFO("Key: %s, Pair's First String: %s ", "CU", "--",key_std.c_str(), first_string_std.c_str());
                    AngKProjDataset* projDataset = TaskManagerSingleton::getInstance().getProjDataset(key_qstr);
                    nlohmann::json proj;
                    proj["pro_url"] = key_std;
                    proj["pro_chipdata"] = projDataset->getChipData().DataJsonSerial();
                    proj["doCmdSequenceArray"] = nlohmann::json::array();
                    OpInfoList& varOper = projDataset->GetOperInfoList();
                    for (int i = 0; i < varOper.size(); i++) {
                        nlohmann::json DoCmdSequenceJson;
                        DoCmdSequenceJson["CmdSequencesGroupCnt"] = varOper[i].vecOpList.size();
                        DoCmdSequenceJson["CmdRun"] = varOper[i].strOpName;
                        DoCmdSequenceJson["CmdID"] = QString::number(varOper[i].iOpId, 16).toUpper().toStdString();
                        DoCmdSequenceJson["CmdSequences"] = nlohmann::json::array();
                        for (int j = 0; j < varOper[i].vecOpList.size(); ++j)
                        {
                            nlohmann::json cmdJson;
                            std::string strSubOper;
                            TranslateSubCmd2String((ChipOperCfgSubCmdID)varOper[i].vecOpList[j], strSubOper);
                            cmdJson["ID"] = QString::number(varOper[i].vecOpList[j], 16).toUpper().toStdString();
                            cmdJson["Name"] = strSubOper;
                            DoCmdSequenceJson["CmdSequences"].push_back(cmdJson);
                        }
                        proj["doCmdSequenceArray"].push_back(DoCmdSequenceJson);
                    }
                    temp_result["proInfo"] = proj;
            }
        }
    }
    
    // 将修改后的temp_result赋值给notification["result"]
    notification["result"] = temp_result;
    SendMessageToClient(notification);
    ALOG_INFO("发送通知: method=%s, params=%s", "CU", "--", method.toStdString().c_str(), result.dump().c_str());
}
// --- 业务逻辑与RPC方法实现 ---
// 注册所有RPC方法处理器
void JsonRpcServer::RegisterHandlers()
{
    // 使用 bind 将成员函数适配为 RpcHandler
    m_handlers["DoCustom"] = std::bind(&JsonRpcServer::HandleDoCustom, this, std::placeholders::_1);
    m_handlers["DoJob"] = std::bind(&JsonRpcServer::HandleDoJob, this, std::placeholders::_1);
    m_handlers["LoadProject"] = std::bind(&JsonRpcServer::HandleLoadProject, this, std::placeholders::_1);
    m_handlers["SiteScanAndConnect"] = std::bind(&JsonRpcServer::HandleSiteScanAndConnect, this, std::placeholders::_1);
    m_handlers["GetProjectInfo"] = std::bind(&JsonRpcServer::HandleGetProjectInfo, this, std::placeholders::_1);
    m_handlers["GetProjectInfoExt"] = std::bind(&JsonRpcServer::HandleGetProjectInfoExt, this, std::placeholders::_1);
    m_handlers["GetSKTInfo"] = std::bind(&JsonRpcServer::HandleGetSKTInfo, this, std::placeholders::_1);
    m_handlers["SendUUID"] = std::bind(&JsonRpcServer::HandleSendUUID, this, std::placeholders::_1);
}
// --- 具体RPC方法实现 ---
void JsonRpcServer::HandleDoCustom(const nlohmann::json &params)
{
    // 执行自定义命令
    //params必须提供三个参数:
    // strIP:烧录的site对应的IP
    // sktEn:需要烧录的座子的位图
    // doCustomData:需要执行的自定义数据，推荐为jsonobject
    
    ALOG_INFO("HandleDoCustom开始执行，接收到参数: %s", "CU", "--", params.dump().c_str());
    
    try {
        // 正确地从JSON获取参数值
        std::string strIP = params["strIP"].get<std::string>();
        uint32_t sktEn = params["sktEn"].get<uint32_t>();
        nlohmann::json doCustomData = params["doCustomData"];
        
        // 将JSON对象序列化为字符串，然后转为QByteArray
        std::string customDataStr = doCustomData.dump();
        QByteArray customDataBytes = QByteArray::fromStdString(customDataStr);
        
        ALOG_INFO("准备执行自定义命令: IP=%s, sktEn=%u, 自定义数据=%s", "CU", "--", 
                  strIP.c_str(), sktEn, customDataStr.c_str());
        
        int cmdResult = AngKMessageHandler::instance().Command_RemoteDoPTCmd(
            strIP, 0, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_DoCustom, sktEn, 8,
            customDataBytes
        );
    } catch (const std::exception& e) {
        ALOG_ERROR("HandleDoCustom执行失败: %s", "CU", "--", e.what());
        
        nlohmann::json errorResult;
        errorResult["result"] = false;
        errorResult["error"] = e.what();
        SendNotification("setDoCustomResult", errorResult);
    }
}
void JsonRpcServer::HandleDoJob(const nlohmann::json &params)
{
    // 执行命令
    //params必须提供三个参数:
    // strIP:烧录的site对应的IP
    // sktEn:需要烧录的座子的位图
    // docmdSeqJson:需要执行的烧录命令序列
    
    ALOG_INFO("HandleDoJob开始执行，接收到参数: %s", "CU", "--", params.dump().c_str());
    
    try {
        // 正确地从JSON获取字符串值
        std::string strIP = params["strIP"].get<std::string>();
        uint32_t sktEn = params["sktEn"].get<uint32_t>();
        nlohmann::json docmdSeqJson = params["docmdSeqJson"];
        
        // 将JSON对象序列化为字符串，然后转为QByteArray
        std::string docmdSeqStr = docmdSeqJson.dump();
        QByteArray docmdSeqData = QByteArray::fromStdString(docmdSeqStr);
        
        ALOG_INFO("准备执行命令: IP=%s, sktEn=%u, 命令序列=%s", "CU", "--", 
                  strIP.c_str(), sktEn, docmdSeqStr.c_str());
        
        int cmdResult = AngKMessageHandler::instance().Command_RemoteDoPTCmd(
            strIP, 0, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_DoCmdSequence, sktEn, 8,
            docmdSeqData
        );
        
        ALOG_INFO("DoJob命令执行完成，结果: %d", "CU", "--", cmdResult);
        
        // 可以选择性地返回执行结果通知
        /*nlohmann::json result;
        result["result"] = (cmdResult == 0);
        result["cmdResult"] = cmdResult;
        result["strIP"] = strIP;
        result["sktEn"] = sktEn;
        SendNotification("setDoJobResult", result);*/
        
    } catch (const std::exception& e) {
        ALOG_ERROR("HandleDoJob执行失败: %s", "CU", "--", e.what());
        
        nlohmann::json errorResult;
        errorResult["result"] = false;
        errorResult["error"] = e.what();
        SendNotification("Error", errorResult);
    }
}
void JsonRpcServer::HandleLoadProject(const nlohmann::json &params)
{
    ALOG_INFO("正在处理 LoadProject 请求", "CU", "--");
    std::string path, taskFileName;
    if (!getJsonString(params, "path", path) || !getJsonString(params, "taskFileName", taskFileName)) {
        SendErrorNotification(-32602, "无效的参数: 缺少 'path' 或 'taskFileName'");
        return;
    }
    QString filePath = QString::fromStdString(path + "/" + taskFileName);
    ALOG_INFO("项目加载路径: %s", "CU", "--", filePath.toStdString().c_str());
    // 直接通知UI加载，不再发送接受响应
    emit sgnLoadProjectFile2UI(filePath); 
}
void JsonRpcServer::HandleSiteScanAndConnect(const nlohmann::json &params)
{
    Q_UNUSED(params);
    ALOG_INFO("正在处理 SiteScanAndConnect 请求", "CU", "--");
    AngKScanManager::instance().SiteScanAndConnect();
    m_isScanSubscribed = true;
    QTimer::singleShot(3000, this, [this]() {
        emit CloseSiteScanAndConnectUI();
        });
    // 可以在这里发送一个确认通知
}
void JsonRpcServer::HandleGetProjectInfo(const nlohmann::json &params)
{
    // 待实现
}
void JsonRpcServer::HandleGetProjectInfoExt(const nlohmann::json &params)
{
    // 待实现
}
void JsonRpcServer::HandleGetSKTInfo(const nlohmann::json& params)
{
    // 待实现
}
void JsonRpcServer::HandleSendUUID(const nlohmann::json& params)
{
    ALOG_INFO("正在处理 SendUUID 请求", "CU", "--");
    std::string uuid, strIP;
    uint32_t nHopNum, sktEnable;
    if (!getJsonString(params, "uuid", uuid) ||
        !getJsonString(params, "strIP", strIP) ||
        !getJsonNumber(params, "nHopNum", nHopNum) ||
        !getJsonNumber(params, "sktEnable", sktEnable)) {
        SendErrorNotification(-32602, "无效的参数");
        return;
    }
    ALOG_INFO("发送 UUID: %s 到硬件", "CU", "--", uuid.c_str());
    AngKMessageHandler::instance().Command_RemoteDoPTCmd(strIP, nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_DoCustom,
        sktEnable, 8, QByteArray(uuid.c_str()));
    // 可以选择性地发送一个确认通知
    nlohmann::json result;
    result["message"] = "SendUUID 请求已发送至硬件";
    SendNotification("SendUUIDResult", result);
}
// 转发来自设备的原始消息
void JsonRpcServer::OnForwardDeviceMessage(const QString& strIPHop, uint16_t BPUID, const QByteArray& message)
{
}
void JsonRpcServer::TranslateSubCmd2String(ChipOperCfgSubCmdID subCmd, std::string& subCmdStr)
{
    switch (subCmd)
    {
    case UnEnable:
        break;
    case CheckID:
        subCmdStr = "CheckID";
        break;
    case PinCheck:
        subCmdStr = "PinCheck";
        break;
    case InsertionCheck:
        subCmdStr = "InsertionCheck";
        break;
    case DevicePowerOn:
        break;
    case DevicePowerOff:
        break;
    case PowerOn:
        break;
    case PowerOff:
        break;
    case SubProgram:
        subCmdStr = "Program";
        break;
    case SubErase:
        subCmdStr = "Erase";
        break;
    case SubVerify:
        subCmdStr = "Verify";
        break;
    case SubBlankCheck:
        subCmdStr = "BlankCheck";
        break;
    case SubSecure:
        subCmdStr = "Secure";
        break;
    case SubIllegalCheck:
        break;
    case SubRead:
        subCmdStr = "Read";
        break;
    case EraseIfBlankCheckFailed:
        subCmdStr = "Erase If BlankCheck Failed";
        break;
    case LowVerify:
        break;
    case HighVerify:
        break;
    case ChecksumCompare:
        subCmdStr = "Checksum Compare";
        break;
    case SubReadChipUID:
        subCmdStr = "ReadChipUID";
        break;
    case High_Low_Verify:
        break;
    default:
        break;
    }
}