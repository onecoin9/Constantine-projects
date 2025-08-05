#include "ACTaskDownload.h"
#include "StyleInit.h"
#include "ui_ACTaskDownload.h"
#include "ACDeviceManager.h"
#include "AngKPathResolve.h"
#include "AngKGlobalInstance.h"
#include "ACMessageBox.h"
#include "ACReportRecord.h"
#include "Thread/ThreadPool.h"
#include <QEventLoop>
#include <QFileDialog>
#include <QToolTip>
#include "AngKCheckBoxHeader.h"
#include "AngKProgressDelegate.h"
#include "../RemoteServer/JsonRpcServer.h"
#include "../TaskManager/TaskManagerSingleton.h"
#include "AngKMessageHandler.h"
#include "MT422SerialHandler.h"
extern Acro::Thread::ThreadPool g_ThreadPool;

#define SUCCESS "#20dfb2"
#define FAILED "#ff6d8a"
#define NORMAL "#b3d9ff"

ACTaskDownload::ACTaskDownload(QWidget *parent, std::shared_ptr<ACTaskManager> _pTaskMgr)
	: AngKDialog(parent)
	, ui(new Ui::ACTaskDownload())
	, m_pDevTableModel(std::make_unique<QStandardItemModel>(ACDeviceManager::instance().getAllDevInfo().size(), 6, ui->devTable))
	, m_strRecordPath(AngKGlobalInstance::ReadValue("TaskProjectPath", "taskPath").toString())
	, m_pTaskManager(_pTaskMgr)
	, m_nCurDownloadingNum(0)
{
	this->setObjectName("ACTaskDownload");
	QT_SET_STYLE_SHEET(objectName());

	ui->setupUi(setCentralWidget());

	this->setFixedSize(900, 400);
	this->SetTitle(tr("Task Download"));

	InitText();
	InitTable();
	InitButton();
	InitDevInfo();
	TaskManagerSingleton::getInstance().setTaskManager(m_pTaskManager.get());
	//默认不开启， 开始校验时开启
	ui->progressText->setText(tr("Progress : "));

	connect(_pTaskMgr.get(), &ACTaskManager::sgnErrorMessage, this, [this](QString title, QString msg) {
		ACMessageBox::showError(this, title, msg);
		return;
		}, Qt::UniqueConnection);

	connect(this, &ACTaskDownload::sgnStartDownloadProject, this, &ACTaskDownload::onSlotStartDownloadProject);
	connect(this, &ACTaskDownload::sgnStopDownloadProject, this, &ACTaskDownload::onSlotStopDownloadProject);
	connect(this, &ACTaskDownload::SendNotification, JsonRpcServer::Instance(), &JsonRpcServer::SendNotification);
}

ACTaskDownload::~ACTaskDownload()
{
	delete ui;
}

void ACTaskDownload::InitText()
{
	ui->taskPathText->setText(tr("Task Path:"));
	ui->downloadButton->setText(tr("Download"));
	ui->taskPathEdit->setReadOnly(true);
}

void ACTaskDownload::InitTable()
{
	// 隐藏水平表头
	ui->devTable->verticalHeader()->setVisible(false);
	ui->devTable->setMouseTracking(true);
	connect(ui->devTable, &AngKTableView::entered, this, [=](QModelIndex modelIdx) {
		if (!modelIdx.isValid()) {
			return;

		}
		QToolTip::showText(QCursor::pos(), modelIdx.data().toString());

		});

	//QStringList headList;
	//headList << tr("IP Address") << tr("HopNum") << tr("SiteSN") << tr("SiteAlias") << tr("Status");

	//m_pDevTableModel->setHorizontalHeaderLabels(headList);
	m_pDevTableModel->setHeaderData(0, Qt::Horizontal, "");
	m_pDevTableModel->setHeaderData(1, Qt::Horizontal, tr("HopNum"));
	m_pDevTableModel->setHeaderData(2, Qt::Horizontal, tr("SiteSN"));
	m_pDevTableModel->setHeaderData(3, Qt::Horizontal, tr("SiteAlias"));
	m_pDevTableModel->setHeaderData(4, Qt::Horizontal, tr("Status"));
	m_pDevTableModel->setHeaderData(5, Qt::Horizontal, tr("Progress"));
	AngKCheckBoxHeader* header = new AngKCheckBoxHeader(Qt::Horizontal, ui->devTable);
	header->setCheckBoxModel(m_pDevTableModel.get(), 0, tr("IP Address"));
	ui->devTable->setHorizontalHeader(header);

	AngKProgressDelegate* progressDelegate = new AngKProgressDelegate();
	progressDelegate->setProgressIndex(5);

	ui->devTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui->devTable->setItemDelegateForColumn(5, progressDelegate);
	ui->devTable->setModel(m_pDevTableModel.get());
	ui->devTable->setAlternatingRowColors(true);
	ui->devTable->horizontalHeader()->setHighlightSections(false);
	ui->devTable->horizontalHeader()->setStretchLastSection(true);
	ui->devTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->devTable->horizontalHeader()->setMinimumSectionSize(100);

	QHeaderView* manuHead = ui->devTable->horizontalHeader();

	manuHead->setSectionResizeMode(QHeaderView::Custom);

	ui->devTable->setColumnWidth(0, 140);
	ui->devTable->setColumnWidth(1, 75);
	ui->devTable->setColumnWidth(2, 200);
	ui->devTable->setColumnWidth(3, 95);
	ui->devTable->setColumnWidth(4, 70);
	ui->devTable->setColumnWidth(5, 150);
}

