#include "acroViewTester.h"
int globalCmdID = 0;
QString globalCmdRun = "";
void acroViewTester::updateGlobalCmd(const QString& selectedOption) {
    if (selectedOption == "Program") {
        globalCmdID = 0x1800;
        globalCmdRun = "Program";
    } else if (selectedOption == "Erase") {
        globalCmdID = 0x1801;
        globalCmdRun = "Erase";
    } else if (selectedOption == "Verify") {
        globalCmdID = 0x1802;
        globalCmdRun = "Verify";
    } else if (selectedOption == "BlankCheck") {
        globalCmdID = 0x1803;
        globalCmdRun = "BlankCheck";
    } else if (selectedOption == "Secure") {
        globalCmdID = 0x1804;
        globalCmdRun = "Secure";
    } else if (selectedOption == "Read" || selectedOption.isEmpty()) { // 默认值为 Read
        globalCmdID = 0x1806;
        globalCmdRun = "Read";
    } else {
        globalCmdID = 0;
        globalCmdRun = "Unknown";
    }
}

void acroViewTester::onComboBoxDoJobJsonChanged(const QString& selectedOption)
{
    updateGlobalCmd(selectedOption);
    qDebug() << "Selected Option:" << selectedOption;
    qDebug() << "Global CmdID:" << QString::number(globalCmdID, 16).toUpper();
    qDebug() << "Global CmdRun:" << globalCmdRun;
}

DeviceInfo parseDeviceInfo(const QJsonObject& params) {
    DeviceInfo deviceInfo;

    QJsonObject device = params.value("device").toObject();
    deviceInfo.chainID = device.value("chainID").toInt();
    deviceInfo.dpsFpgaVersion = device.value("dpsFpgaVersion").toString();
    deviceInfo.dpsFwVersion = device.value("dpsFwVersion").toString();
    deviceInfo.firmwareVersion = device.value("firmwareVersion").toString();
    deviceInfo.firmwareVersionDate = device.value("firmwareVersionDate").toString();
    deviceInfo.fpgaLocation = device.value("fpgaLocation").toString();
    deviceInfo.fpgaVersion = device.value("fpgaVersion").toString();
    deviceInfo.hopNum = device.value("hopNum").toInt();
    deviceInfo.ip = device.value("ip").toString();
    deviceInfo.isLastHop = device.value("isLastHop").toBool();
    deviceInfo.linkNum = device.value("linkNum").toInt();
    deviceInfo.mac = device.value("mac").toString();

    QJsonObject mainBoardInfo = device.value("mainBoardInfo").toObject();
    deviceInfo.mainBoardInfo.hardwareOEM = mainBoardInfo.value("hardwareOEM").toString();
    deviceInfo.mainBoardInfo.hardwareSN = mainBoardInfo.value("hardwareSN").toString();
    deviceInfo.mainBoardInfo.hardwareUID = mainBoardInfo.value("hardwareUID").toString();
    deviceInfo.mainBoardInfo.hardwareVersion = mainBoardInfo.value("hardwareVersion").toString();

    deviceInfo.muAppVersion = device.value("muAppVersion").toString();
    deviceInfo.muAppVersionDate = device.value("muAppVersionDate").toString();
    deviceInfo.muLocation = device.value("muLocation").toString();
    deviceInfo.port = device.value("port").toString();
    deviceInfo.siteAlias = device.value("siteAlias").toString();
    deviceInfo.ipHop = params.value("ipHop").toString();

    return deviceInfo;
}

ProjectInfo parseProjectInfo(const QJsonObject& data) {
    ProjectInfo projectInfo;
    projectInfo.errorCode = data.value("errorCode").toInt();
    projectInfo.message = data.value("message").toString();
    projectInfo.details = data.value("details").toString();
    return projectInfo;
}

JobResult parseJobResult(const QJsonObject& data) {
    JobResult jobResult;
    jobResult.BPUID = data.value("BPUID").toString();
    jobResult.SKTIdx = data.value("SKTIdx").toInt();
    jobResult.nHopNum = data.value("nHopNum").toInt();
    jobResult.result = data.value("result").toString();
    jobResult.strip = data.value("strip").toString();
    return jobResult;
}

