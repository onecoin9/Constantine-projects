#ifndef UDPCOMMUNICATOR_H
#define UDPCOMMUNICATOR_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include "ProtocolStructures.h" 
#include "CommandBuilder.h"     

class UdpCommunicator : public QObject
{
    Q_OBJECT
public:
    explicit UdpCommunicator(QObject *parent = nullptr);
    ~UdpCommunicator();

    bool bind(quint16 localPort = 0, QAbstractSocket::BindMode mode = QAbstractSocket::ShareAddress);

    void setTargetServer(const QHostAddress& address, quint16 port);

    quint16 getLocalPort() const { return m_udpSocket ? m_udpSocket->localPort() : 0; }

    const QHostAddress& getTargetServerAddress() const { return m_targetServerAddress; }
    quint16 getTargetServerPort() const { return m_targetServerPort; }

    void sendDeviceControl(const QString& thermalMode, double targetTemp, int stableTime, double rampRate = 0.0);
    void sendFunctionTestConfig(
        bool enabledTest, bool powerOn, double targetVoltage, int voltageStableTime,
        const QVariant& targetCurrent, const QVariant& targetDriveFrequency,
        const QVariant& targetDriveForce, const QVariant& targetQuadrature,
        const QVariant& targetAmplitude, const QVariant& targetClock
    );
    void sendDataAcquisitionConfig(bool enable, int duration, int interval);

    qint64 sendPacket(const QByteArray& packet);


signals:
    void deviceControlResponseReceived(int code, int timestamp, const QHostAddress& senderAddress, quint16 senderPort);
    void functionTestResponseReceived(int code, int timestamp, const QHostAddress& senderAddress, quint16 senderPort);
    void sensorDataReceived(const CommandBuilder::SensorData& sensorData, const QHostAddress& senderAddress, quint16 senderPort);
    void unknownPacketReceived(quint16 cmdID, const QJsonObject& jsonData, const QHostAddress& senderAddress, quint16 senderPort);
    void bindError(const QString& errorString);
    void sendError(const QString& errorString);

private slots:
    void onReadyRead(); 

private:
    QUdpSocket *m_udpSocket;
    QHostAddress m_targetServerAddress;
    quint16 m_targetServerPort;
    int m_sequenceCounter; 

    void processDatagram(const QByteArray& datagram, const QHostAddress& senderAddress, quint16 senderPort);
};

#endif // UDPCOMMUNICATOR_H