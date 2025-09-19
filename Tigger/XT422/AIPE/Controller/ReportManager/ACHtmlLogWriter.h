#pragma once

#include <QObject>
#include <QFile>
#include <QMutex>

class ACHtmlLogWriter : public QFile
{
	Q_OBJECT

public:
	enum {
		LOGTYPE_NORMAL,
		LOGTYPE_REPORT,
	};
	ACHtmlLogWriter(QObject *parent = nullptr);
	~ACHtmlLogWriter();

	bool CreateLog(QString strLogFile, int LogType = LOGTYPE_NORMAL);
	bool CloseLog();
	bool InsertLog(int LogLevel, const char* fmt, ...);

	bool ConfigDumpBegin();
	bool InsertConfigLog(QString strInfo);
	bool ConfigDumpEnd();
	bool WriteLog(QString& strLog);

	bool SktSimpleDumpBegin();
	bool InsertSktSimpleInfo(QString strInfo);
	bool SktSimpleDumpEnd();

private:
	QString GetLogTemplet();
	QString GetReportTemplet();

	void AppendEndTag();

	bool m_bTimeHead;
	QMutex m_Mutex;
	int m_nEndPos;
};