void acroViewTester::addItemsToHandlers(const QList<QString>& items)
{
    for (const QString& item : items) {
        if (item == "StartService") {
            handlers[item] = [this]() { sendStartService(); };
        }
        else if (item == "SiteScanAndConnect") {
            handlers[item] = [this]() { sendSiteScanAndConnect(); };
        }
        else if (item == "LoadProject") {
            handlers[item] = [this]() { sendLoadProject(); };
        }
        else if (item == "SetAdapterEn") {
            handlers[item] = [this]() { sendAdapterEn(); };
        }
        else if (item == "GetAdapterEn") {
            handlers[item] = [this]() { sendGetAdapterEn(); };
        }
        else if (item == "DoJob") {
            handlers[item] = [this]() { sendDoJob(); };
        }
        else if (item == "DoCustom") {
            handlers[item] = [this]() { sendDoCustom(); };
        }
        else if (item == "GetProjectInfo") {
            handlers[item] = [this]() { sendGetProjectInfo(); };
        }
        else if (item == "LogInterface") {
            handlers[item] = [this]() { sendLogInterface(); };
        }
        else if (item == "EventInterface") {
            handlers[item] = [this]() { sendEventInterface(); };
        }
        else if (item == "GetProgrammerInfo") {
            handlers[item] = [this]() { sendGetProgrammerInfo(); };
        }
        else if (item == "GetJobResult") {
            handlers[item] = [this]() { sendGetJobResult(); };
        }
        else if (item == "SendSiteStatus") {
            handlers[item] = [this]() { sendGetSiteStatus(); };
        }
        else if (item == "GetSktInfo") {
            handlers[item] = [this]() { sendGetSktInfo(); };
        }
        else {
            handlers[item] = [item]() { qDebug() << "Handler not implemented for" << item; };
        }
    }
}

void acroViewTester::sendStartService()
{
    qDebug() << "Sending StartService...";
    openFileApp();
}

void acroViewTester::sendAdapterEn()
{
    qDebug() << "sendAdapterEn...";
    QJsonObject params;
    // 添加具体参数
    jsonRpcClient.sendRequest("sendAdapterEn", params);
}
void acroViewTester::sendSiteScanAndConnect()
{
    QJsonObject paramsSiteScanAndConnect;
    jsonRpcClient.sendRequest("SiteScanAndConnect", paramsSiteScanAndConnect);
    qDebug() << "Sending SiteScanAndConnect...";

}


void acroViewTester::sendLoadProject()
{
    QJsonObject params;
    QSettings settings("AcroView", "acroViewTester");
    QString path = settings.value("Paths/ProjectPath").toString();
    if (path.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入路径");
        return;
    }
    QString fileName = settings.value("Paths/AutoPath").toString();
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择项目文件");
        return;
    }
    QFileInfo fileInfo(fileName);
    QString baseFileName = fileInfo.fileName();

    // 将 baseFileName 用于 JSON 参数
    params["taskFileName"] = baseFileName;
    params["path"] = path;


    jsonRpcClient.sendRequest("LoadProject", params);
}

void acroViewTester::sendGetAdapterEn()
{
    qDebug() << "Sending GetAdapterEn...";
    QJsonObject params;
    // 添加具体参数
    jsonRpcClient.sendRequest("GetAdapterEn", params);
}

void acroViewTester::sendDoJob()
{
    qDebug() << "Sending DoJob...";

    QString progIndex_for_logging = "BPU0_Site1";
    QString actual_strIP = deviceInfo.ip;
    int actual_nHopNum = 0;
    quint32 actual_SktEn = 0x0000FFFF;

    QJsonObject docmdSeqJson;
    docmdSeqJson.insert("CmdID", QString::number(globalCmdID, 16).toUpper());
    docmdSeqJson.insert("CmdRun", globalCmdRun);
    docmdSeqJson.insert("CmdSequencesGroupCnt", 1);

    QJsonObject params;
    params.insert("strIP", actual_strIP);
    params.insert("nHopNum", actual_nHopNum);
    params.insert("PortID", 0);
    params.insert("CmdFlag", 0);
    params.insert("CmdID", static_cast<int>(eSubCmdID::SubCmd_MU_DoCmdSequence));
    params.insert("SKTEn", static_cast<double>(actual_SktEn));
    params.insert("BPUID", 8);
    params.insert("docmdSeqJson", docmdSeqJson);

    jsonRpcClient.sendRequest("DoJob", params);
}

