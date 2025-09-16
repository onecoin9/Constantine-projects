#include "Ag06DoCustomProtocol.h"
#include "DutManager.h"
#include "mtuidgenerator.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDataStream>
#include <QtEndian>
#include <cstring>
#include <QMap>
#include <functional>

static constexpr quint8 CMD_ID_UID = 0x10;
static constexpr quint8 CMD_ID_TRIM = 0x11;
static constexpr quint8 CMD_ID_LOG_UPLOAD = 0x12;

Ag06DoCustomProtocol::Ag06DoCustomProtocol(QObject* parent)
    : QObject(parent)
{
}

void Ag06DoCustomProtocol::setClient(JsonRpcClient* client)
{
    if (m_client == client) return;
    if (m_client) {
        disconnect(m_client, &JsonRpcClient::notificationReceived,
                   this, &Ag06DoCustomProtocol::onNotification);
    }
    m_client = client;
    if (m_client) {
        connect(m_client, &JsonRpcClient::notificationReceived,
                this, &Ag06DoCustomProtocol::onNotification);
    }
}

void Ag06DoCustomProtocol::sendUid(const QString& ip, quint32 sktEn)
{
    // 遍历sktEn比特位，如果为1，则发送uid
    for (int i = 0; i < sizeof(sktEn) * 8; i++) {
        if (sktEn & (1 << i)) {
            QString uidAscii8 = MTUIDGenerator::getInstance().getUID().c_str();
        
            QByteArray payload = buildUidPayload(uidAscii8);
            QByteArray packet = buildPacket(CMD_ID_UID, payload);
            emit requestBuilt(CMD_ID_UID, payload, packet);
            sendBinaryPacket(packet, ip, 1 << i);
        }
    }
}

void Ag06DoCustomProtocol::sendTrimFromJson(const QString& trimJsonText, const QString& ip, quint32 sktEn)
{
    QJsonParseError perr{};
    QJsonDocument doc = QJsonDocument::fromJson(trimJsonText.toUtf8(), &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
        emit error(QString("Trim JSON 解析失败: %1").arg(perr.errorString()));
        return;
    }
    xt_trim_t trim{};
    if (!parseTrimFromJsonObject(doc.object(), trim)) {
        emit error("Trim JSON 转结构体失败");
        return;
    }
    sendTrim(trim, ip, sktEn);
}

void Ag06DoCustomProtocol::sendTrim(const xt_trim_t& trim, const QString& ip, quint32 sktEn)
{
    if (!m_client) { emit error("JsonRpcClient 未设置"); return; }

    QByteArray payload = buildTrimPayload(trim);
    QByteArray packet = buildPacket(CMD_ID_TRIM, payload);
    emit requestBuilt(CMD_ID_TRIM, payload, packet);
    sendBinaryPacket(packet, ip, sktEn);
}

void Ag06DoCustomProtocol::onNotification(const QString& method, const QJsonObject& params)
{
    // 只关心服务端主动推送的 setDoCustomRet
    if (method != "setDoCustomRet") return;

    emit info(QString("收到 setDoCustomRet 通知"));

    QByteArray binaryData;
    if (!tryExtractBinaryFromParams(params, binaryData)) return;

    emit info(QString("二进制数据长度: %1").arg(binaryData.size()));

    parseIncomingPacket(binaryData);
}

QByteArray Ag06DoCustomProtocol::buildPacket(quint8 cmdId, const QByteArray& payloadLE) const
{
    if (payloadLE.size() > 65535) {
        emit const_cast<Ag06DoCustomProtocol*>(this)->error("PAYLOAD 过大");
        return QByteArray();
    }

    QByteArray packet;
    packet.append(char(cmdId));

    // 长度（小端）
    quint16 lenLE = static_cast<quint16>(payloadLE.size());
    packet.append(char(lenLE & 0xFF));
    packet.append(char((lenLE >> 8) & 0xFF));

    packet.append(payloadLE);
    return packet;
}

QByteArray Ag06DoCustomProtocol::buildUidPayload(const QString& uidAscii8) const
{
    return uidAscii8.left(8).toLatin1().leftJustified(8, '\0');
}

