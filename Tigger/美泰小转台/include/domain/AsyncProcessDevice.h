#pragma once

#include "domain/IDevice.h"
#include <QProcess>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace Domain {

class AsyncProcessDevice : public IDevice {
    Q_OBJECT

public:
    enum ProcessType {
        Normal, // 确保有Normal
        Activation,    // aprog.exe with JSON-RPC
        Calibration    // MTCaliTest.exe with file I/O
    };

    explicit AsyncProcessDevice(ProcessType type, QObject *parent = nullptr);
    ~AsyncProcessDevice() override;

    // IDevice interface
    bool initialize() override;
    bool release() override;
    QJsonObject executeCommand(const QString &command, const QJsonObject &params) override;
    
    DeviceType getType() const override;
    DeviceStatus getStatus() const override;
    QString getName() const override;
    QString getDescription() const override;
    
    void setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel>) override;
    std::shared_ptr<Infrastructure::ICommunicationChannel> getCommunicationChannel() const override;
    void setConfiguration(const QJsonObject &config) override;
    QJsonObject getConfiguration() const override;
    bool selfTest() override;

signals:
    void activationCompleted(int siteIndex, const QJsonObject &result);
    void calibrationCompleted(const QString &dutId, const QJsonObject &result);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onJsonRpcResponse();
    void onCalibrationCheckTimer();

private:
    // 激活相关方法
    bool startActivationProcess();
    bool sendJsonRpcCommand(const QString &method, const QJsonObject &params);
    bool waitForJsonRpcServer();
    
    // 标定相关方法
    bool startCalibrationProcess(const QJsonObject &params);
    bool checkCalibrationResult();
    
    ProcessType m_processType;
    QProcess *m_process;
    QTimer *m_responseTimer;
    QNetworkAccessManager *m_networkManager;
    
    // 配置参数
    QString m_executable;
    QStringList m_arguments;
    QString m_jsonRpcUrl;
    int m_jsonRpcPort;
    QString m_sharedDataPath;
    
    // 状态管理
    DeviceStatus m_status;
    QJsonObject m_config;
    QJsonObject m_pendingResult;
    int m_requestId;
};

} // namespace Domain 