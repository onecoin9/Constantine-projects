#include "ACHtmlLogWriter.h"
#include "AngKPathResolve.h"
#include "AngkLogger.h"
#include <QTime>
#include <QTextStream>

#define STR_ENDTAG "<hr/>\r\n</div>\r\n</body>\r\n</html>"

#define LOG_MSG_MAXLEN (2048)
enum eLogLevel {
	LOGLEVEL_LOG,
	LOGLEVEL_WARNING,
	LOGLEVEL_ERR,
	LOGLEVEL_CRIST,
	LOGLEVEL_SUCCESS,
	LOG_NOSHOW = 0x10000000,	///直接输出到log文件中，不打印到屏幕上
};

QString ACHtmlLogWriter::GetReportTemplet()
{
	QString strExe = Utils::AngKPathResolve::localReportTempFilePath();
	QString strTemplet = strExe + "ReportTemplet.html";
	return strTemplet;
}
QString ACHtmlLogWriter::GetLogTemplet()
{
	QString strExe = Utils::AngKPathResolve::localReportTempFilePath();
	QString strTemplet = strExe + "LogTemplet.html";
	return strTemplet;
}

ACHtmlLogWriter::ACHtmlLogWriter(QObject *parent)
	: QFile(parent)
	, m_bTimeHead(true)
	, m_nEndPos(0)
{
}

ACHtmlLogWriter::~ACHtmlLogWriter()
{
}

bool ACHtmlLogWriter::WriteLog(QString& strLog)
{
	int len = strLog.length();
	m_Mutex.lock();
	int curEndPos = size();
	seek(curEndPos - 31);
	writeData(strLog.toStdString().c_str(), strLog.length());
	AppendEndTag();
	flush();
	m_Mutex.unlock();
	return true;
}

bool ACHtmlLogWriter::InsertLog(int LogLevel, const char* fmt, ...)
{
	bool Ret = true;
	int offset = 0;
	char sprint_buf[LOG_MSG_MAXLEN];
	QString LogStatus;
	QString LogShort = "N";
	va_list args;
	QString strLine, strLog;
	memset(sprint_buf, 0, LOG_MSG_MAXLEN);
	switch (LogLevel) {
	case LOGLEVEL_LOG:
		LogStatus = "lognormal";///Normal
		LogShort = "N";
		break;
	case LOGLEVEL_ERR:
		LogStatus = "logerror";///Error
		LogShort = "E";
		break;
	case LOGLEVEL_WARNING:
		LogStatus = "logwarning";///Warning
		LogShort = "W";
		break;
	default:
		LogStatus = "lognormal";///Normal
		break;
	}
	if (m_bTimeHead) {
		QString strTime = QDateTime::currentDateTime().toString("[yyyy/MM/dd hh:mm:ss-%1]").arg(LogShort);
		strLine += strTime;
	}
	va_start(args, fmt);
	vsprintf(sprint_buf + offset, fmt, args);
	va_end(args); /* 将argp置为NULL */
	if (strlen(sprint_buf) > LOG_MSG_MAXLEN - 2) {
		return false;
	}
	QString strOrg = QString("%1").arg(sprint_buf);
	strOrg.replace(" ", "&nbsp");
	strLog = QString("<p class = '%1'>%2</p><br/>\r\n").arg(LogStatus).arg(strOrg);
	strLine += strLog;
	return WriteLog(strLine);
}

void ACHtmlLogWriter::AppendEndTag()
{
	write(STR_ENDTAG, (uint32_t)strlen(STR_ENDTAG));
}

bool ACHtmlLogWriter::ConfigDumpBegin()
{
	QString strTime;
	strTime = QDateTime::currentDateTime().toString("[yyyy/MM/dd hh:mm:ss]");
	QString strLog = QString("\
		<div id = \"configinfo\">\r\n\
		<p class='logtime'>%1 &nbsp;</p><p class = 'SectionHeader button'><span>+</span>Chip Config</p><br/>\r\n\
		<div style = \"display:none; \">\r\n\
		<table><tr><td style='font:12pt Consolas; font-weight:bold;'><center>Chip Config</center></td></tr></table>\r\n\
		<tr><td><hr/></td></tr>\r\n").arg(strTime);
	return WriteLog(strLog);
}

bool ACHtmlLogWriter::SktSimpleDumpBegin()
{
	QString strTime;
	strTime = QDateTime::currentDateTime().toString("[yyyy/MM/dd hh:mm:ss]");
	QString strLog = QString("\
					<div id = \"AdapterInformation\">\r\n\
					<p class='logtime'>%1 &nbsp;</p><p class = 'SectionHeader button'><span>+</span>Adapter Information</p><br/>\r\n\
					<div style = \"display:none; \">\r\n\
					<table><tr><td style='font:12pt Consolas; font-weight:bold;'><center>Adapter Information</center></td></tr></table>\r\n\
					<tr><td><hr/></td></tr>\r\n").arg(strTime);
	return WriteLog(strLog);
}

bool ACHtmlLogWriter::InsertSktSimpleInfo(QString strInfo)
{
	return InsertConfigLog(strInfo);
}

bool ACHtmlLogWriter::SktSimpleDumpEnd()
{
	QString strLine = "\
					<tr><td><hr/></td></tr>\r\n\
					</div>\r\n\
					</div>\r\n";
	return WriteLog(strLine);
}

bool ACHtmlLogWriter::InsertConfigLog(QString strInfo)
{
	QString strLine;
	strInfo.replace(" ", "&nbsp");
	strInfo.replace("\r\n", "&nbsp");
	strLine = QString("<p class='lognormal'>%1</p><br/>\r\n").arg(strInfo);
	return WriteLog(strLine);
}

bool ACHtmlLogWriter::ConfigDumpEnd()
{
	QString strLine = "\
		<tr><td><hr/></td></tr>\r\n\
		</div>\r\n\
		</div>\r\n";
	return WriteLog(strLine);
}


bool ACHtmlLogWriter::CreateLog(QString strLogFile, int LogType)
{
	bool Ret = true;
	uchar* pByte = NULL;
	m_Mutex.lock();
	//Ret=Open(strLogFile,CFile::modeWrite|CFile::modeCreate|CFile::shareDenyWrite,NULL);
	setFileName(strLogFile);
	Ret = open(QIODevice::WriteOnly | QIODevice::Truncate);
	if (Ret) {
		QString strTemplet;
		if (LogType == LOGTYPE_NORMAL) {
			strTemplet = GetLogTemplet();
		}
		else {
			strTemplet = GetReportTemplet();
		}

		QFile tmpFile(strTemplet);
		if (tmpFile.open(QIODevice::ReadOnly)) {
			QByteArray pByte = tmpFile.readAll(); // 读取模板文件的全部内容
			write(pByte); // 写入日志文件
			tmpFile.close();
		}
		else {
			ALOG_ERROR("Unable to open the template file.", "CU", "--");
			return false;
		}
	} else {
		ALOG_ERROR("Unable to open the log file.", "CU", "--");
        return false;
    }

	//close();

	if (pByte) {
		delete[] pByte;
	}
	m_nEndPos = (int)strlen(STR_ENDTAG);
	m_Mutex.unlock();
	return Ret;
}