void ACTaskDownload::InitButton()
{
	connect(ui->downloadButton, &QPushButton::clicked, this, &ACTaskDownload::onSlotDownloadTask);
	connect(ui->taskPathButton, &QPushButton::clicked, this, &ACTaskDownload::onSlotSelectTaskPath);
	connect(this, &ACTaskDownload::sgnClose, this, [=]() {
		if (m_nCurDownloadingNum > 0) {
			ACMessageBox::showWarning(this, tr("Warning"), tr("Please wait for the download task to complete."));
			return;
		}
		accept();
			});

	ui->taskProgressBar->setValue(0);
}

void ACTaskDownload::InitDevInfo()
{
	QVector<DeviceStu> devVec = ACDeviceManager::instance().getAllDevInfo();
	for (int i = 0; i < devVec.size(); ++i) {
		m_pDevTableModel->setData(m_pDevTableModel->index(i, 0), QString::fromStdString(devVec[i].strIP));
		m_pDevTableModel->item(i, 0)->setCheckable(true);

		m_pDevTableModel->setData(m_pDevTableModel->index(i, 1), QString::number(devVec[i].nHopNum));
		m_pDevTableModel->setData(m_pDevTableModel->index(i, 2), QString::fromStdString(devVec[i].tMainBoardInfo.strHardwareSN));
		m_pDevTableModel->setData(m_pDevTableModel->index(i, 3), QString::fromStdString(devVec[i].strSiteAlias));
		m_pDevTableModel->setData(m_pDevTableModel->index(i, 4), tr("Idle"));
	}

	AngKProgressDelegate* ProgressDelegate = qobject_cast<AngKProgressDelegate*>(ui->devTable->itemDelegateForColumn(5));
	ProgressDelegate->UpdateProgressColor(NORMAL);

	ChangeTableColor(TaskProgressState::Normal);
}

