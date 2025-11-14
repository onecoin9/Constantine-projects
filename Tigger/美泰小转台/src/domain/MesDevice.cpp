#include "domain/MesDevice.h"
#include "infrastructure/ICommunicationChannel.h"
#include "DutManager.h"
#include "GlobalItem.h"
#include "core/Logger.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QEventLoop>
#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <QtMath>
namespace Domain {



MesDevice::MesDevice(QObject* parent) 
    : IDevice(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_name("MesDevice")
    , m_type(DeviceType::Unknown)
{
    // 设置默认配置
    m_config["mesUrl"] = "http://127.0.0.1:5000/mes/getchipinfo";
    m_config["timeout"] = 5000; // 5秒超时
    m_config["retryCount"] = 3;
}

MesDevice::~MesDevice()
{
    if (m_networkManager) {
        m_networkManager->deleteLater();
    }
}

MesDevice::DeviceStatus MesDevice::getStatus() const {
    return DeviceStatus::Ready;
}

bool MesDevice::initialize()
{

    try {
        // 检查配置
        QString mesUrl = m_config["mesUrl"].toString();
        if (mesUrl.isEmpty()) {
            emit errorOccurred("MES URL is not configured");
            return false;
        }

        // 测试网络连接
        if (!testConnection()) {
            emit errorOccurred("Failed to connect to MES server");
            return false;
        }
        return true;

    } catch (const std::exception& e) {
        emit errorOccurred(QString("Initialization failed: %1").arg(e.what()));
        return false;
    }
}

bool MesDevice::release()
{
    return true;
}


QString MesDevice::getName() const
{
    return m_name;
}

IDevice::DeviceType MesDevice::getType() const
{
    return m_type;
}

QString MesDevice::getDescription() const
{
    return "MES对接设备，用于扫码和芯片信息查询";
}

QJsonObject MesDevice::executeCommand(const QString &command, const QJsonObject &params)
{
    QJsonObject result;
    result["success"] = false;
    result["command"] = command;
    result["error"] = "";
    //if (!testConnection()) {
    //    result["error"] = "Device is not connected";
    //    return result;
    //}

    //LOG_MODULE_INFO("MesDevice", QString("Start execute %1").arg(command));
    try {
        if (command == "queryChipInfoByUid") {
            auto retObj = handleQueryChipInfoByUid(params);
            if (!retObj["error"].toString().isEmpty())
                LOG_MODULE_ERROR("MesDevice", retObj["error"].toString().toStdString());
            return retObj;
        } 
        else if (command == "queryChipInfoByScan") {
            auto retObj = handleQueryChipInfoByScanCode(params);
            if (!retObj["error"].toString().isEmpty()) {
                LOG_MODULE_ERROR("MesDevice", retObj["error"].toString().toStdString());
                GlobalItem::getInstance().setValue("scanMesResult", 0xFF);
            }
            return result;
        }
        else {
            result["error"] = QString("Unknown command: %1").arg(command);
        }
    } catch (const std::exception& e) {
        result["error"] = QString("Command execution failed: %1").arg(e.what());
    }
    if (!result["error"].toString().isEmpty())
        LOG_MODULE_ERROR("MesDevice", result["error"].toString().toStdString());

    return result;
}

void MesDevice::setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel> channel)
{
    m_commChannel = channel;
}

std::shared_ptr<Infrastructure::ICommunicationChannel> MesDevice::getCommunicationChannel() const
{
    return m_commChannel;
}

void MesDevice::setConfiguration(const QJsonObject &config)
{
    m_config = config;
    
}

QJsonObject MesDevice::getConfiguration() const
{
    return m_config;
}

bool MesDevice::selfTest()
{
    return testConnection();
}

// 私有方法实现
bool MesDevice::testConnection()
{
    QNetworkRequest request;
    request.setUrl(QUrl(m_config["mesUrl"].toString()));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 发送测试请求
    QJsonObject testData;
    testData["version"] = 1;
    testData["data"] = QJsonArray();

    QJsonDocument doc(testData);
    QByteArray data = doc.toJson();

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    
    QNetworkReply* reply = m_networkManager->post(request, data);
    
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    int timeout = m_config["timeout"].toInt(5000);
    timer.start(timeout);
    loop.exec();

    if (timer.isActive()) {
        timer.stop();
        if (reply->error() == QNetworkReply::NoError) {
            reply->deleteLater();
            return true;
        }
    } else {
        // 超时
        reply->abort();
        reply->deleteLater();
        return false;
    }

    reply->deleteLater();
    return false;
}

QJsonObject MesDevice::handleQueryChipInfoByUid(const QJsonObject &params)
{
    QJsonObject result;
    result["result"] = false;

    if (!params.contains("mesUrl")) {
        result["error"] = "Arg error, miss interface";
        return result;
    }

    QString batchNumber = GlobalItem::getInstance().getValue("batchNumber").toString();
    QString ip = params["strIp"].toString();
    QJsonObject requestData;
    QJsonArray dataArray;

    auto siteInfo = Services::DutManager::instance()->getSiteInfoByIp(ip);
    uint16_t sktEn = siteInfo.currentChipMask;
    auto uidMap = siteInfo.uidMap;

    LOG_MODULE_INFO("MesDevice", QString("Current chip mask:%1").arg(sktEn).toStdString());

    for (int i = 0; i < MAX_SOCKET_NUM; i++) {
        if ((sktEn & (1 << i)) != 0) {
            QByteArray uidBytes = siteInfo.uidMap[(uint16_t)(sktEn & (1 << i))];
            QString uid = uidBytes.constData();
            if (uid.length() > 4) {
                QString fUid = uid.mid(0, 4);
                QString sUid = uid.mid(4);
                bool ok;
                uid = QString("%1").arg(fUid.toInt(&ok, 16), 5, 10, QChar('0'))
                        + "-"
                        + QString("%1").arg(sUid.toInt(&ok, 16), 5, 10, QChar('0'));
            }
            QJsonObject chipQuery;
            chipQuery["lotid"] = batchNumber;
            chipQuery["stationid"] = "";
            chipQuery["codeinfo"] = "";
            chipQuery["uid"] = uid;

            if (params.contains("fixedUid")) {
                chipQuery["uid"] = params["fixedUid"].toString();
            }

            dataArray.append(chipQuery);
            LOG_MODULE_INFO("MesDevice", QString("Add chip query, i = %1, real uid = %2").arg(i).arg(uid).toStdString());
        }
    }
    
    if (dataArray.isEmpty()) {
        return result;
    }
    
    requestData["data"] = dataArray;

    // 发送HTTP请求
    QNetworkRequest request;
    request.setUrl(QUrl(params["mesUrl"].toString()));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(requestData);
    QByteArray postData = doc.toJson();

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    LOG_MODULE_INFO("MesDevice", QString("Mes post url:%1 post data:%2").arg(params["mesUrl"].toString()).arg(postData.constData()).toStdString());
    QNetworkReply* reply = m_networkManager->post(request, postData);
    
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    int timeout = params["timeout"].toInt(5000);
    timer.start(timeout);
    loop.exec();

    if (timer.isActive()) {
        timer.stop();
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);

            LOG_MODULE_INFO("MesDevice", QString("Mes recv %1").arg(responseData.constData()).toStdString());
            if (!responseDoc.isNull()) {
                QJsonObject responseObj = responseDoc.object();
                
                result["success"] = true;
                result["response"] = responseObj;
                result["message"] = "Query completed successfully";
                
                // 解析响应 更新设备芯片状态
                int errorcode = responseObj["errorcode"].toInt(0);
                if (errorcode == 1) {
                    QByteArray chipStatus(MAX_SOCKET_NUM, 0);
                    QJsonArray dataArray = responseObj["data"].toArray();
                    for (int i = 0; i < dataArray.count(); i++) {
                        int chipResult = dataArray.at(i)["result"].toInt();
                        if (chipResult == 0) {
                            chipResult = 2;
                        }
                        QString uid = dataArray.at(i)["uid"].toString();
                        uint16_t sktNum = 0;
                        for (auto it = uidMap.begin(); it != uidMap.end(); it++) {
                            QString tmpUid = it.value();
                            if (tmpUid.length() > 4) {
                                QString fUid = tmpUid.mid(0, 4);
                                QString sUid = tmpUid.mid(4);
                                bool ok;
                                tmpUid = QString("%1").arg(fUid.toInt(&ok, 16), 5, 10, QChar('0'))
                                        + "-"
                                        + QString("%1").arg(sUid.toInt(&ok, 16), 5, 10, QChar('0'));
                            }
                            if (tmpUid == uid) {
                                sktNum = it.key();
                                break;
                            }
                        }

                        if (sktNum == 0 || (sktNum & (sktNum - 1)) != 0) {
                            result["error"] = QString("unKnow SKT %1").arg(sktNum);
                            break;
                        }
                        uint16_t sktPos = qFloor(qLn(sktNum) / qLn(2));
                        chipStatus[sktPos] = chipResult;
                        LOG_MODULE_INFO("MesDevice", QString("SktPos: %1 result:%2").arg(sktPos).arg(chipResult).toStdString());
                     }
                    siteInfo.currentChipStatus = chipStatus;
                    LOG_MODULE_INFO("MesDevice", QString("Update %1 chip status:%2").arg(ip).arg(QString(chipStatus.toHex())).toStdString());
                    Services::DutManager::instance()->updateSiteInfoByIp(ip, siteInfo);

                }
            } else {
                result["error"] = "Invalid JSON response from MES";
            }
        } else {
            result["error"] = QString("Network error: %1").arg(reply->errorString());
        }
    } else {
        // 超时
        result["error"] = "Request timeout";
    }

    reply->deleteLater();
    return result;
}


