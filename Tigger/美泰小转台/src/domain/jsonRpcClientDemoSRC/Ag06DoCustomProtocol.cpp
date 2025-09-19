 #include "Ag06DoCustomProtocol.h"
 #include <QJsonDocument>
 #include <QJsonArray>
 #include <QDataStream>
 #include <QtEndian>
 #include <cstring>

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

 void Ag06DoCustomProtocol::sendUid(const QString& uidAscii8, const QString& ip, quint32 sktEn)
 {
     if (!m_client) { emit error("JsonRpcClient 未设置"); return; }
     if (uidAscii8.size() != 8) { emit error("UID 长度必须为 8 字符"); return; }

     QByteArray payload = buildUidPayload(uidAscii8);
     QByteArray packet = buildPacket(CMD_ID_UID, payload);
     emit requestBuilt(CMD_ID_UID, payload, packet);
     sendBinaryPacket(packet, ip, sktEn);
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

 QByteArray Ag06DoCustomProtocol::buildPacket(quint8 cmdId, const QByteArray& payloadLE) const
 {
     QByteArray packet;
     packet.reserve(1 + 2 + payloadLE.size());
     packet.append(char(cmdId));
     // LENGTH 小端
     quint16 len = static_cast<quint16>(payloadLE.size());
     packet.append(char(len & 0xFF));
     packet.append(char((len >> 8) & 0xFF));
     packet.append(payloadLE);
     return packet;
 }

 QByteArray Ag06DoCustomProtocol::buildUidPayload(const QString& uidAscii8) const
 {
     QByteArray payload;
     payload.reserve(8);
     QByteArray ascii = uidAscii8.toUtf8();
     payload.append(ascii.left(8));
     if (payload.size() < 8) payload.append(QByteArray(8 - payload.size(), '\0'));
     return payload;
 }

 QByteArray Ag06DoCustomProtocol::buildTrimPayload(const xt_trim_t& trim) const
 {
     QByteArray payload;
     payload.resize(sizeof(xt_trim_t));
     std::memcpy(payload.data(), &trim, sizeof(xt_trim_t));
     return payload;
 }

 void Ag06DoCustomProtocol::sendBinaryPacket(const QByteArray& packet, const QString& ip, quint32 sktEn)
 {
    QJsonObject params;
    params["strIP"] = ip;
    params["sktEn"] = static_cast<qint64>(sktEn);
    // 无损：将二进制以字节数组形式放入 JSON（每个元素 0-255）
    QJsonArray bytes;
    for (int i = 0; i < packet.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(packet.at(i));
        bytes.append(static_cast<int>(ch));
    }
    params["doCustomData"] = bytes;

     if (!m_client) return;
     m_client->doCustom(params, [this](bool success, const QJsonObject& result, const QString& err){
         if (!success) emit error(QString("DoCustom 执行失败: %1").arg(err));
         else emit info("DoCustom 执行成功");
     });
 }

bool Ag06DoCustomProtocol::tryExtractBinaryFromParams(const QJsonObject& params, QByteArray& outBinary) const
{
    // 服务器通知里，可能把二进制放在不同键或不同表示法：
    // 1) base64 字符串: doCustomData/binary/payload/data
    // 2) 字节数组: [1,2,3,...]
    // 优先使用显式base64，其次数组/十六进制，最后再尝试原始字符串
    const QStringList keys = {QStringLiteral("doCustomData"), QStringLiteral("dataB64"), QStringLiteral("binary"), QStringLiteral("payload"), QStringLiteral("dataHex"), QStringLiteral("data")};
    for (const auto& k : keys) {
        if (!params.contains(k)) continue;
        const QJsonValue v = params.value(k);
    if (v.isString()) {
            const QString s = v.toString();
            if (!s.isEmpty()) {
                // 1) 尝试按 base64 解码
                QByteArray decoded = QByteArray::fromBase64(s.toLatin1());
        if (!decoded.isEmpty()) { if (decoded.size() >= 3) { outBinary = decoded; return true; } }
                // 2) 若不是 base64，可能是直接把二进制放进了 JSON 字符串（包含控制字符）
                //    则直接按 UTF-8/原样取字节
        QByteArray raw = s.toUtf8();
        if (raw.size() >= 3) { outBinary = raw; return true; }
                // 3) 若是十六进制字符串，尝试解析（允许包含空格、逗号、0x 前缀）
                QString hex = s;
                hex.replace(" ", ""); hex.replace(",", ""); hex.replace("0x", "", Qt::CaseInsensitive);
                bool isHex = true; for (QChar c : hex) { if (!c.isDigit() && (c.toLower()<'a' || c.toLower()>'f')) { isHex=false; break; } }
                if (isHex && (hex.size()%2==0) && !hex.isEmpty()) {
                    QByteArray buf; buf.reserve(hex.size()/2);
                    for (int i=0;i<hex.size();i+=2) { bool ok=false; uchar b=uchar(hex.mid(i,2).toUShort(&ok,16)); if (!ok) { buf.clear(); break; } buf.append(char(b)); }
            if (buf.size() >= 3) { outBinary = buf; return true; }
                }
            }
        } else if (v.isArray()) {
            QJsonArray arr = v.toArray();
            QByteArray buf; buf.reserve(arr.size());
            for (const auto& it : arr) buf.append(char(it.toInt() & 0xFF));
        if (buf.size() >= 3) { outBinary = buf; return true; }
        }
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
         if (payload.size() >= 10) {
             QByteArray uidBytes = payload.left(8);
             QString uid = QString::fromLatin1(uidBytes);
             int colonIdx = payload.indexOf(':', 8);
             if (colonIdx >= 0) {
                 QByteArray rest = payload.mid(colonIdx + 1);
                 int zeroIdx = rest.indexOf('\0');
                 QByteArray log = zeroIdx >= 0 ? rest.left(zeroIdx) : rest;
                 emit logUploaded(uid.trimmed(), QString::fromUtf8(log));
                 QJsonObject obj{{"uid", uid.trimmed()}, {"log", QString::fromUtf8(log)}};
                 emit responseParsed(cmd, obj);
                 return;
             }
         }
     }

     // 其他命令默认抛给上层
     QJsonObject obj{{"cmdId", int(cmd)}, {"payloadSize", int(payload.size())}};
     emit responseParsed(cmd, obj);
 }

void Ag06DoCustomProtocol::onNotification(const QString& method, const QJsonObject& params)
{
    Q_UNUSED(method)
    // 优先解析二进制封包（DoCustom 回包）
    QByteArray bin;
    if (tryExtractBinaryFromParams(params, bin)) {
        parseIncomingPacket(bin);
        return;
    }

    // 兜底：如果直接就是日志 JSON（{uid, log}），直接触发信号
    const QString uid = params.value("uid").toString();
    const QString log = params.value("log").toString();
    if (!uid.isEmpty() && !log.isEmpty()) {
        emit logUploaded(uid.trimmed(), log);
        QJsonObject obj{{"uid", uid.trimmed()}, {"log", log}};
        emit responseParsed(CMD_ID_LOG_UPLOAD, obj);
    }
}

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

     return true;
 }

