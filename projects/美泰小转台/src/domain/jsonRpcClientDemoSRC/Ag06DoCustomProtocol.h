 #ifndef AG06_DOCUSTOM_PROTOCOL_H
 #define AG06_DOCUSTOM_PROTOCOL_H

 #include <QObject>
 #include <QByteArray>
 #include <QJsonObject>
 #include <QPointer>
 #include "JsonRpcClient.h"
 #include "xt_trim_param.h"

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
     // 发送 0x10 UID
     void sendUid(const QString& uidAscii8, const QString& ip, quint32 sktEn);

     // 发送 0x11 Trim 参数（从 JSON 文本解析为 xt_trim_t 再下发）
     void sendTrimFromJson(const QString& trimJsonText, const QString& ip, quint32 sktEn);

     // 直接发送 0x11，传入结构体
     void sendTrim(const xt_trim_t& trim, const QString& ip, quint32 sktEn);

     // 连接到 JsonRpcClient 通知，尝试解析 DoCustom 二进制数据
     void onNotification(const QString& method, const QJsonObject& params);

 signals:
     void info(const QString& message);
     void error(const QString& message);
     void requestBuilt(quint8 cmdId, const QByteArray& payload, const QByteArray& packet);
     void responseParsed(quint8 cmdId, const QJsonObject& parsed);
     void logUploaded(const QString& uid, const QString& content);

 private:
     QPointer<JsonRpcClient> m_client;

     // 打包
     QByteArray buildPacket(quint8 cmdId, const QByteArray& payloadLE) const; // 长度使用小端
     QByteArray buildUidPayload(const QString& uidAscii8) const;
     QByteArray buildTrimPayload(const xt_trim_t& trim) const;

     // 发送
     void sendBinaryPacket(const QByteArray& packet, const QString& ip, quint32 sktEn);

     // 解析
     bool tryExtractBinaryFromParams(const QJsonObject& params, QByteArray& outBinary) const;
     void parseIncomingPacket(const QByteArray& packet);

     // JSON -> 结构体
     bool parseTrimFromJsonObject(const QJsonObject& obj, xt_trim_t& out) const;
 };

 #endif // AG06_DOCUSTOM_PROTOCOL_H

