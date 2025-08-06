#include "JsonRpcHandler.h"
#include <QJsonDocument>
#include <QJsonObject>

void JsonRpcHandler::sendJsonRpcData(acroViewTester* tester) {
    QJsonObject paramsUndefined;
    QString methodName = tester->ui.comboBoxJsonRpcName->currentText();
    tester->handleJsonRequest(methodName, paramsUndefined);
}

void JsonRpcHandler::onJsonRpcResponseReceived(acroViewTester* tester, qint64 id, const QJsonValue& result) {
    qDebug() << "Received response for id=" << id << "Result:" << QJsonDocument(result.toObject()).toJson(QJsonDocument::Indented);
    // ´¦ÀíÏìÓ¦Âß¼­
}

void JsonRpcHandler::connectToJsonRpcServer(acroViewTester* tester) {
    if (!tester->jsonRpcClient.isConnected()) {
        tester->jsonRpcClient.connectToServer("127.0.0.1", 12345);
    }
}