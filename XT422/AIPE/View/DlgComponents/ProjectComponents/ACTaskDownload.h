#pragma once

#include "AngKDialog.h"
#include "ACTaskManager.h"
#include <QStandardItemModel>
namespace Ui { class ACTaskDownload; };

enum class TaskProgressState
{
	Normal = 0,
	Success,
	Failed,
	Mix
};

class ACTaskDownload : public AngKDialog
{
	Q_OBJECT

public:
	ACTaskDownload(QWidget *parent, std::shared_ptr<ACTaskManager> _pTaskMgr);
	~ACTaskDownload();

	void InitText();

	void InitTable();

	void InitButton();

	void InitDevInfo();

	void SetDownloadStatus(bool status);

	void RequestSKTInfo();

	void PrintMessage(int nResultCode);

	void ChangeTableColor(TaskProgressState _taskProgress);

	void CalAverProgress();

	void UpdateTaskDownloadState();

	void JudgeTaskFinish();

	void onSlotSelectTaskPathFromJsonRpc(QString sfilePath);

	void setModelTableSeleted();
	bool testbool = false;
signals:
	void sgnTestLoop(int);
	void sgnDownloadProject(QString, int, QString);
	void sgnStartDownloadProject();
	void sgnUpdateTaskInfo();
	void sgnRequestSKTInfo(std::string, int);
	void sgnStopDownloadProject(int);
	void SendNotification(const QString& method, const nlohmann::json& result);
	void sgnTaskDownLoadStatus2(std::string devIP, uint32_t HopNum);
	void sgnCheckDevVFiles(QString, int, QString);

public slots:
	void onSlotDownloadTask();

	void onSlotSelectTaskPath();

	

	void onSlotDownloadProgress(int, QString, int);

	void onSlotStartDownloadProject();

	void onSlotUpdateChkProgress(double);

	void onSlotStopDownloadProject(int);

private:
	Ui::ACTaskDownload* ui;
	std::unique_ptr<QStandardItemModel> m_pDevTableModel;
	std::shared_ptr<ACTaskManager>		m_pTaskManager;
	

	std::vector<QString>	m_DownloadIPHop;
	int	nSelectDev;
	QString	m_strRecordPath;
	int m_nCurDownloadingNum;
};
