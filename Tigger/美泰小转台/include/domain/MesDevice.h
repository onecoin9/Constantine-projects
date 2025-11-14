#ifndef  MESDEVICE_H
#define MESDEVICE_H

#include "IDevice.h"
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QDateTime>

namespace Domain {

/**
 * @brief MesDevice - Mes???
 */
class MesDevice : public IDevice
{
    Q_OBJECT
public:
    explicit MesDevice(QObject* parent = nullptr);
    ~MesDevice() override;

    bool initialize() override;
    bool release() override;
    DeviceStatus getStatus() const override;
    QString getName() const override;
    DeviceType getType() const override;
    QString getDescription() const override;
    QJsonObject executeCommand(const QString &command, const QJsonObject &params) override;
    void setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel> channel) override;
    std::shared_ptr<Infrastructure::ICommunicationChannel> getCommunicationChannel() const override;
    void setConfiguration(const QJsonObject &config) override;
    QJsonObject getConfiguration() const override;
    bool selfTest() override;

private:

    enum SortingType {
        SortingByUid = 0,
        SortingByScan,
    };
    bool testConnection();
    QJsonObject handleQueryChipInfoByUid(const QJsonObject &params);
    QJsonObject handleQueryChipInfoByScanCode(const QJsonObject& params);

private:
    QNetworkAccessManager* m_networkManager;
    QJsonObject m_config;
    QString m_name;
    DeviceType m_type;
};

} // namespace Domain

#endif // ! MESDEVICE_H
