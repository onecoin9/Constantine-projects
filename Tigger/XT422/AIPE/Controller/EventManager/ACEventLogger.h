#pragma once

#include <QObject>
#include "ACEventBuild.h"

#define EventLogger ACEventLogger::eventLogger()

class ACEventLogger : public QObject
{
	Q_OBJECT

public:
	ACEventLogger();
	~ACEventLogger();

	/// <summary>
	/// 事件记录单例对象，全局唯一
	/// </summary>
	/// <returns>返回静态事件单例对象</returns>
	static ACEventLogger* eventLogger();

	/// <summary>
	/// 事件通过json格式记录并发送
	/// </summary>
	/// <param name="eventJson">事件json字符串</param>
	static void SendEvent(std::string eventJson);

	QString EventLogFile();
signals:
	void sgnSendEvent(std::string);
};
