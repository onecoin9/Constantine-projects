#include "AngKLogManager.h"
#include "AngKGlobalInstance.h"
#include "AngKPathResolve.h"
#include "AngKMisc.h"
#include "MessageNotify/NotifyManager.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>


AngKLogManager::AngKLogManager(QObject* object)
	: QObject(object)
	, m_mutex(QMutex::Recursive)
	, m_buffer(new QBuffer())
	, m_bStop(false)
{
	KeepTimeLogFile();

	connect(Utils::AngkLogger::logger(), &Utils::AngkLogger::sgnLogMsg2FileBuffer, this, &AngKLogManager::onSlotLogMsg2FileBuffer, Qt::DirectConnection);

	
	
	m_logFile.open(Utils::AngkLogger::logger()->logFileName().toUtf8().constData(), std::ios::out | std::ios::app);

	moveToThread(&logThread);

	connect(&logThread, &QThread::started, this, &AngKLogManager::doWork);

	logThread.start();
}

AngKLogManager::~AngKLogManager()
{
	setStop(true);
	if (m_buffer)
	{
		m_buffer = nullptr;
		delete m_buffer;
	}
	m_logFile.close();

	logThread.quit();
	logThread.wait();
}

void AngKLogManager::setBufferConnection() {
	connect(m_buffer, SIGNAL(readyRead()), this, SLOT(onBuffer_readRead()));
}

AngKLogManager* AngKLogManager::instancePtr() {
	static AngKLogManager logManager;
	return &logManager;
}

void AngKLogManager::doWork()
{
	while (!m_bStop)
	{
		if (!m_logMsgQueue.isEmpty()) {
			QMutexLocker locker(&m_mutex);
			QString endMsg = m_logMsgQueue.dequeue();
			saveToLogFile(endMsg);
			addToBuffer(endMsg);
		}
		else {
			QThread::msleep(50);
		}
	}
}

void AngKLogManager::setStop(bool _stop)
{
	m_bStop = _stop;
}


void AngKLogManager::saveToLogFile(const QString& strMsg)
{
	QMutexLocker locker(&m_mutex);

	UpdateLogFile();

	//std::ofstream logFile;
	//m_logFile << Utils::AngkLogger::logger()->msg2LogMsg(strMsg).toUtf8().constData() << std::endl;
	m_logFile << strMsg.toUtf8().constData() << std::endl;
	m_logFile.flush();
}

void AngKLogManager::addToBuffer(const QString& strMsg)
{
	QMutexLocker locker(&m_mutex);

	if (m_buffer != nullptr) {
		m_buffer->open(QIODevice::Append);
		//m_buffer->write(Utils::AngkLogger::logger()->msg2LogMsg(strMsg).toUtf8());
		QString copyMsg = strMsg + "\n";
		m_buffer->write(copyMsg.toUtf8());
		emit m_buffer->readyRead();
		m_buffer->close();
	}
}

void AngKLogManager::UpdateLogFile()
{
	QFileInfo info(Utils::AngkLogger::logger()->logFileName());

	int64_t infoSize = info.size();
	int64_t logSize = AngKGlobalInstance::ReadValue("LogFile", "size").toLongLong() * 1024;

	if (infoSize >= logSize) {
		m_logFile.close();

		Utils::AngKPathResolve::ReCreateLogFile();
		m_logFile.open(Utils::AngkLogger::logger()->logFileName().toUtf8().constData(), std::ios::out | std::ios::app);
	}
}

void AngKLogManager::KeepTimeLogFile()
{
	int logKeepTime = AngKGlobalInstance::ReadValue("LogFile", "keepTime").toInt() * 24 * 60 * 60;

	if (logKeepTime == 0)
		return;

	QString logPath = Utils::AngKPathResolve::logPath();

	QDir directory(logPath);
	QFileInfoList fileList = directory.entryInfoList(QDir::Files); // 获取目录下所有文件的信息列表

	foreach(QFileInfo fileInfo, fileList) {
		QString fileName = fileInfo.fileName();
		QDateTime createdTime = fileInfo.created();
		uint createTime = createdTime.toTime_t();
		if (QDateTime::currentDateTime().toTime_t() - createTime > logKeepTime)
		{
			Utils::AngKMisc::deleteFile(fileName);
		}
	}


}

void AngKLogManager::onBuffer_readRead()
{
	QMutexLocker locker(&m_mutex);
	
	m_buffer->open(QIODevice::ReadOnly);
	QByteArray data = m_buffer->readAll();
	emit readyWrite(QString::fromUtf8(data.data()));
	m_buffer->close();
	m_buffer->setBuffer(NULL);
}

void AngKLogManager::onSlotLogMsg2FileBuffer(QString test)
{
	QMutexLocker locker(&m_mutex);
	m_logMsgQueue.push_back(test);
}