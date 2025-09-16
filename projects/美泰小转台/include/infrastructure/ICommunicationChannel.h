#ifndef ICOMMUNICATIONCHANNEL_H
#define ICOMMUNICATIONCHANNEL_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QString>
#include <memory>

namespace Infrastructure {

/**
 * @brief 通信通道接口
 * 定义了所有通信方式的统一接口
 */
class ICommunicationChannel : public QObject
{
    Q_OBJECT
public:
    enum class Status {
        Disconnected,
        Connected,
        Error
    };

    explicit ICommunicationChannel(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ICommunicationChannel() = default;

    // 基本操作
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual Status getStatus() const = 0;

    // 数据传输
    virtual qint64 send(const QByteArray &data) = 0;
    virtual QByteArray receive(int timeoutMs = 1000) = 0;
    virtual void clearBuffer() = 0;

    // 配置
    virtual void setParameters(const QVariantMap &params) = 0;
    virtual QVariantMap getParameters() const = 0;

signals:
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);
    void statusChanged(Status status);
    void connected();
    void disconnected();
};

} // namespace Infrastructure

#endif // ICOMMUNICATIONCHANNEL_H 