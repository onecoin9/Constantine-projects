#include "handlerController.h"
#include "tcphandler.h"

HandlerController::HandlerController()
    : m_isEnabled(false)
    , m_isBatchRunning(false)
    , m_protocolVersion("1.0.0")  // 设置默认协议版本
{
}

HandlerController::~HandlerController()
{

}


void HandlerController::getVersion()
{
    QByteArray receive;
    QJsonObject sendObj;
    sendObj["method"] = "getversion";
    QJsonObject dataObj;
    sendObj["data"] = dataObj;

    QJsonDocument sendDoc(sendObj);
    QByteArray sendData = sendDoc.toJson(QJsonDocument::Compact);
    bool result =tcphanlder.sendData(sendData,receive);
}

void HandlerController::setEnable()
{
    QByteArray receive;
    QJsonObject rootObj;
    rootObj["method"] = "setenable";

    QJsonArray dataArray;

    QJsonObject item1;
    item1["bibid"] = 1;
    item1["enable"] = "1110111101111111110111";
    dataArray.append(item1);

    QJsonObject item2;
    item2["bibid"] = 2;
    item2["enable"] = "1111111101111111110111";
    dataArray.append(item2);

    QJsonObject item3;
    item3["bibid"] = 3;
    item3["enable"] = "1111101111111111011111";
    dataArray.append(item3);

    rootObj["data"] = dataArray;

    QJsonDocument doc(rootObj);

    QByteArray sendData = doc.toJson(QJsonDocument::Compact);

    bool result = tcphanlder.sendData(sendData, receive);
}

void HandlerController::lotStart()
{
    QByteArray receive;
    QJsonObject dataObj;
    dataObj["lotid"] = "lot001";
    dataObj["productid"] = "product001";
    dataObj["recipe"] = "recipe001";
    dataObj["hardware"] = "hard001";
    dataObj["operator"] = "op001";
    dataObj["selfcheck"] = false;

    QJsonObject rootObj;
    rootObj["method"] = "lotstart";
    rootObj["data"] = dataObj;

    QJsonDocument doc(rootObj);

    QByteArray sendData = doc.toJson(QJsonDocument::Compact);

    bool result = tcphanlder.sendData(sendData, receive);
}

void HandlerController::lotEnd()
{
    QByteArray receive;
    QJsonObject sendObj;
    sendObj["method"] = "lotend";
    QJsonObject dataObj;
    sendObj["data"] = dataObj;

    QJsonDocument sendDoc(sendObj);
    QByteArray sendData = sendDoc.toJson(QJsonDocument::Compact);
    bool result = tcphanlder.sendData(sendData, receive);
}

void HandlerController::endTest()
{
    QByteArray receive;
    
    QJsonArray sitesArray;

    QJsonObject site1;
    site1["siteid"] = 1;
    site1["result"] = 2;
    sitesArray.append(site1);

    QJsonObject site2;
    site2["siteid"] = 2;
    site2["result"] = 1;
    sitesArray.append(site2);

    QJsonObject site3;
    site3["siteid"] = 3;
    site3["result"] = 0;
    sitesArray.append(site3);

    QJsonObject dataObj;
    dataObj["bibid"] = 1;
    dataObj["sites"] = sitesArray;

    QJsonObject rootObj;
    rootObj["method"] = "testend";
    rootObj["data"] = dataObj;

    QJsonDocument doc(rootObj);

    QByteArray byteArray = doc.toJson(QJsonDocument::Compact);

    bool result = tcphanlder.sendData(byteArray, receive);
}

void HandlerController::sendAlarm()
{
    QByteArray receive;
    QJsonObject sendObj;
    sendObj["method"] = "alarm";
    QJsonObject dataObj;
    dataObj["message"] = "Test fail";
    sendObj["data"] = dataObj;

    QJsonDocument sendDoc(sendObj);
    QByteArray byteArray = sendDoc.toJson(QJsonDocument::Compact);
    bool result = tcphanlder.sendData(byteArray, receive);
}