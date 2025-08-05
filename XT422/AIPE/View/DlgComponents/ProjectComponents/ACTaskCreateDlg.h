#pragma once

#include "AngKDialog.h"
#include "ACTaskManager.h"
#include <QButtonGroup>
#include <QStandardItemModel>
namespace Ui { class ACTaskCreateDlg; };

class ACTaskCreateDlg : public AngKDialog
{
	Q_OBJECT

	enum class ChooseType
	{
		Select_All = 0,	//全选
		Select_None,	//全不选
		Select_Invert	//反选
	};

	enum class BindProgType
	{
		Bind_Prog_Alias = 0,	//绑定编程器别名
		Bind_Prog_SN,			//绑定编程器SN
		Bind_ALL_Prog			//绑定所有编程器
	};
public:
	ACTaskCreateDlg(QWidget *parent, std::shared_ptr<ACTaskManager> _pTaskMgr);
	~ACTaskCreateDlg();

	void InitText();

	void InitButton();

	void InitAdapterButton();

	void InitComboBox();

	void InitBindGroup();

	void InitTaskTable();

	void SetDevBindInfo(QString devBind);

	void SetAdapterIndex(QString adapterIdx);

	QString GetDevBindInfo();

	QString GetAdapterIndex();

	void SetTaskInfo(std::vector<TaskProperty>& _taskVec);

	void CreateMasterAnalyzeTask();

protected:
	virtual bool eventFilter(QObject* watched, QEvent* event);

	void AddTaskTableRow(QString _projPath, QString _bindInfo, QString _doubleCheck, QString _adpValue);

	void ResetTaskPath();

	void SetTaskPath();
public slots:
	void onSlotOpenTaskPath();
	void onSlotSaveTaskPath();
	void onSlotBindProject();
	void onSlotSwitchAdapterShow(bool);
	void onSlotBindTask2Proj();
	void onSlotSwitchSelect(int);
	void onSlotSwitchBindSelect(int);
	void onSlotAddTaskTableRow();
	void onSlotDeleteTableRow();
	void onSlotSetProjectBind(const QModelIndex&);
	void onSlotEditProj(bool checked = false);
private:
	Ui::ACTaskCreateDlg *ui;
	std::shared_ptr<ACTaskManager> m_pTaskManager;
	std::unique_ptr<QStandardItemModel> m_pTaskTableModel;
	std::unique_ptr<QPushButton> m_pAddTableButton;
	QVector<QPushButton*>	m_vecBPUButton;
	QButtonGroup* m_pButtonGroup;
	QString	m_strRecordTaskPath;
	QString	m_strRecordProjPath;
	QModelIndex	m_curModelIdx;
};
