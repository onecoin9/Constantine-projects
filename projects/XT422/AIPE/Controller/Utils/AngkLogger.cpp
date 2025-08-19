#include "AngkLogger.h"
#include "AngKPathResolve.h"
#include "AngKMisc.h"
#include <QBuffer>
#include <QDateTime>
#include <iostream>
#include <fstream>
#include <QDebug>
#include <QString>
#include <QThread>

Utils::AngkLogger::LogLevel Utils::AngkLogger::m_curLogLevel = Utils::AngkLogger::LogLevel::DEBUG;
namespace Utils
{
	AngkLogger::AngkLogger()
	{

	}

	AngkLogger::~AngkLogger()
	{
	}

	AngkLogger* AngkLogger::logger()
	{
		static AngkLogger logger;
		return &logger;
	}

	int AngkLogger::Log(const char* strOutput)
	{
		ALOG_INFO("%s.", "CU", "--", strOutput);
		return 0;
	}

	int AngkLogger::Warning(const char* strOutput)
	{
		ALOG_WARN("%s.", "CU", "--", strOutput);
		return 0;
	}

	int AngkLogger::Error(const char* strOutput)
	{
		ALOG_ERROR("%s.", "CU", "--", strOutput);
		return 0;
	}

	void AngkLogger::PrintLog(LogLevel nLevel, const char* send, const char* recv, const char* fm, ...)
	{
		if (m_curLogLevel > nLevel)
			return;

		QString fileFmt;
		QString srcFmt;

		va_list argList;
		va_start(argList, fm);
		QString strFormat = QString::vasprintf(fm, argList);
		va_end(argList);

		//if (m_curLogLevel == LogLevel::DEBUG)
		//{
		//	fileFmt = QString("[%1:%2]").arg(sfilename).arg(sline);
		//}

		srcFmt = QString("[%1][%2]").arg(send).arg(recv);

		strFormat = fileFmt + srcFmt + strFormat;

		QString endMsg;

		switch (nLevel)
		{
		case Utils::AngkLogger::LogLevel::DEBUG:
			endMsg = "D]" + strFormat;
			break;
		case Utils::AngkLogger::LogLevel::INFO:
			endMsg = "I]" + strFormat;
			break;
		case Utils::AngkLogger::LogLevel::WARN:
			endMsg = "W]" + strFormat;
			break;
		case Utils::AngkLogger::LogLevel::Error:
			endMsg = "E]" + strFormat;
			break;
		case Utils::AngkLogger::LogLevel::FATAL:
			endMsg = "F]" + strFormat;
			break;
		default:
			break;
		}
		QString logMsg = logger()->msg2LogMsg(endMsg);
		emit logger()->sgnLogMsg2FileBuffer(logMsg);
	}

	void AngkLogger::SetLevel(LogLevel nLevel)
	{
		m_curLogLevel = nLevel;
	}

	QString AngkLogger::logFileName()
	{
		QString strFileName = AngKPathResolve::logFile();

		//if (AngKMisc::getFileSize(strFileName) > AngKGlobalInstance::ReadValue("LogFile", "size").toInt() * 1024)
		//{
		//	AngKMisc::deleteFile(strFileName);
		//}
		
		return strFileName;
	}

	QString AngkLogger::msg2LogMsg(const QString& strMsg)
	{
		QString strTime = QDateTime::currentDateTime().toString("[yyyy/MM/dd hh:mm:ss.zzz-");
		return strTime + strMsg /*+ "\n"*/;
	}

}