void acroViewTester::sendDoCustom()
{
    //qDebug() << "Sending DoCustom...";
    //QString testStep;
    //QJsonObject commands;
    //// 构造params对象
    //QJsonObject params;
    //params.insert("testStep", testStep);
    //params.insert("commands", commands);

    QString progIndex_for_logging = "BPU0_Site1";
    QString actual_strIP = deviceInfo.ip;
    int actual_nHopNum = 0;
    quint32 actual_SktEn = 0x0000FFFF;

    QJsonObject docmdSeqJson;
    docmdSeqJson.insert("CmdID", QString::number(globalCmdID, 16).toUpper());
    docmdSeqJson.insert("CmdRun", globalCmdRun);
    docmdSeqJson.insert("CmdSequencesGroupCnt", 1);

    QJsonObject params;
    params.insert("strIP", actual_strIP);
    params.insert("nHopNum", actual_nHopNum);
    params.insert("PortID", 0);
    params.insert("CmdFlag", 0);
    params.insert("CmdID", static_cast<int>(eSubCmdID::SubCmd_MU_DoCustom));
    params.insert("SKTEn", static_cast<double>(actual_SktEn));
    params.insert("BPUID", 8);
    params.insert("docmdSeqJson", docmdSeqJson);
    // 发送请求
    jsonRpcClient.sendRequest("DoCustom", params);
}

void acroViewTester::sendGetProjectInfo()
{
    qDebug() << "Sending GetProjectInfo...";
    QJsonObject params;
    // 添加具体参数
    jsonRpcClient.sendRequest("GetProjectInfo", params);
}

void acroViewTester::sendLogInterface()
{
    qDebug() << "Sending LogInterface...";
    QJsonObject params;
    // 添加具体参数
    jsonRpcClient.sendRequest("LogInterface", params);
}

void acroViewTester::sendEventInterface()
{
    qDebug() << "Sending EventInterface...";
    QJsonObject params;
    // 添加具体参数
    jsonRpcClient.sendRequest("EventInterface", params);
}

void acroViewTester::sendGetProgrammerInfo()
{
    qDebug() << "Sending GetProgrammerInfo...";
    QJsonObject params_getConfig; // No params needed for this example method
    params_getConfig["project_url"] = "C:/Users/Administrator/Desktop/XT422/AIPE/Build/project/default.eapr";
    jsonRpcClient.sendRequest("GetProjectInfoExt", params_getConfig,
        // Success Callback
        [](const QJsonValue& result) {
            QJsonObject logOutput;
            logOutput["status"] = "GetProjectInfoExt Success";
            // Assuming 'result' is an object as per your original code.
            // If 'result' could be any QJsonValue type (object, array, string, number, bool, null),
            // you can directly assign it:
            // logOutput["data"] = result;
            // If you are sure it's an object and want to ensure that:
            if (result.isObject()) {
                logOutput["data"] = result.toObject();
            }
            else if (result.isArray()) {
                logOutput["data"] = result.toArray();
            }
            else {
                logOutput["data"] = result; // Store as is (string, number, bool, null)
            }

            // Use .noquote() to prevent qDebug from adding extra quotes around the JSON string
            qDebug().noquote() << QJsonDocument(logOutput).toJson(QJsonDocument::Indented);
        },
        // Error Callback
            [](int code, const QString& message, const QJsonValue& data) {
            qWarning() << "GetProjectInfoExt Error:" << code << message << data;
        }
        );
}

void acroViewTester::sendGetJobResult()
{
    qDebug() << "Sending GetJobResult...";
    QJsonObject params;
    // 添加具体参数
    jsonRpcClient.sendRequest("GetJobResult", params);
}

