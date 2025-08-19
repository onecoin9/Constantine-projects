#pragma once

#include "ACProjManager.h"
#include <QObject>
#include <QStandardItemModel>

struct TaskProperty
{
	std::string taskPath;
	std::string devBindInfo;
	std::string doubleCheck;
	std::string adpValue;
	std::string crc16Str;
};

typedef QPair<QString, ACProjManager*> projAdp;

class ACTaskManager : public QObject
{
	Q_OBJECT

public:
	ACTaskManager(QObject *parent);
	~ACTaskManager();
	int CreateTask(std::unique_ptr<QStandardItemModel>& _taskTable, QString _saveTaskPath);

	int OpenTask(QString _taskPath, std::vector<TaskProperty>& _vecProp);

	int LoadTask(QString _taskPath, std::map<QString, int>& _DevResult);

	projAdp& GetProjInfo(QString _projName);

	QMap<QString, QPair<QString, ACProjManager*>>& GetAllProjInfo();

	QString GetTaskPath() { return m_curOpenTask; }

	QString calculateFileCRC16(const QString& fileName);

signals:
	void sgnErrorMessage(QString title, QString msg);
private:
	QMap<QString, QPair<QString, ACProjManager*>> m_mapBindProjMgr;
	QString m_curOpenTask;
};
