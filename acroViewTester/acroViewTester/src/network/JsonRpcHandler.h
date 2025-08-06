#ifndef JSONRPCHANDLER_H
#define JSONRPCHANDLER_H

#include "acroViewTester.h"

class JsonRpcHandler {
public:
    static void sendJsonRpcData(acroViewTester* tester);
    static void onJsonRpcResponseReceived(acroViewTester* tester, qint64 id, const QJsonValue& result);
    static void connectToJsonRpcServer(acroViewTester* tester);
};

#endif // JSONRPCHANDLER_H