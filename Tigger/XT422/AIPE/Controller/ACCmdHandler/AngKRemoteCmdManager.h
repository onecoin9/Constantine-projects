#pragma once

#include <QObject>
#include <QQueue>
#include <QTimer>
#include <QMutex>
#include <QMap>

struct stuCmdData
{
	std::string devIP;
	uint32_t nHopNum;
	uint32_t nPortID;
	uint32_t nCmdFlag;
	uint32_t nCmdID;
	uint16_t nSKTNum;
	uint16_t nBPUID;
	QByteArray CmdDataBytes;
};

class AngKRemoteCmdManager : public QObject
{
	Q_OBJECT

public:
	AngKRemoteCmdManager(QObject* parent = nullptr);
	~AngKRemoteCmdManager();

	void SetCmdInterval(int mesc);

	void StartTimer();

	void StopTimer();

	void AddRemoteCmd(stuCmdData _cmdData);

	bool DoCmd(uint32_t cmdID, std::string devIP, uint32_t nHopNum);

	bool CheckCmdFinish(uint32_t cmdID);
public slots:
	void onSlotCmdTimerQueue();

private:
	QTimer* m_pCmdTimer;
	QQueue<stuCmdData>	m_pQueueCmd;
	QMap<uint32_t, stuCmdData>	m_mapCmdData;
	QMutex m_mutex;
	uint32_t m_doCmdID;
};