#ifndef AG06_DOCUSTOM_PROTOCOL_H
#define AG06_DOCUSTOM_PROTOCOL_H

#include <QObject>
#include <QByteArray>
#include <QJsonObject>
#include <QPointer>
#include "domain/JsonRpcClient.h"
#include "xt_trim_param.h"
#include <QMap>
#include <functional>
#include <QSemaphore>
#include <QMutex>

/**
 * Ag06DoCustomProtocol
 * 封装 AG06 协议在 DoCustom 中的二进制打包与解析：
 * - 0x10 下发 UID（8 字符 ASCII）
 * - 0x11 下发 Trim 参数（二进制结构体 xt_trim_t）
 * - 0x12 日志上传（解析并发出信号）
 *
 * 通过 JSON-RPC 的 DoCustom 方法发送，二进制数据以字节数组放入 params["doCustomData"]（每个元素 0-255）。
 * 兼容：服务端也能接受字符串形式（历史实现），但推荐字节数组以确保无损。
 * params 还包含："strIP"、"sktEn"。
 */
class Ag06DoCustomProtocol : public QObject
{
    Q_OBJECT
public:
    explicit Ag06DoCustomProtocol(QObject* parent = nullptr);

    void setClient(JsonRpcClient* client);

public slots:
    // 统一对外接口
    bool executeCommand(const QJsonObject& params);

    // 原有接口保留
    bool sendUid(const QString& ip, quint32 sktEn);
    bool getUid(const QString& ip, quint32 sktEn);

    void sendTrimFromJson(const QString& trimJsonText, const QString& ip, quint32 sktEn);
    void sendTrim(const xt_trim_t& trim, const QString& ip, quint32 sktEn);
    void onNotification(const QString& method, const QJsonObject& params);

signals:
    void info(const QString& message);
    void error(const QString& message);
    void requestBuilt(quint8 cmdId, const QByteArray& payload, const QByteArray& packet);
    void responseParsed(quint8 cmdId, const QJsonObject& parsed);
    void logUploaded(const QString& uid, const QString& content);

private:

    QMap<QString, QMutex*> doCustomMutex;
    QMap<QString, QSemaphore*> m_doCustomSemaphore;

    QPointer<JsonRpcClient> m_client;
    using CommandCallback = std::function<bool(const QJsonObject&)>;
    QMap<QString, CommandCallback> m_commandCallbacks;
    
    void registerCallbacks();

    // 打包
    QByteArray buildPacket(quint8 cmdId, const QByteArray& payloadLE) const; // 长度使用小端
    QByteArray buildUidPayload(const QString& uidAscii8) const;
    QByteArray buildTrimPayload(const xt_trim_t& trim) const;

    // 发送
    void sendBinaryPacket(const QByteArray& packet, const QString& ip, quint32 sktEn);

    // 解析
    bool tryExtractBinaryFromParams(const QJsonObject& params, QByteArray& outBinary) const;
    void parseIncomingPacket(const QJsonObject& params, const QByteArray& packet);

    // JSON -> 结构体
    bool parseTrimFromJsonObject(const QJsonObject& obj, xt_trim_t& out) const;


    // 该类在仅在docustom时调用，为工作流线程中的局部变量
    QString mDevIp;
};

#endif // AG06_DOCUSTOM_PROTOCOL_H
