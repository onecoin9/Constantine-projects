#pragma once

#include <QDebug>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>
#include <QFile>
#include <QTextStream>
#include <fstream>
#include "ACEventLogger.h"
#include "json.hpp"
#include "AngkLogger.h"

// 定义事件结构
struct Event {
	// 事件相关数据
};

class ACEventManager : public QThread
{
	Q_OBJECT

public:
	ACEventManager(QObject *parent = nullptr);
	~ACEventManager();

	void run() override;

	void stop();

private:
	void processEvent(const std::string& event);
	void recordEventToFile(const std::string& event);

public slots:
	void onSlotReceiveEvent(std::string);
private:
	QMutex m_Mutex;
	QQueue<std::string> m_EventQueue;
	QWaitCondition		m_pCondition;
	std::ofstream		m_EventlogFile;
	bool				m_bIsRunning;
};
