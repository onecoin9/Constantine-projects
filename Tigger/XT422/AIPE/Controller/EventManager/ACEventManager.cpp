#include "ACEventManager.h"
#include "AngkLogger.h"

ACEventManager::ACEventManager(QObject *parent)
	: QThread(parent)
	, m_bIsRunning(false)
{
	connect(EventLogger, &ACEventLogger::sgnSendEvent, this, &ACEventManager::onSlotReceiveEvent, Qt::DirectConnection);
	m_EventlogFile.open(EventLogger->EventLogFile().toUtf8().constData(), std::ios::out | std::ios::app);
}

ACEventManager::~ACEventManager()
{
	stop();
	m_EventlogFile.close();
}

void ACEventManager::run()
{
	m_bIsRunning = true;
	while (m_bIsRunning) {
		QMutexLocker locker(&m_Mutex);
		if (!m_EventQueue.isEmpty()) {
			std::string event = m_EventQueue.dequeue();
			processEvent(event);
			recordEventToFile(event);
		}
		m_pCondition.wait(&m_Mutex, 100); // 等待100毫秒，避免长时间占用CPU
	}
}

void ACEventManager::stop()
{
	m_bIsRunning = false;
	m_pCondition.wakeOne();
	wait();
}

void ACEventManager::processEvent(const std::string& event)
{
	try {
		auto eventJson = nlohmann::json::parse(event);

		if (eventJson.contains("ETime")) {
			std::string strEventTime = eventJson["ETime"].get<std::string>();
		}
		std::string strEventName = eventJson["EName"].get<std::string>();
		std::string strEventSender = eventJson["ESender"].get<std::string>();

	}
	catch (nlohmann::json::exception& e) {
		ALOG_FATAL("Parse Event Json error : %s", "CU", "--", e.what());
	}
}

void ACEventManager::recordEventToFile(const std::string& event)
{
	try {
		auto eventJson = nlohmann::json::parse(event);

		//EventTime
		std::string strEventTime;
		if (eventJson.contains("ETime")) {
			strEventTime = "[" + eventJson["ETime"].get<std::string>() + "]";
		}
		//EventName
		std::string strEventName = "[" + eventJson["EName"].get<std::string>() + "]";


		std::string strIpHop = "";
		if (event.find("\"ipHop\"") != event.npos)
			strIpHop = "[" + eventJson["ipHop"].get<std::string>() + "]";

		//EventSender
		std::string strEventSender = "[" + eventJson["ESender"].get<std::string>() + "]";



		m_EventlogFile << strEventTime.c_str() << strEventName.c_str() << strIpHop.c_str() << strEventSender.c_str() << eventJson.dump().c_str() << std::endl;
		m_EventlogFile.flush();

		//ALOG_INFO("%s", strEventSender.c_str(), "CU", eventJson.dump().c_str());
	}
	catch (nlohmann::json::exception& e) {
		ALOG_FATAL("Parse Event Json failed : %s", "CU", "--", e.what());
	}
}

void ACEventManager::onSlotReceiveEvent(std::string eventJson)
{
	QMutexLocker locker(&m_Mutex);
	m_EventQueue.enqueue(eventJson);
	m_pCondition.wakeOne();
}