void acroViewTester::sendGetSktInfo()
{
    QJsonObject params_getConfig; // No params needed for this example method
    jsonRpcClient.sendRequest("GetSKTInfo", params_getConfig,
        // Success Callback
        [](const QJsonValue& result) {
            qDebug() << "GetSKTInfo Success:" << QJsonDocument(result.toObject()).toJson(QJsonDocument::Indented);
        },
        // Error Callback
            [](int code, const QString& message, const QJsonValue& data) {
            qWarning() << "GetSKTInfo Error:" << code << message << data;
        }
        );
}

void acroViewTester::sendGetSiteStatus()
{
    QJsonObject jsonObject = collectAllBlocksStatus();
    QJsonDocument jsonDoc(jsonObject);
    QByteArray jsonData = jsonDoc.toJson();

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qWarning() << "TestSite: Failed to parse JSON data:" << jsonError.errorString();
        return;
    }
    QJsonObject jsonObj = doc.object();

    jsonRpcClient.sendRequest("SiteSocketInit", jsonObj,
        [this](const QJsonValue& result) {
            qDebug() << "sendBlocksStatus succeeded.";

        },
        [](int code, const QString& message, const QJsonValue& data) {
            qWarning() << "sendBlocksStatus failed with error code" << code << "and message:" << message;

        },
            5000
            );
}

void acroViewTester::loadComboBoxItems()
{
    QList<ItemText> items;
    items.append({ "StartService" });
    items.append({ "SiteScanAndConnect" });
    items.append({ "LoadProject" });
    items.append({ "SetAdapterEn" });
    items.append({ "GetAdapterEn" });
    items.append({ "DoJob" });
    items.append({ "DoCustom" });
    items.append({ "GetProjectInfo" });
    items.append({ "GetProgrammerInfo" });
    items.append({ "GetSktInfo" });
    items.append({ "SendSiteStatus" });
    items.append({ "SetAdapterEn" });
    items.append({ "GetAdapterEn" });
    items.append({ "LogInterface" });
    items.append({ "EventInterface" });
    items.append({ "GetJobResult " });
    items.append({ "GetProgrammerInfo" });
    items.append({ "GetSiteStatus" });
    ui.comboBoxJsonRpcName->clear();
    QList<QString> stringItems;
    for (const auto& item : items)
    {
        stringItems.append(item.text);
    }
    addItemsToHandlers(stringItems);
    for (const auto& item : items)
    {
        ui.comboBoxJsonRpcName->addItem(item.text);
    }
}

void acroViewTester::onJsonRpcConnected()
{
    qDebug() << "Successfully connected!";
    ui.jsonRpcResultLabel->setText("Successfully connected!");
    ui.pushButtonOpenJsonRPC->setText("断开连接");
}

void acroViewTester::onJsonRpcDisconnected()
{
    qWarning() << "Disconnected from server.";
    ui.jsonRpcResultLabel->setText("Disconnected from JSON-RPC server.");
    ui.pushButtonOpenJsonRPC->setText("连接服务器");
}

void acroViewTester::onJsonRpcSocketError(QAbstractSocket::SocketError error)
{
    qCritical() << "Socket Error:" << error;
    ui.jsonRpcResultLabel->setText(QString("Socket Error: %1").arg(error));
    ui.pushButtonOpenJsonRPC->setText("重新连接");

}

void acroViewTester::onJsonRpcProtocolError(const QString& errorString)
{
    qCritical() << "Protocol Error:" << errorString;
    ui.jsonRpcResultLabel->setText(QString("Protocol Error: %1").arg(errorString));

}

void acroViewTester::onJsonRpcServerCommandReceived(const QString& cmd, const QJsonObject& data)
{
    qInfo() << "Received Command from Server - Cmd:" << cmd << "Data:" << QJsonDocument(data).toJson(QJsonDocument::Compact);
    if (cmd == "LogOut") {
        qInfo() << "Server requested logout.";
        jsonRpcClient.disconnectFromServer();
        // a.quit(); // If you have a QCoreApplication or QApplication instance
    }
}

