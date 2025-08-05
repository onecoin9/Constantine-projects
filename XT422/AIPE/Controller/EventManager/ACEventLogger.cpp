#include "ACEventLogger.h"
#include "AngKPathResolve.h"

ACEventLogger::ACEventLogger()
{
}

ACEventLogger::~ACEventLogger()
{
}

ACEventLogger* ACEventLogger::eventLogger()
{
	static ACEventLogger eventLogger;
	return &eventLogger;
}

void ACEventLogger::SendEvent(std::string eventJson)
{
	emit EventLogger->sgnSendEvent(eventJson);
}

QString ACEventLogger::EventLogFile()
{
	QString strEventFile = Utils::AngKPathResolve::eventFile();

	return strEventFile;
}
