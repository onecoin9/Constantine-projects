#include "domain/AsyncProcessDevice.h"
#include "core/Logger.h"
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileSystemWatcher>
#include <QDir>
#include <QFileInfo>
#include <QEventLoop>
#include <QThread>
#include <QUrl>
#include <QFile>
#include <QBuffer>

namespace Domain {

AsyncProcessDevice::AsyncProcessDevice(ProcessType type, QObject *parent)
    : IDevice(parent)
    , m_processType(type)
    , m_process(nullptr)
    , m_responseTimer(new QTimer(this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_status(DeviceStatus::Disconnected)
    , m_requestId(1)
{
    m_responseTimer->setSingleShot(false);
    m_responseTimer->setInterval(1000); // 每秒检查一次
    connect(m_responseTimer, &QTimer::timeout, this, &AsyncProcessDevice::onCalibrationCheckTimer);
}

AsyncProcessDevice::~AsyncProcessDevice()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
}

bool AsyncProcessDevice::initialize()
{
    m_status = DeviceStatus::Initializing;
    emit statusChanged(m_status);
    
    if (m_processType == Activation) {
        if (startActivationProcess()) {
            m_status = DeviceStatus::Ready;
            emit statusChanged(m_status);
            return true;
        }
    } else {
        // 标定设备不需要预启动进程
        m_status = DeviceStatus::Ready;
        emit statusChanged(m_status);
        return true;
    }
    
    m_status = DeviceStatus::Error;
    emit statusChanged(m_status);
    return false;
}

bool AsyncProcessDevice::release()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        if (!m_process->waitForFinished(3000)) {
            m_process->kill();
        }
    }
    m_responseTimer->stop();
    m_status = DeviceStatus::Disconnected;
    emit statusChanged(m_status);
    return true;
}

QJsonObject AsyncProcessDevice::executeCommand(const QString &command, const QJsonObject &params)
{
    QJsonObject result;
    result["success"] = false;
    
    if (m_status != DeviceStatus::Ready) {
        result["error"] = "Device not ready";
        return result;
    }
    
    if (m_processType == Activation) {
        if (command == "activateSite") {
            if (sendJsonRpcCommand("activateSite", params)) {
                result["success"] = true;
                result["message"] = "Activation command sent, waiting for async result";
            } else {
                result["error"] = "Failed to send JSON-RPC command";
            }
        } else if (command == "querySiteInfo") {
            if (sendJsonRpcCommand("querySiteInfo", params)) {
                result["success"] = true;
                result["message"] = "Query command sent, waiting for async result";
            } else {
                result["error"] = "Failed to send JSON-RPC command";
            }
        }
    } else if (m_processType == Calibration) {
        if (command == "startCalibration") {
            if (startCalibrationProcess(params)) {
                result["success"] = true;
                result["message"] = "Calibration started, monitoring result file";
            } else {
                result["error"] = "Failed to start calibration process";
            }
        }
    }
    
    return result;
}

bool AsyncProcessDevice::startActivationProcess()
{
    if (m_process) {
        return true; // 已经启动
    }
    
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AsyncProcessDevice::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &AsyncProcessDevice::onProcessError);
    
    QString program = m_config.value("executable").toString("aprog.exe");
    QStringList arguments;
    arguments << "-r"; // 启动 JSON-RPC 服务器模式
    
    m_process->start(program, arguments);
    
    if (!m_process->waitForStarted(5000)) {
        LOG_MODULE_ERROR("AsyncProcessDevice", "Failed to start activation process");
        return false;
    }
    
    // 等待 JSON-RPC 服务器就绪
    return waitForJsonRpcServer();
}

bool AsyncProcessDevice::waitForJsonRpcServer()
{
    m_jsonRpcUrl = m_config.value("jsonRpcUrl").toString("http://localhost:8080/jsonrpc");
    
    // 简单的重试机制检查服务器是否就绪
    for (int i = 0; i < 10; ++i) {
        QUrl url(m_jsonRpcUrl);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        QJsonObject testRequest;
        testRequest["jsonrpc"] = "2.0";
        testRequest["method"] = "ping";
        testRequest["id"] = m_requestId++;
        
        QByteArray jsonData = QJsonDocument(testRequest).toJson();
        QBuffer *buffer = new QBuffer(&jsonData);
        buffer->open(QIODevice::ReadOnly);
        QNetworkReply *reply = m_networkManager->post(request, buffer);
        buffer->setParent(reply); // 确保 buffer 随 reply 一起被删除
        
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(1000, &loop, &QEventLoop::quit); // 1秒超时
        loop.exec();
        
        if (reply->error() == QNetworkReply::NoError) {
            reply->deleteLater();
            LOG_MODULE_INFO("AsyncProcessDevice", "JSON-RPC server is ready");
            return true;
        }
        reply->deleteLater();
        QThread::msleep(500); // 等待500ms再重试
    }
    
    LOG_MODULE_ERROR("AsyncProcessDevice", "JSON-RPC server failed to become ready");
    return false;
}