void acroViewTester::connectToJsonRpcServer()
{
    if (jsonRpcClient.isConnected()) {
        jsonRpcClient.disconnectFromServer();
        ui.pushButtonOpenJsonRPC->setText("连接服务器");
        return;
    }
    
    // 否则，尝试建立连接
    QString host = "127.0.0.1";  
    int port = 12345;            
    
    ui.jsonRpcResultLabel->setText("Connecting to JSON-RPC server...");
    ui.pushButtonOpenJsonRPC->setEnabled(false);  
    
    // 为了防止 UI 卡死，可以使用异步连接
    QTimer::singleShot(0, this, [=]() {
        jsonRpcClient.connectToServer(host, port);
        ui.pushButtonOpenJsonRPC->setEnabled(true);  
    });
}


void acroViewTester::onJsonRpcResponseReceived(qint64 id, const QJsonValue& result)
{
    qDebug() << "Received response for id=" << id << "Result:" << QJsonDocument(result.toObject()).toJson(QJsonDocument::Indented);
    ui.jsonRpcResultTextBrowser->setText(QJsonDocument(result.toObject()).toJson(QJsonDocument::Indented));
    if (!result.isObject()) {
        qWarning() << "Result is not an object.";
        return;
    }

    QJsonObject jsonObj = result.toObject();
    QJsonObject params = jsonObj.value("params").toObject();
    if (params.isEmpty()) {
        qWarning() << "Params is empty.";
        //return;
    }

    QString method = jsonObj.value("method").toString();
    if (!method.isEmpty()) {
        if (method == "DeviceDiscovered")
        {
            auto addRow = [&](const QString& name, const QString& value) {
                QStandardItem* item_name = new QStandardItem(name);
                QStandardItem* item_value = new QStandardItem(value);
                jsonModels[currentModelIndex]->appendRow({ item_name, item_value });
            };

        deviceInfo = parseDeviceInfo(params);
        addRow("Chain ID", QString::number(deviceInfo.chainID));
        addRow("DPS FPGA Version", deviceInfo.dpsFpgaVersion);
        addRow("DPS FW Version", deviceInfo.dpsFwVersion);
        addRow("Firmware Version", deviceInfo.firmwareVersion);
        addRow("Firmware Version Date", deviceInfo.firmwareVersionDate);
        addRow("FPGA Location", deviceInfo.fpgaLocation);
        addRow("FPGA Version", deviceInfo.fpgaVersion);
        addRow("Hop Num", QString::number(deviceInfo.hopNum));
        addRow("IP", deviceInfo.ip);
        addRow("Is Last Hop", deviceInfo.isLastHop ? "Yes" : "No");
        addRow("Link Num", QString::number(deviceInfo.linkNum));
        addRow("MAC", deviceInfo.mac);
        addRow("Hardware OEM", deviceInfo.mainBoardInfo.hardwareOEM);
        addRow("Hardware SN", deviceInfo.mainBoardInfo.hardwareSN);
        addRow("Hardware UID", deviceInfo.mainBoardInfo.hardwareUID);
        addRow("Hardware Version", deviceInfo.mainBoardInfo.hardwareVersion);
        addRow("MU App Version", deviceInfo.muAppVersion);
        addRow("MU App Version Date", deviceInfo.muAppVersionDate);
        addRow("MU Location", deviceInfo.muLocation);
        addRow("Port", deviceInfo.port);
        addRow("Site Alias", deviceInfo.siteAlias);
        addRow("IP Hop", deviceInfo.ipHop);

            if (currentModelIndex < 7)
            {
                currentModelIndex = (currentModelIndex + 1) % jsonModels.size();
            }
            else
            {
                mergeModels(jsonModels, jsonRpcResultModel);
                ui.tableViewJsonRpcResult->setModel(jsonRpcResultModel);
            }
        }
        else if (method == "ClientDoCmd")
        {
            auto addRow1 = [&](const QString& name, const QString& value) {
                QStandardItem* item_name = new QStandardItem(name);
                QStandardItem* item_value = new QStandardItem(value);
                jsonModels1[currentModelIndex1]->appendRow({ item_name, item_value });
            };

        QString cmd = params.value("cmd").toString();
        QJsonObject data = params.value("data").toObject();
        jobResult = parseJobResult(data);

        if (currentModelIndex1 >= 0 && currentModelIndex1 < jsonModels1.size()) {
            addRow1("Command", cmd);
            addRow1("BPUID", jobResult.BPUID);
            addRow1("SKTIdx", QString::number(jobResult.SKTIdx));
            addRow1("nHopNum", QString::number(jobResult.nHopNum));
            addRow1("Result", jobResult.result);
            addRow1("Strip", jobResult.strip);
        }
        else {
            qWarning() << "currentModelIndex1 is out of range!";
        }

        if (currentModelIndex1 < 7)
        {
            currentModelIndex1 = (currentModelIndex1 + 1) % jsonModels1.size();
        }
        else
        {
            mergeModels(jsonModels1, jsonRpcResultModel1);
            ui.tableViewJsonDoJobResult->setModel(jsonRpcResultModel1);
        }
    }
    else if (method == "SiteSocketInit")
    {
        QJsonObject data = params.value("data").toObject();
        //addRow("Site", QString::number(data.value("sockets").toInt()));
        //此处还需要详细设计
    }
    else if (method == "DoCustom")
    {
        QJsonObject data = params.value("data").toObject();
        //addRow("Site", QString::number(data.value("sockets").toInt()));
        //此处还需要详细设计
    }
        else if (method == "LoadProjectResult")
        {
            QString cmd = params.value("cmd").toString();
            QString data = params.value("data").toString();

            QStandardItem* item_cmd = new QStandardItem("Command");
            QStandardItem* item_cmd_value = new QStandardItem(cmd);

            QStandardItem* item_result = new QStandardItem("Result");
            QStandardItem* item_result_value = new QStandardItem(data);

            jsonRpcResultModel->clear();
            jsonRpcResultModel->setHorizontalHeaderLabels({ "Attribute", "Value" });
            jsonRpcResultModel->appendRow({ item_cmd, item_cmd_value });
            jsonRpcResultModel->appendRow({ item_result, item_result_value });

            ui.tableViewJsonRpcResult->setModel(jsonRpcResultModel);

            // 在文本浏览器中显示完整的JSON
            ui.jsonRpcResultTextBrowser->setText(QJsonDocument(result.toObject()).toJson(QJsonDocument::Indented));

            // 如果需要，可以在这里更新UI或执行其他相关操作
            if (data == "success") {
                qDebug() << "Project loaded successfully";
                // 更新UI状态或显示成功消息
            }
            else {
                qWarning() << "Project loading failed";
                // 显示错误消息
            }
        }
        //else
        //{
        //    jsonRpcResultModel->clear();
        //    jsonRpcResultModel->setHorizontalHeaderLabels({ "Attribute", "Value" });
        //    jsonRpcResultModel->appendRow(
        //        {
        //        new QStandardItem("Method"),
        //        new QStandardItem(method)
        //        });
        //    jsonRpcResultModel->appendRow(
        //        {
        //        new QStandardItem("Params"),
        //        // new QStandardItem(QJsonDocument(params).toJson(QJsonDocument::Indented))
        //        });
        //}

        //qWarning() << "Method is empty.";
        //return;
    }
    QString jsonResult = jsonObj.value("message").toString();

    if(!jsonResult.isEmpty())
    {
    if (jsonResult == "DoCustom")
    {
        QJsonObject data = params.value("data").toObject();
        //addRow("Site", QString::number(data.value("sockets").toInt()));
        //此处还需要详细设计
    }
    }

}