QByteArray Ag06DoCustomProtocol::buildTrimPayload(const xt_trim_t& trim) const
{
    QByteArray arr(sizeof(xt_trim_t), '\0');
    std::memcpy(arr.data(), &trim, sizeof(xt_trim_t));
    return arr;
}

void Ag06DoCustomProtocol::sendBinaryPacket(const QByteArray& packet, const QString& ip, quint32 sktEn)
{
    if (!m_client) return;

    QJsonObject params;
    params["strIP"] = ip;
    params["sktEn"] = static_cast<qint64>(sktEn);

    // 将二进制数据转换为字节数组
    QJsonArray dataArray;
    for (int i = 0; i < packet.size(); ++i) {
        dataArray.append(static_cast<quint8>(packet.at(i)));
    }
    params["doCustomData"] = dataArray;

    emit info(QString("发送 DoCustom: IP=%1, sktEn=%2, 数据长度=%3")
              .arg(ip).arg(sktEn).arg(packet.size()));

    m_client->doCustom(params);
}

bool Ag06DoCustomProtocol::tryExtractBinaryFromParams(const QJsonObject& params, QByteArray& outBinary) const
{
    // setDoCustomRet 通知的格式：
    // {
    //   "result": {
    //     "data": {
    //       "results": [ ... 字节数组或字符串 ... ]
    //     }
    //   }
    // }

    auto result = params.value("result").toObject();
    auto data = result.value("data").toObject();
    auto results = data.value("results");

    if (results.isArray()) {
        QJsonArray arr = results.toArray();
        outBinary.clear();
        for (const auto& val : arr) {
            if (val.isDouble()) {
                int byte = val.toInt();
                if (byte >= 0 && byte <= 255) {
                    outBinary.append(char(byte));
                }
            }
        }
        return !outBinary.isEmpty();
    }
    else if (results.isString()) {
        QString str = results.toString();
        outBinary = str.toUtf8();
        return !outBinary.isEmpty();
    }
    return false;
}

void Ag06DoCustomProtocol::parseIncomingPacket(const QByteArray& packet)
{
    if (packet.size() < 3) {
        // 兜底：尝试裸 ASCII 日志格式：UID(8) ':' log '\0'
        if (packet.size() >= 10 && packet.size() < 1024) {
            if (packet.size() >= 9 && packet.at(8) == ':') {
                QByteArray uidBytes = packet.left(8);
                QString uid = QString::fromLatin1(uidBytes).trimmed();
                QByteArray rest = packet.mid(9);
                int zeroIdx = rest.indexOf('\0');
                QByteArray log = zeroIdx >= 0 ? rest.left(zeroIdx) : rest;
                if (!uid.isEmpty() && !log.isEmpty()) {
                    emit logUploaded(uid, QString::fromUtf8(log));
                    QJsonObject obj{{"uid", uid}, {"log", QString::fromUtf8(log)}};
                    emit responseParsed(CMD_ID_LOG_UPLOAD, obj);
                }
            }
        }
        return;
    }
    const uchar* d = reinterpret_cast<const uchar*>(packet.constData());
    quint8 cmd = d[0];
    quint16 len = d[1] | (quint16(d[2]) << 8);
    if (packet.size() < 3 + int(len)) {
        // 长度不匹配，尝试裸 ASCII 日志兜底
        if (packet.size() >= 9 && packet.at(8) == ':') {
            QByteArray uidBytes = packet.left(8);
            QString uid = QString::fromLatin1(uidBytes).trimmed();
            QByteArray rest = packet.mid(9);
            int zeroIdx = rest.indexOf('\0');
            QByteArray log = zeroIdx >= 0 ? rest.left(zeroIdx) : rest;
            if (!uid.isEmpty() && !log.isEmpty()) {
                emit logUploaded(uid, QString::fromUtf8(log));
                QJsonObject obj{{"uid", uid}, {"log", QString::fromUtf8(log)}};
                emit responseParsed(CMD_ID_LOG_UPLOAD, obj);
            }
        }
        return;
    }
    QByteArray payload = packet.mid(3, len);

    if (cmd == CMD_ID_LOG_UPLOAD) {
        // 解析: uid(8 ASCII) ':' 分隔符 后跟以 \0 结尾字符串
        if (payload.size() >= 10 && payload.at(8) == ':') {
            QByteArray uidBytes = payload.left(8);
            QString uid = QString::fromLatin1(uidBytes).trimmed();
            QByteArray logData = payload.mid(9);
            int zeroIdx = logData.indexOf('\0');
            QByteArray log = zeroIdx >= 0 ? logData.left(zeroIdx) : logData;
            QString logContent = QString::fromUtf8(log);

            emit info(QString("日志上传解析: UID=%1").arg(uid));
            emit logUploaded(uid, logContent);

            QJsonObject obj;
            obj["uid"] = uid;
            obj["log"] = logContent;
            emit responseParsed(cmd, obj);
        } else {
            emit error(QString("0x12 日志数据格式错误，长度=%1").arg(payload.size()));
        }
    } else if (cmd == CMD_ID_UID) {
        emit info(QString("收到 0x10 响应（UID），长度=%1").arg(payload.size()));
        emit responseParsed(cmd, QJsonObject{{"payload_hex", QString::fromLatin1(payload.toHex())}});
    } else if (cmd == CMD_ID_TRIM) {
        emit info(QString("收到 0x11 响应（Trim），长度=%1").arg(payload.size()));
        emit responseParsed(cmd, QJsonObject{{"payload_hex", QString::fromLatin1(payload.toHex())}});
    } else {
        emit info(QString("未知命令 ID: 0x%1").arg(cmd, 2, 16, QChar('0')));
    }
}