bool AsyncProcessDevice::sendJsonRpcCommand(const QString &method, const QJsonObject &params)
{
    QUrl url(m_jsonRpcUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject jsonRpcRequest;
    jsonRpcRequest["jsonrpc"] = "2.0";
    jsonRpcRequest["method"] = method;
    jsonRpcRequest["params"] = params;
    jsonRpcRequest["id"] = m_requestId++;
    
    QByteArray jsonData = QJsonDocument(jsonRpcRequest).toJson();
    QBuffer *buffer = new QBuffer(&jsonData);
    buffer->open(QIODevice::ReadOnly);
    QNetworkReply *reply = m_networkManager->post(request, buffer);
    buffer->setParent(reply); // 确保 buffer 随 reply 一起被删除
    connect(reply, &QNetworkReply::finished, this, &AsyncProcessDevice::onJsonRpcResponse);
    
    LOG_MODULE_INFO("AsyncProcessDevice", QString("Sent JSON-RPC command: %1").arg(method).toStdString());
    return true;
}

bool AsyncProcessDevice::startCalibrationProcess(const QJsonObject &params)
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
    
    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &AsyncProcessDevice::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred, this, &AsyncProcessDevice::onProcessError);
    }
    
    QString program = m_config.value("executable").toString("MTCaliTest.exe");
    QStringList arguments;
    
    // 从参数中构建命令行参数
    QString algorithm = params.value("algorithm").toString("ALG01");
    QString inputFile = params.value("inputFile").toString();
    QString outputFile = params.value("outputFile").toString();
    
    arguments << "-alg" << algorithm;
    if (!inputFile.isEmpty()) {
        arguments << "-input" << inputFile;
    }
    if (!outputFile.isEmpty()) {
        arguments << "-output" << outputFile;
    }
    
    m_sharedDataPath = outputFile; // 监控输出文件
    
    m_process->start(program, arguments);
    
    if (!m_process->waitForStarted(3000)) {
        LOG_MODULE_ERROR("AsyncProcessDevice", "Failed to start calibration process");
        return false;
    }
    
    // 开始监控结果文件
    m_responseTimer->start();
    
    LOG_MODULE_INFO("AsyncProcessDevice", QString("Started calibration process with algorithm: %1").arg(algorithm).toStdString());
    return true;
}

void AsyncProcessDevice::onJsonRpcResponse()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        LOG_MODULE_ERROR("AsyncProcessDevice", QString("JSON-RPC request failed: %1").arg(reply->errorString()).toStdString());
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject response = doc.object();
    
    if (response.contains("result")) {
        QJsonObject result = response["result"].toObject();
        QString method = result.value("method").toString();
        
        if (method == "activateSite") {
            int siteIndex = result.value("siteIndex").toInt();
            emit activationCompleted(siteIndex, result);
            LOG_MODULE_INFO("AsyncProcessDevice", QString("Activation completed for site %1").arg(siteIndex).toStdString());
        }
    } else if (response.contains("error")) {
        LOG_MODULE_ERROR("AsyncProcessDevice", QString("JSON-RPC error: %1").arg(response["error"].toObject()["message"].toString()).toStdString());
    }
}

void AsyncProcessDevice::onCalibrationCheckTimer()
{
    if (checkCalibrationResult()) {
        m_responseTimer->stop();
    }
}

bool AsyncProcessDevice::checkCalibrationResult()
{
    if (m_sharedDataPath.isEmpty()) return false;
    
    QFileInfo fileInfo(m_sharedDataPath);
    if (fileInfo.exists() && fileInfo.size() > 0) {
        // 文件存在且有内容，尝试读取结果
        QFile file(m_sharedDataPath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            
            if (!doc.isNull()) {
                QJsonObject result = doc.object();
                QString dutId = result.value("dutId").toString();
                
                emit calibrationCompleted(dutId, result);
                LOG_MODULE_INFO("AsyncProcessDevice", QString("Calibration completed for DUT: %1").arg(dutId).toStdString());
                return true;
            }
        }
    }
    return false;
}

void AsyncProcessDevice::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    LOG_MODULE_INFO("AsyncProcessDevice", QString("Process finished with exit code: %1").arg(exitCode).toStdString());
    if (m_processType == Calibration) {
        // 标定进程结束，最后检查一次结果
        checkCalibrationResult();
    }
}

void AsyncProcessDevice::onProcessError(QProcess::ProcessError error)
{
    LOG_MODULE_ERROR("AsyncProcessDevice", QString("Process error: %1").arg(static_cast<int>(error)).toStdString());
    m_status = DeviceStatus::Error;
    emit statusChanged(m_status);
}

// IDevice interface implementations
IDevice::DeviceType AsyncProcessDevice::getType() const
{
    return DeviceType::ProcessRunner;
}

IDevice::DeviceStatus AsyncProcessDevice::getStatus() const
{
    return m_status;
}

QString AsyncProcessDevice::getName() const
{
    return m_processType == Activation ? "AsyncActivationDevice" : "AsyncCalibrationDevice";
}

QString AsyncProcessDevice::getDescription() const
{
    return m_processType == Activation ? 
        "Asynchronous activation device with JSON-RPC communication" : 
        "Asynchronous calibration device with file monitoring";
}

void AsyncProcessDevice::setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel>)
{
    // AsyncProcessDevice 不使用传统通信通道
}

std::shared_ptr<Infrastructure::ICommunicationChannel> AsyncProcessDevice::getCommunicationChannel() const
{
    return nullptr;
}

void AsyncProcessDevice::setConfiguration(const QJsonObject &config)
{
    m_config = config;
}

QJsonObject AsyncProcessDevice::getConfiguration() const
{
    return m_config;
}

bool AsyncProcessDevice::selfTest()
{
    return m_status == DeviceStatus::Ready;
}

} // namespace Domain 
