#pragma once

#include <QBuffer>
#include <QMutex>
#include <QThread>
#include <QQueue>
#include <fstream>
#include "AngkLogger.h"

class AngKLogManager : public QObject
{
	Q_OBJECT

public:
	static AngKLogManager* instancePtr();
	void doWork();
	void setStop(bool _stop);

	void setBufferConnection();
private:
	explicit AngKLogManager(QObject* object = nullptr);
	~AngKLogManager();
	void saveToLogFile(const QString& strMsg);
	void addToBuffer(const QString& strMsg);
	void UpdateLogFile();
	void KeepTimeLogFile();
Q_SIGNALS:
	void readyWrite(QString);
private Q_SLOTS:
	void onBuffer_readRead();
public slots:
	void onSlotLogMsg2FileBuffer(QString);

private:
	QMutex m_mutex;
	QBuffer* m_buffer;
	std::ofstream m_logFile;
	QQueue<QString> m_logMsgQueue;
	bool m_bStop;

	QThread logThread;
};