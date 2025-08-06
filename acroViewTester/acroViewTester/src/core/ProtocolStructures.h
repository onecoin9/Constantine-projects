// ProtocolStructures.h
#ifndef PROTOCOLSTRUCTURES_H
#define PROTOCOLSTRUCTURES_H

#include <QtGlobal>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QDebug>
#include <QDataStream>

// 协议固定头部
struct ProtocolHeader {
    quint32 cmdFlag;      
    quint16 cmdID;        
    quint16 cmdDataSize;  

    static const quint32 SWXY_FLAG = 0x53575859;

    ProtocolHeader() : cmdFlag(SWXY_FLAG), cmdID(0), cmdDataSize(0) {}
};


namespace ProtocolUtils {

    // 构建完整的数据包
    inline QByteArray buildPacket(quint16 cmdID, const QJsonObject& jsonData) {
        QJsonDocument doc(jsonData);
        QByteArray jsonBytes = doc.toJson(QJsonDocument::Compact); 

        ProtocolHeader header;
        header.cmdID = cmdID;
        header.cmdDataSize = static_cast<quint16>(jsonBytes.size());

        QByteArray packet;
        QDataStream stream(&packet, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::LittleEndian); 

        stream << header.cmdFlag;
        stream << header.cmdID;
        stream << header.cmdDataSize;
        packet.append(jsonBytes);

        return packet;
    }

    inline bool parseHeader(const QByteArray& packetData, ProtocolHeader& header, QByteArray& jsonDataPayload) {
        if (packetData.size() < 8) { 
            qWarning() << "Packet too short for header";
            return false;
        }

        QDataStream stream(packetData);
        stream.setByteOrder(QDataStream::LittleEndian);

        stream >> header.cmdFlag;
        stream >> header.cmdID;
        stream >> header.cmdDataSize;

        if (header.cmdFlag != ProtocolHeader::SWXY_FLAG) {
            qWarning() << "Invalid CmdFlag:" << Qt::hex << header.cmdFlag;
            return false;
        }

        
        int expectedMinSize = 8 + header.cmdDataSize;
        // if (packetData.size() < expectedMinSize + 1) { // 如果有额外的一个字节
        if (packetData.size() < expectedMinSize) { 
             qWarning() << "Packet too short for CmdData. Expected at least" << expectedMinSize << "got" << packetData.size();
             return false;
        }

        jsonDataPayload = packetData.mid(8, header.cmdDataSize);
        return true;
    }

    // 解析JSON数据部分
    inline QJsonObject parseJsonData(const QByteArray& jsonDataPayload) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(jsonDataPayload, &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parse error:" << error.errorString();
            return QJsonObject(); // 返回空对象表示错误
        }
        if (!doc.isObject()) {
            qWarning() << "JSON data is not an object";
            return QJsonObject();
        }
        return doc.object();
    }
}

#endif // PROTOCOLSTRUCTURES_H