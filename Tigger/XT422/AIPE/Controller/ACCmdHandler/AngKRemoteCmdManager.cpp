#include "AngKRemoteCmdManager.h"
#include "AngKMessageHandler.h"
#include "AngkLogger.h"

AngKRemoteCmdManager::AngKRemoteCmdManager(QObject *parent)
	: QObject(parent)
	, m_pCmdTimer(nullptr)
	, m_mutex(QMutex::Recursive)
	, m_doCmdID(0)
{
	m_pCmdTimer = new QTimer(this);
	//connect(m_pCmdTimer, &QTimer::timeout, this, &AngKRemoteCmdManager::onSlotCmdTimerQueue);
}

AngKRemoteCmdManager::~AngKRemoteCmdManager()
{
	m_pCmdTimer = nullptr;
	delete m_pCmdTimer;
}

void AngKRemoteCmdManager::SetCmdInterval(int mesc)
{
	m_pCmdTimer->setInterval(mesc);
}

void AngKRemoteCmdManager::StartTimer()
{
	m_pCmdTimer->start();
}

void AngKRemoteCmdManager::StopTimer()
{
	m_pCmdTimer->stop();
}

void AngKRemoteCmdManager::AddRemoteCmd(stuCmdData _cmdData)
{
	QMutexLocker locker(&m_mutex);

	m_pQueueCmd.push_back(_cmdData);
	m_mapCmdData[_cmdData.nCmdID] = _cmdData;

	m_doCmdID = m_pQueueCmd.front().nCmdID;
}

bool AngKRemoteCmdManager::DoCmd(uint32_t cmdID, std::string devIP, uint32_t nHopNum)
{
	QMutexLocker locker(&m_mutex);

	if (m_mapCmdData.contains(cmdID)) {
		stuCmdData dataInfo = m_mapCmdData.value(cmdID);
		if (dataInfo.devIP == devIP && dataInfo.nHopNum == nHopNum) {
			int ret = AngKMessageHandler::instance().Command_RemoteDoPTCmd(dataInfo.devIP, dataInfo.nHopNum, dataInfo.nPortID, dataInfo.nCmdFlag, dataInfo.nCmdID, dataInfo.nSKTNum, 8, dataInfo.CmdDataBytes);

			m_mapCmdData.remove(cmdID);
		}
		else {
			return false;
		}

	}

	return true;
}

bool AngKRemoteCmdManager::CheckCmdFinish(uint32_t cmdID)
{
	QMutexLocker locker(&m_mutex);

	if (m_mapCmdData.contains(cmdID)) {
		return false;
	}

	return true;
}

void AngKRemoteCmdManager::onSlotCmdTimerQueue()
{
	if (!m_pQueueCmd.isEmpty() && m_pCmdTimer->isActive()) {

		stuCmdData dataInfo = m_pQueueCmd.dequeue();

		int ret = AngKMessageHandler::instance().Command_RemoteDoPTCmd(dataInfo.devIP, dataInfo.nHopNum, dataInfo.nPortID, dataInfo.nCmdFlag, dataInfo.nCmdID, dataInfo.nSKTNum, 8, dataInfo.CmdDataBytes);
		if(ret == 0)
			m_pCmdTimer->stop();
	}
}