void ACTaskDownload::onSlotSelectTaskPath()
{
	QString sfilePath = QFileDialog::getOpenFileName(this, "Select Task File...", m_strRecordPath, tr("Task File(*.actask)"));
	if (!sfilePath.isEmpty()) {
		m_strRecordPath = sfilePath;
		ui->taskPathEdit->setText(sfilePath);
	}
}
void ACTaskDownload::onSlotSelectTaskPathFromJsonRpc(QString sfilePath)
{
		m_strRecordPath = sfilePath;
		ui->taskPathEdit->setText(sfilePath);
}
void ACTaskDownload::onSlotDownloadProgress(int nValue, QString strIP, int nHop)
{
	int downloadValue = 50 + nValue * 0.5;
	for (int i = 0; i < m_pDevTableModel->rowCount(); ++i) {
		if (m_pDevTableModel->item(i, 0)->checkState() == Qt::CheckState::Checked
			&& m_pDevTableModel->data(m_pDevTableModel->index(i, 0)).toString() == strIP
			&& m_pDevTableModel->data(m_pDevTableModel->index(i, 1)).toInt() == nHop) {

			if (downloadValue < 50) {
				m_pDevTableModel->setData(m_pDevTableModel->index(i, 4), tr("Failed"));
				//失败进度也是100，表示执行完成，但是失败
				m_pDevTableModel->setData(m_pDevTableModel->index(i, 5), 100, Qt::UserRole + eIV_Value);
				m_pDevTableModel->setData(m_pDevTableModel->index(i, 5), tr("Failed"), Qt::UserRole + eIV_ProgressType);
				if (JsonRpcServer::Instance()->RUNING_STATUS) {
					testbool = false;
					nlohmann::json result;
					result["result"] = testbool;
					
					nlohmann::json data;
					data["strIp"] = strIP.toStdString();
					data["nHop"] = nHop;
					result["data"] = data;
					emit SendNotification("setLoadProjectResult", result);
					if((i + 2) == m_pDevTableModel->rowCount())
					this->close();
				}
				JudgeTaskFinish();
				break;
			}
			else if (downloadValue == 100) {
				m_pDevTableModel->setData(m_pDevTableModel->index(i, 4), tr("Success"));
				m_pDevTableModel->setData(m_pDevTableModel->index(i, 5), tr("Success"), Qt::UserRole + eIV_ProgressType);
				//下面仅作xt422项目在下载项目完毕之后，发送从422com.json中读取的波特率，发送给下位机，现已弃用
				/*connect(this, &ACTaskDownload::sgnTaskDownLoadStatus2, &MT422SerialHandler::instance(), &MT422SerialHandler::slotTaskDownLoadStatus2, Qt::QueuedConnection);
				emit sgnTaskDownLoadStatus2(strIP.toStdString(), 0);*/
				ALOG_INFO("JsonRpcServer.RUNING_STATUS:%d", "CU", "--", JsonRpcServer::Instance()->RUNING_STATUS);
				if (JsonRpcServer::Instance()->RUNING_STATUS) {
					testbool = true;
					nlohmann::json result;
					result["result"] = testbool;
					nlohmann::json data;
					data["strIp"] = strIP.toStdString();
					data["nHop"] = nHop;
					result["data"] = data;
					emit SendNotification("setLoadProjectResult", result);
					if ((i + 2) == m_pDevTableModel->rowCount())
					this->close();
				}
			}

			m_pDevTableModel->setData(m_pDevTableModel->index(i, 5), downloadValue, Qt::UserRole + eIV_Value);
			
			if (downloadValue < 50 || downloadValue >= 100) {
				JudgeTaskFinish();
			}

		}
	}

	CalAverProgress();
}
void ACTaskDownload::setModelTableSeleted() {
	for (int i = 0; i < m_pDevTableModel->rowCount(); ++i) {
		m_pDevTableModel->item(i, 0)->setCheckState(Qt::CheckState::Checked);
	}
}
void ACTaskDownload::onSlotStartDownloadProject()
{
	for (int i = 0; i < m_pDevTableModel->rowCount(); ++i) {
		if (m_pDevTableModel->item(i, 0)->checkState() == Qt::CheckState::Checked
			&& m_pDevTableModel->data(m_pDevTableModel->index(i, 4)).toString() != tr("Failed")) {
			QString strDevIP = m_pDevTableModel->data(m_pDevTableModel->index(i, 0)).toString();
			int nHop = m_pDevTableModel->data(m_pDevTableModel->index(i, 1)).toInt();

			g_ThreadPool.PushTask([this](QString& strDevIP, int nHop, QString& strTaskPath) {

				emit sgnDownloadProject(strDevIP, nHop, strTaskPath);
				}, std::move(strDevIP), nHop, ui->taskPathEdit->text());
		}


		// 未下发task，但是之前下发过task的设备，需要判断已下发task与当前task是否一致，如果不一致，则将其视为未下发任务，不允许烧录
		QString strDevIP = m_pDevTableModel->data(m_pDevTableModel->index(i, 0)).toString();
		int nHop = m_pDevTableModel->data(m_pDevTableModel->index(i, 1)).toInt();
		DeviceStu devStu = ACDeviceManager::instance().getDevInfo(strDevIP, nHop);
		if (m_pDevTableModel->item(i, 0)->checkState() != Qt::CheckState::Checked
			&& (devStu.ProgEnvReady == ProEnvStatus::Success || devStu.ProgEnvReady == ProEnvStatus::SuccessButCurTskChanged)){
			g_ThreadPool.PushTask([this](QString& strDevIP, int nHop, QString& strTaskPath) {
				emit sgnCheckDevVFiles(strDevIP, nHop, strTaskPath);
			}, std::move(strDevIP), nHop, ui->taskPathEdit->text());
		}

	}
	emit sgnUpdateTaskInfo(); 
}

