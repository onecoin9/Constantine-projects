#ifndef UTILS_LOGGER_H
#define UTILS_LOGGER_H
#pragma once

#include <fstream>
#include <QtGlobal>
#include <QObject>
#include <QMutex>
#include <QtCore/qalgorithms.h>
#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qstring.h>
#include <QtCore/qvector.h>
#include <QtCore/qset.h>
#include <QtCore/qcontiguouscache.h>
#include "AngKCommonTools.h"
#include "IOutput.h"
class QBuffer;
class AngKMisc;

#pragma execution_character_set("utf-8")

namespace Utils
{
	class AngkLogger : public QObject, public IOutput
	{
		Q_OBJECT

	protected:
		AngkLogger();
		~AngkLogger();

	public:

		enum class LogLevel
		{
			DEBUG = 0,
			INFO,
			WARN,
			Error,
			FATAL
		};

		static AngkLogger* logger();
		QString logFileName();
		QString msg2LogMsg(const QString& strMsg);

		//打印日志
		virtual int32_t Log(int32_t iLevel, const char* strOutput) {
			return 0;
		}
		virtual void DoFlush() { return; };
		virtual int Log(const char* strOutput);
		virtual int Warning(const char* strOutput);
		virtual int Error(const char* strOutput);
		virtual int Message(const char* strOutput) { return 0; }
		virtual int MsgChoose(const char* strOutput) { return 0; };
		virtual void SetProgress(uint uiComplete) {};
		virtual void SetProgPos(uint uiPos) {};
		virtual int  Exit() { return 0; };
		virtual bool BeCanceled() { return true; };
		virtual int Record(const char* strOutput) { return 0; };
		virtual int SetStatus(uint uiStatusArr[], uint uiCount) { return 0; };

		template<LogLevel lvl, typename... Args>
		static void Test1(const char* fm, Args&&... args) {
			if constexpr (lvl > LogLevel::DEBUG)
				return;

			char sprint_buf[1024] = { 0 };
			snprintf(sprint_buf, 1024, fm, std::forward<Args>(args)...);
			QString test = sprint_buf;

			emit logger()->sgnLogMsg2FileBuffer(test);
		}

		/// <summary>
		/// 日志输出函数
		/// </summary>
		/// <param name="nLevel">日志等级</param>
		/// <param name="send">日志发送方</param>
		/// <param name="recv">日志接收方</param>
		/// <param name="fm">日志格式化</param>
		/// <param name="">格式化多参数</param>
		static void PrintLog(LogLevel nLevel, const char* send, const char* recv, const char* fm, ...);
		static void SetLevel(LogLevel nLevel);

	signals:
		void sgnLogMsg2FileBuffer(QString);

	private:
		static LogLevel m_curLogLevel;
	};
}

//[代码位置:代码行数][消息发送方][消息接收方]
#define DEBUGFMT(val) "[%s:%d][%s][%s]" val
#define __FILENAME__ (strrchr("\\" __FILE__, '\\') + 1)
//#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1):__FILE__)
//[消息发送方][消息接收方]
#define NODEBUGFMT(val) "[%s][%s]" val

//#define ALOG_DEBUG(fm, ...)					Utils::AngkLogger::Test1<Utils::AngkLogger::LogLevel::DEBUG>(ASD(fm), __FILE__, __LINE__, __VA_ARGS__)
#define ALOG_DEBUG(fm,send,recv, ...)					Utils::AngkLogger::PrintLog(Utils::AngkLogger::LogLevel::DEBUG, send, recv, fm, __VA_ARGS__)
#define ALOG_INFO(fm,send,recv, ...)					Utils::AngkLogger::PrintLog(Utils::AngkLogger::LogLevel::INFO, send, recv, fm, __VA_ARGS__)
//#define ALOG_INFO(fm, ...)					Utils::AngkLogger::InfoLog(NODEBUGFMT(fm), __VA_ARGS__)
#define ALOG_WARN(fm,send,recv, ...)					Utils::AngkLogger::PrintLog(Utils::AngkLogger::LogLevel::WARN, send, recv, fm, __VA_ARGS__)
#define ALOG_ERROR(fm,send,recv, ...)					Utils::AngkLogger::PrintLog(Utils::AngkLogger::LogLevel::Error, send, recv, fm, __VA_ARGS__)
#define ALOG_FATAL(fm,send,recv, ...)					Utils::AngkLogger::PrintLog(Utils::AngkLogger::LogLevel::FATAL, send, recv, fm, __VA_ARGS__)

#define SET_ALOG_LEVEL(level)				Utils::AngkLogger::logger()->SetLevel(level)
#endif //UTILS_LOGGER_H