void acroViewTester::sendJsonRpcData()
{
    ///Example 1: Call GetConfig_Json with callback
 /*   QJsonObject params_getConfig;
    jsonRpcClient.sendRequest("CreateProjectByTemplate", params_getConfig,
        [](const QJsonValue& result) {
            qDebug() << "GetConfig_Json Success:" << QJsonDocument(result.toObject()).toJson(QJsonDocument::Indented);
        },
        [](int code, const QString& message, const QJsonValue& data) {
            qWarning() << "GetConfig_Json Error:" << code << message << data;
        }
        );*/

        /// Example 2: Call a method with parameters (assuming method "Add" exists)
        //QJsonArray params_add;
        //params_add.append(5);
        //params_add.append(10);
        //jsonRpcClient.sendRequest("Add", params_add,
        //    [](const QJsonValue& result) { qDebug() << "Add Result:" << result.toInt(); },
        //    [](int code, const QString& message, const QJsonValue& data) { qWarning() << "Add Error:" << code << message << data; }
        //);

        /// Example 3: Send a notification
        //QJsonObject params_log;
        //params_log["level"] = "info";
        //params_log["message"] = "Client connected and sent requests.";
        //jsonRpcClient.sendNotification("LogEvent", params_log);

    QJsonObject paramsUndefined;
    QString methodName = ui.comboBoxJsonRpcName->currentText();
    handleJsonRequest(methodName, paramsUndefined);

}