void ACTaskDownload::onSlotUpdateChkProgress(double dProgress)
{
	//ui->taskProgressBar->setValue(dProgress);
	for (int i = 0; i < m_pDevTableModel->rowCount(); ++i) {
		if (m_pDevTableModel->item(i, 0)->checkState() == Qt::CheckState::Checked) {
			m_pDevTableModel->setData(m_pDevTableModel->index(i, 5), dProgress, Qt::UserRole + eIV_Value);
		}
	}

	CalAverProgress();
}

void ACTaskDownload::onSlotStopDownloadProject(int nResultCode)
{
	PrintMessage(nResultCode);

	ui->downloadButton->setEnabled(true);
	m_nCurDownloadingNum = 0;
	for (int i = 0; i < m_pDevTableModel->rowCount(); ++i) {
		if (m_pDevTableModel->item(i, 0)->checkState() == Qt::CheckState::Checked)
		{
			m_pDevTableModel->setData(m_pDevTableModel->index(i, 4), tr("Idle"));
		}
	}
}

void ACTaskDownload::onSlotDownloadTask()
{
	//QEventLoop loop;
	m_DownloadIPHop.clear();
	nSelectDev = 0;
	ui->taskProgressBar->setValue(0);
	m_nCurDownloadingNum = 0;
	AngKProgressDelegate* ProgressDelegate = qobject_cast<AngKProgressDelegate*>(ui->devTable->itemDelegateForColumn(5));
	ProgressDelegate->UpdateProgressColor(NORMAL);
	ChangeTableColor(TaskProgressState::Normal);

	if (ui->taskPathEdit->text().isEmpty()) {
		ACMessageBox::showWarning(this, QObject::tr("Warning"), QObject::tr("Please select a task file."));
		return;
	}

	for (int i = 0; i < m_pDevTableModel->rowCount(); ++i) {
		if (m_pDevTableModel->item(i, 0)->checkState() == Qt::CheckState::Checked) {
			nSelectDev++;
		}
	}
	if (nSelectDev == 0) {
		ACMessageBox::showWarning(this, QObject::tr("Warning"), QObject::tr("Please select at least one device."));
		return;
	}


	//当前默认只添加一个工程
	QString projName;
	ACReportRecord& GReporter = GetReporter();
	for (auto& projIter : m_pTaskManager->GetAllProjInfo().toStdMap()) {
		projName = projIter.first;
		//GReporter.SetProjName(projName);
	}

	for (int i = 0; i < m_pDevTableModel->rowCount(); ++i) {
		if (!projName.isEmpty()) {
			QString strDevIP = m_pDevTableModel->data(m_pDevTableModel->index(i, 0)).toString();
			int nHop = m_pDevTableModel->data(m_pDevTableModel->index(i, 1)).toInt();
			DeviceStu devSu = ACDeviceManager::instance().getDevInfo(strDevIP, nHop);
			GReporter.AddSite(projName, &devSu, 8);
		}
	}
	ACHtmlLogWriter ReportWriter;
	if (!m_pTaskManager->GetTaskPath().isEmpty()/* && m_pTaskManager->GetTaskPath() == ui->taskPathEdit->text()*/) {
		QFileInfo fInfo(m_pTaskManager->GetTaskPath());
		QString strReportPath = Utils::AngKCommonTools::CreateReportFile(fInfo.baseName());
		if (ReportWriter.CreateLog(strReportPath, ACHtmlLogWriter::LOGTYPE_REPORT)) {

			if (GReporter.WriteReportToAloneFile(&ReportWriter) == ACERR_OK) {
				ALOG_INFO("Report Task File : %s successfully", "CU", "--", strReportPath.toStdString().c_str());
			}
			else {
				ALOG_ERROR("Report Task File : %s failed", "CU", "--", strReportPath.toStdString().c_str());
			}
		}
	}

	GReporter.ReInit();
	m_pTaskManager->GetAllProjInfo().clear();
	ui->taskProgressBar->setValue(0);

	QString strTaskPath = ui->taskPathEdit->text();
	std::map<QString, int> m_mapDevReady;
	for (int i = 0; i < m_pDevTableModel->rowCount(); ++i) {
		if (m_pDevTableModel->item(i, 0)->checkState() == Qt::CheckState::Checked) {
			QString strDevIP = m_pDevTableModel->data(m_pDevTableModel->index(i, 0)).toString();
			int nHop = m_pDevTableModel->data(m_pDevTableModel->index(i, 1)).toInt();
			QString strDevIPHop = strDevIP + ":" + QString::number(nHop);
			m_mapDevReady[strDevIPHop] = 0;
		}
	}

	//加载Task
	int retLoad = m_pTaskManager->LoadTask(ui->taskPathEdit->text(), m_mapDevReady);

	if (retLoad != 0) {
		PrintMessage(retLoad);
		return;
	}

	for (auto& iterDev : m_mapDevReady) {
		QStringList strIPHopList = iterDev.first.split(":");
		DeviceStu tmpDev = ACDeviceManager::instance().getDevInfo(strIPHopList[0], strIPHopList[1].toInt());

		for (int i = 0; i < m_pDevTableModel->rowCount(); ++i) {
			QString strDevIP = m_pDevTableModel->data(m_pDevTableModel->index(i, 0)).toString();
			int nHop = m_pDevTableModel->data(m_pDevTableModel->index(i, 1)).toInt();
			if (strDevIP == strIPHopList[0] && nHop == strIPHopList[1].toInt()) {
				if (retLoad != 0 || iterDev.second != 0) {
					tmpDev.ProgEnvReady = ProEnvStatus::Failed;
					PrintMessage(retLoad);

					ACDeviceManager::instance().setDevInfo(strIPHopList[0], strIPHopList[1].toInt(), tmpDev);
					m_pDevTableModel->setData(m_pDevTableModel->index(i, 4), tr("Failed"));
					//ChangeTableColor(strDevIP.toStdString(), nHop, QColor(FAILED));
					AngKProgressDelegate* ProgressDelegate = qobject_cast<AngKProgressDelegate*>(ui->devTable->itemDelegateForColumn(5));
					ProgressDelegate->UpdateProgressColor(NORMAL);
					m_pDevTableModel->setData(m_pDevTableModel->index(i, 5), 100, Qt::UserRole + eIV_Value);
					m_pDevTableModel->setData(m_pDevTableModel->index(i, 5), tr("Failed"), Qt::UserRole + eIV_ProgressType);
					continue;
				}

				ACDeviceManager::instance().setDevInfo(strIPHopList[0], strIPHopList[1].toInt(), tmpDev);
				m_DownloadIPHop.push_back(iterDev.first);
				m_pDevTableModel->setData(m_pDevTableModel->index(i, 4), tr("Loading"));

				m_pDevTableModel->setData(m_pDevTableModel->index(i, 5), 0, Qt::UserRole + eIV_Value);
				m_pDevTableModel->setData(m_pDevTableModel->index(i, 5), tr("Loading"), Qt::UserRole + eIV_ProgressType);
				m_nCurDownloadingNum++;
			}
		}
	}

	if (m_nCurDownloadingNum > 0) {
		SetDownloadStatus(true);

		g_ThreadPool.PushTask([this]() {
			QMap<QString, QPair<QString, ACProjManager*>> allProjInfo = m_pTaskManager->GetAllProjInfo();
			int ret = 0;
			for (auto& projInfo : allProjInfo.toStdMap()) {
				//先校验文件内容是否可靠
				connect(projInfo.second.second, &ACProjManager::sgnUpdateChkProgress, this, &ACTaskDownload::onSlotUpdateChkProgress);
				ret = projInfo.second.second->CheckAllFileChkSum();
				disconnect(projInfo.second.second, &ACProjManager::sgnUpdateChkProgress, this, &ACTaskDownload::onSlotUpdateChkProgress);
			}

			if (ret == 0) {
				emit sgnStartDownloadProject();
			}
			else {
				emit sgnStopDownloadProject(ret);
			}
			});
	}
}

