#ifndef  _JSONRPC_H_
#define  _JSONRPC_H_

#include "ACError.h"
#include "ACTypes.h"
#include <QObject>
#include <QJsonValue>
#include <QTcpSocket>
#include <QHash>
#include "ILog.h"


typedef QHash<QString, QJsonValue> tRPCParamsHash;
typedef QHash<QString, QJsonValue>::iterator tRPCParamsHashItr;

class IRPCSender
{
public:
	virtual int32_t SendRespJson(QByteArray& JsonResp)=0;
};


class CJsonRPCSender : public IRPCSender
{
public:
	CJsonRPCSender(QTcpSocket* pTcpSocket) {
		m_pTcpSocket = pTcpSocket;
	}
	int32_t SendRespJson(QByteArray& JsonResp) {
		int32_t byteswrite = 0;
		uint32_t Size = JsonResp.size();
		QByteArray BytesSend;
		BytesSend.insert(0, (char*)&Size, 4);
		BytesSend.append(JsonResp);
		byteswrite = (int32_t)m_pTcpSocket->write(BytesSend);
		return (byteswrite == BytesSend.size()) ? 0 : ERR_JSONRPC_SendData;

	}
private:
	QTcpSocket* m_pTcpSocket;
};


#endif 