void acroViewTester::handleJsonRequest(const QString& methodName, const QJsonObject& params)
{

    auto it = handlers.find(methodName);
    if (it != handlers.end())
    {
        it->second();
    }
    else
    {
        jsonRpcClient.sendRequest(methodName, params,
            [](const QJsonValue& result) {
                qDebug() << "GetConfig_Json Success:" << QJsonDocument(result.toObject()).toJson(QJsonDocument::Indented);
            },
            [](int code, const QString& message, const QJsonValue& data) {
                qWarning() << "GetConfig_Json Error:" << code << message << data;
            }
            );
    }
}

QJsonObject acroViewTester::collectAllBlocksStatus()
{
    QJsonObject jsonRpcObject;
    QJsonArray sitesArray;

    for (TestSite* site : m_testSites)
    {
        QJsonObject siteStatus = site->collectBlocksStatus();
        sitesArray.append(siteStatus);
    }

    jsonRpcObject["Site"] = sitesArray;

    qDebug() << "sitesArray:" << sitesArray;
    qDebug() << "jsonRpcObject:" << jsonRpcObject;

    return jsonRpcObject;
}

void acroViewTester::on_pushButtonSendUID_clicked()
{
    QDate currentDate = QDate::currentDate();

    int year = currentDate.year();
    int month = currentDate.month();
    int day = currentDate.day();

    int bit31 = 1;
    int bit30 = 0;
    int bit29_27 = 0;
    int bit26_23 = (year - 2021)%10; 
    int bit22_19 = month;
    int bit18_14 = day;
    int bit13_0 = 1234; 

    quint32 uid = 0;
    uid |= (bit31 << 31);
    uid |= (bit30 << 30);
    uid |= (bit29_27 << 27);
    uid |= (bit26_23 << 23);
    uid |= (bit22_19 << 19);
    uid |= (bit18_14 << 14);
    uid |= (bit13_0 & 0x3FFF);

    QString uidHex = QString("0x%1").arg(uid, 8, 16, QChar('0')).toUpper();

   // QMessageBox::information(this, "生成的 UID", QString("生成的 UID 为: %1").arg(uidHex));
 
    QJsonObject params;
    QString strIP = "192.168.70.109";
    quint32 hopname = 0;
    params["uuid"] = uidHex;
    params["strIP"] = strIP;
    params["nHopNum"] = static_cast<double>(hopname);
    quint32 actual_SktEn = 0x0000FFFF; // 例如，所有 site 使能
    params["sktEnable"] = static_cast<double>(actual_SktEn);

    jsonRpcClient.sendRequest("SendUUID", params,
        // 成功回调
        [](const QJsonValue& result) { // 捕获 filePath 用于日志
            qDebug() << "SendUUID Success Result:" << QJsonDocument::fromVariant(result.toVariant()).toJson(QJsonDocument::Indented);
            // 在这里处理成功加载项目的逻辑，例如更新 UI
        },
        // 错误回调
            [](int code, const QString& message, const QJsonValue& data) { // 捕获 filePath 用于日志
            qWarning() << "SendUUID Error Code:" << code << "Message:" << message << "Data:" << data;
            // 在这里处理加载项目失败的逻辑，例如显示错误消息
        }
        );

}