void ACTaskDownload::SetDownloadStatus(bool status) {
	if (status) {
		ui->downloadButton->setEnabled(false);
	}
	else {
		ui->downloadButton->setEnabled(true);
		RequestSKTInfo();
	}
}

void ACTaskDownload::RequestSKTInfo()
{
	for (int i = 0; i < m_pDevTableModel->rowCount(); ++i) {
		if (m_pDevTableModel->item(i, 0)->checkState() == Qt::CheckState::Checked) {
			QString strIP = m_pDevTableModel->data(m_pDevTableModel->index(i, 0)).toString();
			int nHop = m_pDevTableModel->data(m_pDevTableModel->index(i, 1)).toInt();

			emit sgnRequestSKTInfo(strIP.toStdString(), nHop);
		}
	}
}

void ACTaskDownload::PrintMessage(int nResultCode)
{
	AngKProjFile::PrintMessage(this, nResultCode, tr("Load Task Error"));
	//switch (FORMATFILE_RET(nResultCode))
	//{
	//case E_FMTFILE_OK:
	//	break;
	//case E_TAG_NOTFOND:
	//case E_TAG_UNEOUGH:
	//case E_TAG_EXIST:
	//	ACMessageBox::showError(this, tr("Load Task Error"), tr("Import project failed."));
	//	break;
	//case E_FMTFILE_ERRER:
	//	ACMessageBox::showError(this, tr("Load Task Error"), tr("Load project file error, please check if the file is valid."));
	//	break;
	//case E_TAG_CHECKSUM:
	//	ACMessageBox::showError(this, tr("Load Task Error"), tr("Check the task file failed, please check if the file is valid."));
	//	break;
	//case E_PWD_ERROR:
	//	ACMessageBox::showError(this, tr("Load Task Error"), tr("Password error."));
	//	break;
	//case E_PWD_CANCEL:
	//	ACMessageBox::showError(this, tr("Load Task Error"), tr("Password not entered."));
	//	break;
	//default:
	//	break;
	//}
}