// 辅助函数
static uint32_t jsonToUint32(const QJsonObject& o, const char* key, uint32_t def = 0) {
    return static_cast<uint32_t>(o.value(key).toVariant().toULongLong());
}
static uint16_t jsonToUint16(const QJsonObject& o, const char* key, uint16_t def = 0) {
    return static_cast<uint16_t>(o.value(key).toVariant().toUInt());
}
static uint8_t jsonToUint8(const QJsonObject& o, const char* key, uint8_t def = 0) {
    return static_cast<uint8_t>(o.value(key).toVariant().toUInt());
}

bool Ag06DoCustomProtocol::parseTrimFromJsonObject(const QJsonObject& obj, xt_trim_t& out) const
{
    // 预期 JSON 结构示例：
    // {
    //   "t1_trim_en": {"value": 0x...},
    //   "t1_trim_regs": { ... 五个寄存器字段 ... },
    //   "t1_output_ctrl_value": {"dc6":..,"dc5":..,"ac27":..,"ac4":..},
    //   "t1_trim_params": {icc_min:.., ...},
    //   "delay_set": {power_on_delay_ms:.., ...}
    // }
    if (!obj.contains("t1_trim_en") || !obj.contains("t1_trim_regs") ||
        !obj.contains("t1_output_ctrl_value") || !obj.contains("t1_trim_params") ||
        !obj.contains("delay_set")) return false;

    // t1_trim_en
    {
        QJsonObject e = obj.value("t1_trim_en").toObject();
        out.t1_trim_en.value = jsonToUint32(e, "value", 0);
    }

    // helper: reg_bit
    auto parseRegBit = [](const QJsonObject& r)->xt_reg_bit_t{
        xt_reg_bit_t a{};
        a.addr = jsonToUint8(r, "addr");
        a.start_bit = jsonToUint8(r, "start_bit");
        a.width_bit = jsonToUint8(r, "width_bit");
        a.write_back = jsonToUint8(r, "write_back");
        return a;
    };

    // t1_trim_regs
    {
        QJsonObject r = obj.value("t1_trim_regs").toObject();
        out.t1_trim_regs.output_ctrl = parseRegBit(r.value("output_ctrl").toObject());
        out.t1_trim_regs.dc_trim = parseRegBit(r.value("dc_trim").toObject());
        out.t1_trim_regs.ac_en = parseRegBit(r.value("ac_en").toObject());
        out.t1_trim_regs.ac_trim = parseRegBit(r.value("ac_trim").toObject());
        out.t1_trim_regs.eoc = parseRegBit(r.value("eoc").toObject());
    }

    // t1_output_ctrl_value
    {
        QJsonObject o = obj.value("t1_output_ctrl_value").toObject();
        out.t1_output_ctrl_value.dc6 = jsonToUint8(o, "dc6");
        out.t1_output_ctrl_value.dc5 = jsonToUint8(o, "dc5");
        out.t1_output_ctrl_value.ac27 = jsonToUint8(o, "ac27");
        out.t1_output_ctrl_value.ac4 = jsonToUint8(o, "ac4");
    }

    // t1_trim_params
    {
        QJsonObject p = obj.value("t1_trim_params").toObject();
        out.t1_trim_params.icc_min = jsonToUint32(p, "icc_min");
        out.t1_trim_params.icc_max = jsonToUint32(p, "icc_max");
        out.t1_trim_params.dc_basic_min = jsonToUint16(p, "dc_basic_min");
        out.t1_trim_params.dc_basic_max = jsonToUint16(p, "dc_basic_max");
        out.t1_trim_params.dc_p2p_max = jsonToUint16(p, "dc_p2p_max");
        out.t1_trim_params.dc_trim_min = jsonToUint16(p, "dc_trim_min");
        out.t1_trim_params.dc_trim_max = jsonToUint16(p, "dc_trim_max");
        out.t1_trim_params.dc_trim_best = jsonToUint16(p, "dc_trim_best");
        out.t1_trim_params.ac_trim_min = jsonToUint32(p, "ac_trim_min");
        out.t1_trim_params.ac_trim_max = jsonToUint32(p, "ac_trim_max");
        out.t1_trim_params.ac_trim_best = jsonToUint32(p, "ac_trim_best");
        out.t1_trim_params.ac_avg_min = jsonToUint16(p, "ac_avg_min");
        out.t1_trim_params.ac_avg_max = jsonToUint16(p, "ac_avg_max");
        out.t1_trim_params.ac_p2p_min = jsonToUint16(p, "ac_p2p_min");
        out.t1_trim_params.ac_p2p_max = jsonToUint16(p, "ac_p2p_max");
        out.t1_trim_params.ac_freq_min = jsonToUint32(p, "ac_freq_min");
        out.t1_trim_params.ac_freq_max = jsonToUint32(p, "ac_freq_max");
    }

    // delay_set
    {
        QJsonObject d = obj.value("delay_set").toObject();
        out.delay_set.power_on_delay_ms = jsonToUint32(d, "power_on_delay_ms");
        out.delay_set.t1_dc_stable_ms = jsonToUint32(d, "t1_dc_stable_ms");
        out.delay_set.t1_ac_stable_ms = jsonToUint32(d, "t1_ac_stable_ms");
        out.delay_set.delay_after_program_ms = jsonToUint32(d, "delay_after_program_ms");
        out.delay_set.reg_operation_delay_us = jsonToUint32(d, "reg_operation_delay_us");
    }

    emit const_cast<Ag06DoCustomProtocol*>(this)->info("Trim JSON 成功转换为结构体");
    return true;
}


void Ag06DoCustomProtocol::registerCallbacks()
{
    // 注册所有支持的命令回调
    m_commandCallbacks["sendUid"] = [this](const QJsonObject& params) {
        sendUid(params["strIp"].toString(), params["sktEn"].toInt());
    };

}

// 统一对外接口
void Ag06DoCustomProtocol::executeCommand(const QJsonObject& params)
{
    if (m_commandCallbacks.isEmpty()) {
        registerCallbacks();
    }
    
    QString command = params.value("command").toString();
    if (!m_commandCallbacks.contains(command)) {
        emit error(QString("未知命令: %1").arg(command));
        return;
    }
    
    // 调用对应命令的回调函数
    m_commandCallbacks[command](params);
}