QJsonObject MesDevice::handleQueryChipInfoByScanCode(const QJsonObject& params)
{

    QJsonObject result;
    result["result"] = false;

    if (!params.contains("mesUrl")) {
        result["error"] = "Arg error, miss interface";
        return result;
    }

    QString batchNumber = GlobalItem::getInstance().getValue("batchNumber").toString();
    QString ip = params["strIp"].toString();
    QString interface = params["interface"].toString();
    QJsonObject requestData;
    QJsonArray dataArray;

    QJsonObject chipQuery;
    chipQuery["lotid"] = batchNumber;
    chipQuery["stationid"] = "";
    chipQuery["codeinfo"] = params["codeInfo"].toString();
    chipQuery["uid"] = "";
    dataArray.append(chipQuery);


    if (dataArray.isEmpty()) {
        return result;
    }

    requestData["data"] = dataArray;

    // 发送HTTP请求
    QNetworkRequest request;
    request.setUrl(QUrl(params["mesUrl"].toString()));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(requestData);
    QByteArray postData = doc.toJson();

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QNetworkReply* reply = m_networkManager->post(request, postData);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    int timeout = params["timeout"].toInt(5000);
    timer.start(timeout);
    loop.exec();

    if (timer.isActive()) {
        timer.stop();
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);

            LOG_MODULE_INFO("MesDevice", QString("Mes recv %1").arg(responseData.constData()).toStdString());
            if (!responseDoc.isNull()) {
                QJsonObject responseObj = responseDoc.object();

                result["success"] = true;
                result["response"] = responseObj;
                result["message"] = "Query completed successfully";

                // 解析响应 更新设备芯片状态
                int errcode = responseObj["errcode"].toInt(0);
                if (errcode == 1) {
                    QByteArray chipStatus(MAX_SOCKET_NUM, 0);
                    QJsonArray dataArray = responseObj["data"].toArray();
                    for (int i = 0; i < dataArray.count(); i++) {
                        int chipResult = dataArray.at(i)["result"].toInt();
                        // 记录分选结果
                        GlobalItem::getInstance().setValue("scanMesResult", chipResult);
                    }

                }
            }
            else {
                result["error"] = "Invalid JSON response from MES";
            }
        }
        else {
            result["error"] = QString("Network error: %1").arg(reply->errorString());
        }
    }
    else {
        // 超时
        result["error"] = "Request timeout";
    }

    reply->deleteLater();
    return result;
}

} // namespace Domain