void ACTaskDownload::ChangeTableColor(TaskProgressState _taskProgress)
{
	QString styleSheet;
	switch (_taskProgress)
	{
	case TaskProgressState::Normal:
		styleSheet = QString("QProgressBar::chunk { background-color: %1; }").arg(NORMAL);
		break;
	case TaskProgressState::Success:
		styleSheet = QString("QProgressBar::chunk { background-color: qlineargradient(spread:pad,x1:0,y1:0,x2:1,y2:0,stop:0 rgba(14, 186, 220, 1),stop:0.5 rgba(29, 219, 181, 1), stop:1 rgba(40, 241, 157, 1)); }");
		break;
	case TaskProgressState::Failed:
		styleSheet = QString("QProgressBar::chunk { background-color: qlineargradient(spread:pad,x1:0,y1:0,x2:1,y2:0,stop:0 rgba(255, 143, 138, 1),stop:1 rgba(255, 109, 136, 1)); }");
		break;
	case TaskProgressState::Mix:
		styleSheet = QString("QProgressBar::chunk { background-color: qlineargradient(spread:pad,x1:0,y1:0,x2:1,y2:0,stop:0 rgba(248,215,110, 1),stop:0.5 rgba(245,207,79, 1), stop:1 rgba(244,201,61, 1)); }");
		break;
	default:
		break;
	}

	ui->taskProgressBar->setStyleSheet(styleSheet);
}

void ACTaskDownload::CalAverProgress()
{
	int nDevCount = 0;
	int nAllDevProgress = 0;
	for (int i = 0; i < m_pDevTableModel->rowCount(); ++i) {
		if (m_pDevTableModel->item(i, 0)->checkState() == Qt::CheckState::Checked) {
			nAllDevProgress += m_pDevTableModel->data(m_pDevTableModel->index(i, 5), Qt::UserRole + eIV_Value).toInt();
			nDevCount++;
		}
	}

	if (nDevCount != 0) {
		float allValue = nAllDevProgress * 1.0 / nDevCount;
		ui->taskProgressBar->setValue(allValue);
	}
	else {
		ui->taskProgressBar->setValue(0);
	}
}

void ACTaskDownload::UpdateTaskDownloadState()
{
	bool hasSuccess = false;
	bool hasFailed = false;

	for (int i = 0; i < m_pDevTableModel->rowCount(); ++i) {
		if (m_pDevTableModel->item(i, 0)->checkState() == Qt::CheckState::Checked) {
			if (m_pDevTableModel->data(m_pDevTableModel->index(i, 4)).toString() == tr("Failed")) {
				hasFailed = true;
			}
			else if (m_pDevTableModel->data(m_pDevTableModel->index(i, 4)).toString() == tr("Success")) {
				hasSuccess = true;

			}
		}

		if (hasSuccess && hasFailed) {
			ChangeTableColor(TaskProgressState::Mix);
			return;
		}
	}

	if (hasSuccess) {
		ChangeTableColor(TaskProgressState::Success);
	}
	else if(hasFailed){
		ChangeTableColor(TaskProgressState::Failed);
	}
}

void ACTaskDownload::JudgeTaskFinish()
{
	m_nCurDownloadingNum--;
	if (m_nCurDownloadingNum == 0) {
		SetDownloadStatus(false);
		UpdateTaskDownloadState();
	}
}
