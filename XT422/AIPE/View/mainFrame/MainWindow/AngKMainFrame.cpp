#include "AngKMainFrame.h"
#include "ui_AngKMainFrame.h"
#include "../View/GlobalInit/StyleInit.h"
#include "AngKShowLogArea.h"
#include "AngKCommonTools.h"
#include "AngKChipDialog.h"
#include "AngKProgrammerWidget.h"
#include "AngKGlobalInstance.h"
#include "AppModeType.h"
#include "AngKMessageHandler.h"
#include "AngKProjFile.h"
#include "AngKChkManager.h"
#include "AngKDataBuffer.h"
#include "AngKBufferHexEdit.h"
#include "ACEventLogger.h"
#include "AngKTransmitSignals.h"
#include "AngKLogManager.h"
#include "AngKRemoteCmdManager.h"
#include "AngKProjectOperation.h"
#include "AutoMessageServer.h"
#include "ChkSum.h"
#include "ICD.h"
#include "Tag_Proj.h"
#include "json.hpp"
#include "QInputDialog.h"
#include "CRC/Crc32_Comm.h"
#include "ACMessageBox.h"
#include "ACDeviceManager.h"
#include "ACReportRecord.h"
#include "MessageNotify/notifymanager.h"
#include "CustomMessageHandler.h"
#include "AngKUpdateFirmware.h"
#include "ACSSDProjManager.h"
#include <QLibrary>
#include <QPropertyAnimation>
#include <QSpacerItem>
#include <QScrollBar>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

#define DDRMULTIPLE 4096

int calc_crc16sum(unsigned char* buf, unsigned int size, unsigned short* pCRC16Sum);
extern bool bAutoConnect;

namespace AcroView
{
	AngKMainFrame::AngKMainFrame(QWidget* parent, AngKProjDataset* _proj, std::shared_ptr<ACTaskManager> _pTaskMgr)
		: QWidget(parent)
		, m_isExpend(false)
		, m_logWgtMinHeight(0)
		, m_logWgtMaxHeight(0)
		, m_animWgt(nullptr)
		, m_log(nullptr)
		, m_projDataset(_proj)
		, m_pDataBuffer(nullptr)
		, m_bUpdateProgress(false)
		, m_bAnalyzeProgress(false)
		, m_LinkHopNum(-1)
		, m_pRemoteCmdManager(nullptr)
		, m_pChkManager(nullptr)
		, m_pChipSelectDlg(nullptr)
		, m_curSelectSite(0)
		, m_supplyDriver(0)
		, m_bCheckSktUseInfo(false)
		, m_openProjPath(AngKGlobalInstance::ReadValue("TaskProjectPath", "projPath").toString())
		, m_pTaskManager(_pTaskMgr)
		, projSemaphore(nullptr)
		, m_nRecordDocmdTime(0)
		, m_bBurning(false)
		, m_pAutoMsgServer(std::make_unique<AutoMessageServer>(this))
		, m_pSKTResultTimer(nullptr)
		, m_bSKTTimerStarted(false)
	{
		ui = new Ui::AngKMainFrame();
		ui->setupUi(this);

		//初始化界面相关
		InitAnimWgt();
		InitControlWgt();
		InitScrollArea();
		InitPropetryWidget();
		InitMessgaeConnect();
		DevRootPartCheck();

		m_oldMargins = ui->operateWidget->layout()->contentsMargins();
		m_logWgtMinHeight = m_log->minimumHeight();
		m_logWgtMaxHeight = m_log->maximumHeight();

		//初始化IC相关设置
		InitToolWidget(ChipDataJsonSerial());

		this->setObjectName("AngKMainFrame");
		QT_SET_STYLE_SHEET(objectName());

		//临时代码，设备管理还在过渡阶段
		//将原先的设备管理设备导入到新的设备管理类中
		std::map<std::string, DeviceStu> devMap;
		AngKDeviceModel::instance().GetConnetDevMap(devMap);

		for (auto srcDev : devMap) {
			QStringList IPHopList = QString::fromStdString(srcDev.first).split(":");
			ACDeviceManager::instance().addDev(IPHopList[0], IPHopList[1].toInt(), srcDev.second);
		}

		int LicEnable = AngKGlobalInstance::instance()->ReadValue("LIC", "LicEnable", "1").toInt();
		int LicPort = AngKGlobalInstance::instance()->ReadValue("LIC", "LicServerPort", "2030").toInt();
		QString LicIP = AngKGlobalInstance::instance()->ReadValue("LIC", "LicServerIP", "127.0.0.1").toString();
		if (LicEnable == 1) {
			m_pAutoMsgServer->startServer(LicIP, LicPort);
			connect(m_pAutoMsgServer.get(), &AutoMessageServer::reportRequested, this, &AngKMainFrame::onSlotHandleReportRequested);
		}
		else {
			ALOG_INFO("Auth Service Disabled.", "CU", "--");
		}

		connect(&ACDeviceManager::instance(), &ACDeviceManager::devOffLine, this, &AngKMainFrame::onSlotDevOffLine);

		connect(&ACDeviceManager::instance(), &ACDeviceManager::devOnLine, this, &AngKMainFrame::onSlotDevOnLine);
		
		std::shared_ptr<ITimeProvider> realTime = std::make_shared<RealTimeProvider>();
		realGenerator = new MTUIDGenerator(realTime);

		connect(this, &AngKMainFrame::SendNotification, JsonRpcServer::Instance(), &JsonRpcServer::SendNotification,Qt::QueuedConnection);

		// 初始化SKT结果收集定时器
		m_pSKTResultTimer = new QTimer(this);
		m_pSKTResultTimer->setSingleShot(true);  // 单次触发
		m_pSKTResultTimer->setInterval(3000);    // 3秒
		connect(m_pSKTResultTimer, &QTimer::timeout, this, [this]() {
			  // 定时器超时处理函数
            nlohmann::json setSKTEnResult;
            if (m_sktResultList.isEmpty()) {
                // 如果结果列表为空，发送错误信息
                setSKTEnResult["status"] = -1;
                setSKTEnResult["data"] = nlohmann::json::object(); // data 为一个空的JSON对象
            }
            else {
                // 发送收集到的结果列表
                setSKTEnResult["status"] = 1;
				
                nlohmann::json dataObj;
                // 将QStringList转换为JSON数组
                dataObj["data"] = nlohmann::json::array();
                for (const QString& item : m_sktResultList) {
                    dataObj["results"].push_back(item.toStdString());
                }
                setSKTEnResult["data"] = dataObj;
            }
            emit SendNotification("setSKTEnResult",setSKTEnResult);
			// 重置状态
			m_sktResultList.clear();
			m_bSKTTimerStarted = false;
		});

	}


	AngKMainFrame::~AngKMainFrame()
	{
		//m_operList.clear();
		if (m_threadComplete.isRunning()) {
			m_threadComplete.quit();
			m_threadComplete.wait();
		}

		//析构关闭时，输出一个report
		QString projName;
		for (auto& projIter : m_pTaskManager->GetAllProjInfo().toStdMap()) {
			projName = projIter.first;
		}

		if (m_pAutoMsgServer)
		{
			int StopWhenExit = AngKGlobalInstance::instance()->ReadValue("LIC", "StopWhenExit", "1").toInt();
			m_pAutoMsgServer->stopServer(StopWhenExit);
		}

		ACReportRecord& GReporter = GetReporter();

		if (m_pTaskManager->GetTaskPath().isEmpty())
			return;

		if (!projName.isEmpty()) {

			QVector<DeviceStu> devVec = ACDeviceManager::instance().getAllDevInfo();
			for (auto& devInfo : devVec) {
				GReporter.AddSite(projName, &devInfo, 8);
			}

			for (auto obj : ui->scrollAreaWidgetContents->children()) {
				if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
					AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
					if (pro) {
						DeviceStu devInfo = pro->GetDevInfo();
						uint32_t nTotal = 0;
						uint32_t nFailed = 0;
						pro->GetBPUExeCnt(nTotal, nFailed);
						GReporter.UpdateAdapterCounter(projName, QString::fromStdString(devInfo.tMainBoardInfo.strHardwareSN), nTotal, nFailed);
					}
				}
			}

			ACHtmlLogWriter ReportWriter;
			//切换Task时，需要输出一次报告
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


		delete ui;
	}

	void AngKMainFrame::InitAnimWgt()
	{
		m_animWgt = new AnimWidgetManager(ui->operateWidget);
		m_log = new AngKShowLogArea(this);
		m_log->setObjectName("AngKShowLogArea");
		connect(m_log, &AngKShowLogArea::sgnLogAreaExpand, this, &AngKMainFrame::onSlotLogAreaExpand);
		this->installEventFilter(m_animWgt);
		m_animWgt->addWidget(m_log->objectName(), m_log);
	}

	void AngKMainFrame::InitControlWgt()
	{
		connect(ui->controlWidget, &AngKControlWidget::sgnAllCheck, this, [=](int nState) {
			bool bCheck = false;
			if (nState == Qt::CheckState::Checked)
				bCheck = true;

			for (auto obj : ui->scrollAreaWidgetContents->children())
			{
				if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget"))
				{
					AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
					if (pro->GetDevConnectState())
						pro->setProgramerCheck(bCheck);
				}
			}
		});
		connect(ui->controlWidget, &AngKControlWidget::sgnStartBurnData, this, &AngKMainFrame::onSlotStartBurnData);
	}

	void AngKMainFrame::InitScrollArea()
	{
		//AngKMessageHandler::instance().Command_RemoteDoPTCmd(0, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_GetBPUEnable, ALL_BPU, QByteArray());

		if (g_AppMode == ConnectType::Demo)
		{
			InitDemoMode();
		}
		else {
			//测试代码
			std::map<std::string, BPUInfo> mapBPUInfo = m_projDataset->GetBPUInfo();

			std::map<std::string, DeviceStu> insertDev;
			AngKDeviceModel::instance().GetConnetDevMap(insertDev);
			//开发第一阶段eMMC只支持8个bpu，当前默认只有一个Ag06
			int nCount = 0;
			for (auto iter : insertDev) {
				QString progIPHop = QString::fromStdString(iter.second.strIP) + ":" + QString::number(iter.second.nHopNum);
				AngKProgrammerWidget* progWgt = new AngKProgrammerWidget(nCount, progIPHop);
				progWgt->setProgramerName(QString::fromStdString(iter.second.strSiteAlias));
				
				//progWgt->SetDevInfo(iter.second);
				m_pProgWgtVec[progIPHop] = progWgt;
				ui->scrollAreaWidgetContents->layout()->addWidget(progWgt);
				nCount++;
			}


		}

		for (auto varProg : m_pProgWgtVec) {
			connect(varProg.second, &AngKProgrammerWidget::sgnCurrentAllSelect, this, &AngKMainFrame::onSlotSetAllSelect);
		}

		//初始化获取时信号没有建立，需要主动调用判断一次
		onSlotSetAllSelect(true);

		ui->scrollAreaWidgetContents->layout()->addItem(new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Expanding));
		bool bDark = AngKGlobalInstance::GetSkinMode() == (int)ViewMode::Dark;
		QColor skinColor = bDark ? QColor(27, 27, 27) : QColor(250, 250, 250);
		ui->scrollAreaWidgetContents->setPalette(QPalette(skinColor));
		ui->scrollAreaWidgetContents2->setPalette(QPalette(skinColor));

		QString strVerStyle, strHorStyle;
		if (bDark)
		{
			strVerStyle = "QScrollBar:vertical {background-color: #1b1b1b; width: 8px;} \
			QScrollBar::handle:vertical{background-color: #464646; border-radius: 4px;}\
			QScrollBar::add-page:vertical{background: #1b1b1b;} \
			QScrollBar::sub-page:vertical{background: #1b1b1b;}";
			strHorStyle = "QScrollBar:horizontal{background-color: #1b1b1b; height: 8px;} \
			QScrollBar::handle:horizontal{background-color: #464646; border-radius: 4px;}\
			QScrollBar::add-page:horizontal{background: #1b1b1b;} \
			QScrollBar::sub-page:horizontal{background: #1b1b1b;}";
		}
		else
		{
			strVerStyle = "QScrollBar:vertical {background-color: #fafafa; width: 8px;} \
			QScrollBar::handle:vertical{background-color: #cccccc; border-radius: 4px;}\
			QScrollBar::add-page:vertical{background: #fafafa;} \
			QScrollBar::sub-page:vertical{background: #fafafa;}";
			strHorStyle = "QScrollBar:horizontal {background-color: #fafafa; height: 8px;} \
			QScrollBar::handle:horizontal{background-color: #cccccc; border-radius: 4px;}\
			QScrollBar::add-page:horizontal{background: #fafafa;} \
			QScrollBar::sub-page:horizontal{background: #fafafa;}";
		}


		ui->scrollArea->verticalScrollBar()->setStyleSheet(strVerStyle);
		ui->scrollArea->horizontalScrollBar()->setStyleSheet(strHorStyle);

		connect(ui->splitter_2, &QSplitter::splitterMoved, this, &AngKMainFrame::onSlotSplitterMoved);
	}

	void AngKMainFrame::InitPropetryWidget()
	{
		connect(ui->propetryWidget, &AngKPropetryWidget::sgnAddProject, this, &AngKMainFrame::onSlotAddProject);

	}

	void AngKMainFrame::DevRootPartCheck() {
		auto devList = ACDeviceManager::instance().getAllDevInfo();
		for (auto iter : devList) {
			if (iter.strMULocation.length() != 0 && iter.strMULocation.compare("Normal") != 0) {
				ACMessageBox::showError(this, tr("Device startup error"), tr("The boot partition of the programmer is abnormal; please upgrade the programmer's firmware."));
			}
		}
	}

	int AngKMainFrame::WaitCmdExeQueueAvailable(QString strIPHop, int Timeout, int AvailableCnt)
	{
		int Ret = -1;
		bool bTryRet = m_IPHop_projSemaphore[strIPHop]->tryAcquire(AvailableCnt, Timeout);
		if (bTryRet == true) {//如果能够得到，那么就立即释放
			m_IPHop_projSemaphore[strIPHop]->release(AvailableCnt);
			Ret = 0;
		}
		return Ret;
	}

	void AngKMainFrame::SetProjBindBPU(int nSktEn, QString strIP, int nHop, std::string strAdapterID)
	{
		for (auto obj : ui->scrollAreaWidgetContents->children()) {
			if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
				AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
				QString strIPHop = strIP + ":" + QString::number(nHop);
				if (strIPHop == pro->GetIpHop()) {
					//pro->ResetSiteCheck(nHop);
					pro->SetProjBindBPUState(nSktEn, strAdapterID);
				}
			}
		}
	}

	void AngKMainFrame::RecordTaskEvent(QString strIP, int nHop, int bResult, QString taskFile)
	{
		nlohmann::json LoadTaskJson;
		LoadTaskJson["FilePath"] = taskFile.toStdString();
		LoadTaskJson["RetCode"] = bResult;
		LoadTaskJson["RetInfo"] = bResult != 0 ? "Load Task Failed" : "Load Task Success";
		EventLogger->SendEvent(EventBuilder->GetLoadTask(LoadTaskJson));
		QJsonObject JobResult;
		QString strTaskLog = QString("Device %1:%2 Load task file : %3 ").arg(strIP).arg(nHop).arg(taskFile);
		if (bResult == 0) {
			strTaskLog += "successfully";
			JobResult["result"] = "successfully";
		}
		else {
			strTaskLog += "failed";
			JobResult["result"] = "failed";
		}
		
		JobResult["strip"] = strIP;
		JobResult["nHopNum"] = nHop;
		JobResult["taskFile"] = taskFile;
		//for (QTcpSocket* socket : JsonRpcServer::Instance()->m_scanSubscribers) {
		//	JsonRpcServer::Instance()->SendClientDoCmd(socket, "setLoadProjectResult", JobResult);
		//	//ALOG_INFO("%s.", "CU", "--", "test");
		//}
		ALOG_INFO("%s.", "CU", "--", strTaskLog.toStdString().c_str());
	}

	void AngKMainFrame::ExecuteBurnCommand(std::string& docmdSeqJson, QStringList& processList, int OperType)
	{
		int chipCMD = 0;
		TranslateOperType2Cmd((OperationTagType)OperType, chipCMD);
		nlohmann::json DoCmdSequenceJson;
		for (auto varOper : m_projDataset->GetOperInfoList())
		{
			if (varOper.iOpId == chipCMD)
			{
				if (varOper.vecOpList.empty()) {
					ACMessageBox::showWarning(this, tr("Warning"), tr("The currently executed command set is empty, please check before programming the chip."));
					return;
				}

				DoCmdSequenceJson["CmdSequencesGroupCnt"] = varOper.vecOpList.size();
				DoCmdSequenceJson["CmdRun"] = varOper.strOpName;
				DoCmdSequenceJson["CmdID"] = QString::number(chipCMD, 16).toUpper().toStdString();

				DoCmdSequenceJson["CmdSequences"] = nlohmann::json::array();
				for (int i = 0; i < varOper.vecOpList.size(); ++i)
				{
					nlohmann::json cmdJson;
					std::string strSubOper;
					TranslateSubCmd2String((ChipOperCfgSubCmdID)varOper.vecOpList[i], strSubOper);
					cmdJson["ID"] = QString::number(varOper.vecOpList[i], 16).toUpper().toStdString();
					cmdJson["Name"] = strSubOper;
					processList.push_back(QString::fromStdString(strSubOper));
					DoCmdSequenceJson["CmdSequences"].push_back(cmdJson);
				}
			}
		}

		docmdSeqJson = DoCmdSequenceJson.dump();
	}

	/**
	 * @brief 检查自动烧录记录，跟踪批次完成状态并在整个批次完成时通知自动机
	 * @param strIP 设备IP地址
	 * @param nHop 设备跳数
	 * @param nBPUIdx BPU索引
	 * @param nSktIdx SKT索引
	 * @param resultCode 烧录结果代码
	 */
	void AngKMainFrame::CheckAutoBurnRecord(QString strIP, int nHop, int nBPUIdx, int nSktIdx, QString resultCode)
	{
		ALOG_INFO("CheckAutoBurnRecord(%s, %d, %d,%d,%s);", "CU", "--", strIP.toStdString(), nHop, nBPUIdx, nSktIdx, resultCode.toStdString());

		// 1. 构建设备唯一标识符
		QString strIPHop = strIP + ":" + QString::number(nHop);

		// 2. 计算当前完成的SKT对应的位掩码
		BPU_SKT_VALUE sktValue = BPUCalVector[(nBPUIdx * 2) + nSktIdx];

		// 3. 用于存储已完成批次的记录（将在批次完成时被填充）
		std::pair<int, BurnRecord> completedBatchRecord;

		// 4. 遍历所有设备的自动烧录记录
		for (auto& deviceRecord : m_mapAutoRecord) {
			const std::string& deviceIdentifier = deviceRecord.first;  // 设备标识符 "IP:Hop"
			int initialBatchMask = deviceRecord.second.first;          // 该批次初始要烧录的SKT位掩码
			BurnRecord& burnProgress = deviceRecord.second.second;     // 该批次当前的烧录进度

			// 检查是否是目标设备
			if (deviceIdentifier == strIPHop.toStdString()) {
				// 检查当前完成的SKT是否属于这个批次的范围内
				int belongsToThisBatch = initialBatchMask & sktValue;
				if (belongsToThisBatch) {
					// 更新批次进度：标记这个SKT已完成
					burnProgress.curEnable |= belongsToThisBatch;
					burnProgress.maskDone[belongsToThisBatch] = resultCode;

					// 关键判断：检查这个批次是否已经全部完成
					int initialSktMask = initialBatchMask;     // 初始批次包含的所有SKT
					int completedSktMask = burnProgress.curEnable;  // 当前已完成的所有SKT

					if (initialSktMask == completedSktMask) {
						// 批次完全完成！保存记录以便后续处理
						completedBatchRecord = deviceRecord.second;
					}
				}
			}
		}

		// 5. 如果有批次完成，则通知自动机并清理记录
		int completedBatchInitialMask = completedBatchRecord.first;      // 已完成批次的初始掩码
		BurnRecord completedBurnProgress = completedBatchRecord.second;  // 已完成批次的进度记录

		if (completedBatchInitialMask != 0 && completedBurnProgress.curEnable != 0) {
			// 获取设备序列号和自动机插件
			std::string devSN = ACDeviceManager::instance().getDevInfo(strIP, nHop).tMainBoardInfo.strHardwareSN;
			auto autoPlugin = ACAutomaticManager::instance()->GetAutomaticPlugin();
			int siteIdx = autoPlugin->GetSiteIdxBySn(devSN);

			// 准备结果数组，用于告知自动机每个SKT的烧录结果
			int maxSktCount = 16;
			uint8_t* burnResults = new uint8_t[maxSktCount];
			memset(burnResults, 0, maxSktCount);

			// 遍历批次中每个已完成SKT的结果
			for (const auto& sktResultPair : completedBurnProgress.maskDone) {
				int sktMask = sktResultPair.first;              // SKT位掩码
				const QString& resultCode = sktResultPair.second; // 烧录结果

				int autoIdx_temp = Utils::AngKCommonTools::SwapSKTIdx_Soft2Auto(sktMask, maxSktCount);
				ALOG_INFO("autoIdx_temp:%d,autoIdx:%d", "CU", "--", autoIdx_temp, sktMask);

				// 将位掩码转换为具体的SKT索引结果
				for (int i = 0; i < maxSktCount; ++i) {
					// 检查第i位是否被设置（即第i个SKT是否完成）
					if ((sktMask >> i) & 1) {
						if (resultCode != "4000") {
							burnResults[i] = 2; // 烧录失败
						}
						else {
							burnResults[i] = 1; // 烧录成功
						}
					}
				}
			}

			// 通知自动机：该站点的一个批次已完成
			ALOG_INFO("autoPlugin->SetDoneSite(%d, %d, 0);", "CU", "--", siteIdx, burnResults);
			autoPlugin->SetDoneSite(siteIdx, burnResults, 0);
			delete[] burnResults;

			// 清理已完成的批次记录
			erase_item_by_key_and_value(m_mapAutoRecord, strIPHop.toStdString(), completedBatchRecord);
		}
		else {
			// 调试输出：条件不满足时的变量状态
			ALOG_INFO("条件不满足 - 设备:%s, BPU:%d, SKT:%d, 结果:%s", "CU", "--",
				strIPHop.toStdString().c_str(), nBPUIdx, nSktIdx, resultCode.toStdString().c_str());
			ALOG_INFO("completedBatchInitialMask = 0x%X (%d)", "CU", "--",
				completedBatchInitialMask, completedBatchInitialMask);
			ALOG_INFO("completedBurnProgress.curEnable = 0x%X (%d)", "CU", "--",
				completedBurnProgress.curEnable, completedBurnProgress.curEnable);
			ALOG_INFO("当前SKT位掩码 sktValue = 0x%X (%d)", "CU", "--", sktValue, sktValue);

			// 打印当前m_mapAutoRecord的状态
			ALOG_INFO("当前m_mapAutoRecord记录数量: %d", "CU", "--", (int)m_mapAutoRecord.size());
			for (const auto& deviceRecord : m_mapAutoRecord) {
				const std::string& deviceIdentifier = deviceRecord.first;
				int initialBatchMask = deviceRecord.second.first;
				const BurnRecord& burnProgress = deviceRecord.second.second;

				ALOG_INFO("设备:%s, 初始掩码:0x%X, 当前进度:0x%X, 完成SKT数量:%d", "CU", "--",
					deviceIdentifier.c_str(), initialBatchMask, burnProgress.curEnable,
					(int)burnProgress.maskDone.size());

				// 如果是当前设备，打印更详细的信息
				if (deviceIdentifier == strIPHop.toStdString()) {
					ALOG_INFO("  -> 这是当前设备的记录！", "CU", "--");
					ALOG_INFO("  -> 初始批次包含的SKT: 0x%X", "CU", "--", initialBatchMask);
					ALOG_INFO("  -> 当前已完成的SKT: 0x%X", "CU", "--", burnProgress.curEnable);
					ALOG_INFO("  -> 当前SKT是否属于此批次: %s", "CU", "--",
						(initialBatchMask & sktValue) ? "是" : "否");

					// 打印每个已完成SKT的详细信息
					for (const auto& maskResult : burnProgress.maskDone) {
						ALOG_INFO("    SKT掩码:0x%X, 结果:%s", "CU", "--",
							maskResult.first, maskResult.second.toStdString().c_str());
					}
				}
			}
		}
	}

	void AngKMainFrame::InitMessgaeConnect()
	{
		qRegisterMetaType<std::string>("std::string");
		qRegisterMetaType<uint32_t>("uint32_t");
		qRegisterMetaType<uint16_t>("uint16_t");
		qRegisterMetaType<nlohmann::json>("nlohmann::json");
		connect(ui->toolWidget, &AngKToolWidget::sgnOpenBufferTool, this, &AngKMainFrame::onSlotOpenBufferTool);
		connect(ui->toolWidget, &AngKToolWidget::sgnOpenSelectChipDlg, this, &AngKMainFrame::onSlotOpenSelectChipDlg);
		connect(&AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sgnHexEditInput, this, [=](quint64 uOffset, uchar uVal) {
			m_pDataBuffer->BufferWrite(uOffset, &uVal, 1);
			});
		connect(&AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sgnProperty2Mainframe, this, &AngKMainFrame::onSlotProperty2Mainframe);
		connect(&AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sigLinkStatusChanged, this, &AngKMainFrame::onSlotLinkStatusChanged);
		connect(this, &AngKMainFrame::sgnSupplyInstallDriver, this, &AngKMainFrame::onSlotSupplyInstallDriver);
		connect(this, &AngKMainFrame::sgnHandleEventInfo, this, &AngKMainFrame::onSlotHandleEventInfo,Qt::QueuedConnection);
		connect(this, &AngKMainFrame::sgnRecvDoCmdResult, this, &AngKMainFrame::onSlotRecvDoCmdResult);
		//?autodirection没有触发?，显式启用Qt::QueuedConnection跨线程
		bool connected = connect(this, &AngKMainFrame::sgnRecvDoCustom, this, &AngKMainFrame::onSlotRecvDoCustom, Qt::QueuedConnection);
		connect(this, &AngKMainFrame::sgnModifyDeviceInfo, this, &AngKMainFrame::onSlotModifyDeviceInfo, Qt::QueuedConnection);
		connect(this, &AngKMainFrame::sgnUpdateSKTEnable, this, &AngKMainFrame::onSlotUpdateSKTEnable, Qt::QueuedConnection);
		connect(this, &AngKMainFrame::sgnSetProgress, this, &AngKMainFrame::onSlotSetProgress, Qt::QueuedConnection);
		connect(this, &AngKMainFrame::sgnSetUpdateFw, this, &AngKMainFrame::onSlotSetUpdateFw);
		connect(this, &AngKMainFrame::sgnTaskInitChipInfo, this, &AngKMainFrame::onSlotTaskInitChipInfo);
		connect(this, &AngKMainFrame::sgnTaskSetProjBindBPU, this, &AngKMainFrame::onSlotTaskSetProjBindBPU, Qt::QueuedConnection);

		connect(ACAutomaticManager::instance()->GetAutomaticPlugin(), &IAutomatic::chipIsInPlace, this, &AngKMainFrame::onSlotAutomatic_ChipInPlace, Qt::QueuedConnection);
		connect(ACAutomaticManager::instance()->GetAutomaticPlugin(), &IAutomatic::printMessage, this, [=](QString msgStr) {
			ALOG_INFO("Recv Auto Message : %s", "CU", "AT", msgStr.toStdString().c_str(), Qt::DirectConnection);
			
			});
	}

	void AngKMainFrame::showToolWidget(bool isShow)
	{
		if (isShow)
			ui->toolWidget->show();
		else
			ui->toolWidget->hide();
	}

	void AngKMainFrame::showLogArea(bool state)
	{
		m_log->SetExpandCheck(false);
		m_log->setFixedWidth(ui->operateWidget->width());
		int checkHeight = ui->operateWidget->height() - (ui->processPathWidget->height() + ui->operateWidget->layout()->spacing() + m_oldMargins.top());
		//m_log->setMinimumHeight(ui->operateWidget->height() - (ui->processPathWidget->height() + ui->operateWidget->layout()->spacing() + m_oldMargins.top()));
		m_log->setFixedHeight(m_log->GetUIMinHeight());
		if (state)
		{
			ui->operateWidget->layout()->setContentsMargins(m_oldMargins.left(), m_oldMargins.top(), m_oldMargins.right(), m_oldMargins.bottom() + 100);
			m_log->move(0, ui->operateWidget->height() - (m_oldMargins.bottom() + 100));
			m_log->show();
			m_log->raise();
		}
		else
		{
			ui->operateWidget->layout()->setContentsMargins(m_oldMargins);
			m_log->move(0, ui->operateWidget->height() + 1);
			m_log->hide();
		}
	}

	void AngKMainFrame::setPropetryAreaShow(WinActionType state, bool bSHow)
	{
		switch (state)
		{
			case WinActionType::Project:
			case WinActionType::Programmer:
			case WinActionType::Site:
			{
				ui->propetryWidget->showArea((int)state, bSHow);
			}
				break;
		default:
			break;
		}
	}

	void AngKMainFrame::setLogText(QString text)
	{
		m_log->setLogText(text);
	}

	void AngKMainFrame::setProjDataset(AngKProjDataset* pDataSet)
	{
		m_projDataset = pDataSet;
		//ALOG_INFO("%s", "CU", m_projDataset->getChipData().json2String().c_str());
	}

	void AngKMainFrame::onSlotLogAreaExpand(int checkState)
	{
		switch (checkState)
		{
		case Qt::CheckState::Unchecked:
			{
				int nStart = ui->processPathWidget->height() + ui->operateWidget->layout()->spacing() + m_oldMargins.top();
				int nEnd = ui->operateWidget->height() - m_logWgtMinHeight;
				m_animWgt->popupWidget(m_log->objectName(), false, AnimDirection::Top, nStart, nEnd);
				m_log->setFixedHeight(m_log->GetUIMinHeight());
			}
			break;
		case Qt::CheckState::Checked:
			{
				int nStart = ui->operateWidget->height() - m_logWgtMinHeight;
				int nEnd = ui->processPathWidget->height() + ui->operateWidget->layout()->spacing() + m_oldMargins.top();
				m_animWgt->popupWidget(m_log->objectName(), true, AnimDirection::Top, nStart, nEnd);
				m_log->setFixedHeight(ui->operateWidget->height() - (ui->processPathWidget->height() + ui->operateWidget->layout()->spacing() + m_oldMargins.top()));
			}
			break;
		default:
			break;
		}
	}

	void AngKMainFrame::setDataBuffer(AngKDataBuffer* _dataBuffer)
	{
		m_pDataBuffer = _dataBuffer;
	}

	void AngKMainFrame::InstallDriver(QStringList mstDrvList, QStringList devDrvList, ChipDataJsonSerial chipInfo)
	{
		// 暂时不放开，因为GetBPUInfo还未完善，默认还是使用点击
		//CheckBPUAdapter(chipInfo.getJsonValue<std::string>("chipAdapter"));

		QByteArray Driver_IC_Data;
		QString chipDataXML;
		QString DDadrvFile;//Device Driver
		QString MDadrvFile;//Master Driver
		QString chipConfigXML;
		QString chkFile;
		uint64_t adrvFileSize;
		uint32_t calCRC32;

		for (auto path : devDrvList) {
			if (path.contains(".xml") && path.contains("chipData", Qt::CaseInsensitive)) {
				chipDataXML = path;
			}
			if (path.contains(".adrv") && path.contains("Drv")) {
				DDadrvFile = path;
			}
			else if (path.contains("chipconfig", Qt::CaseInsensitive)) {
				chipConfigXML = path;
			}
			else if (path.contains(".CHK", Qt::CaseInsensitive)) {
				chkFile = path;
				m_pChkManager = new AngKChkManager(chkFile);
			}
		}

		for (auto path : mstDrvList) {
			if (path.contains(".adrv") && path.contains("Mst")) {
				MDadrvFile = path;
			}
		}

		std::string default;
		SendDriverCmd("",0,DDadrvFile, MDadrvFile, default);
	}

	int AngKMainFrame::SendDriverCmd(QString strIP, int nHop, QString devDrv, QString mstDrv, std::string& strInstallDriver, uint32_t sktEn)
	{
		QByteArray Driver_IC_Data;
		QString chkFile;
		uint64_t adrvFileSize;
		uint32_t calCRC16;

		//adrv驱动文件分为masterDriver和DeviceDriver所以两个合并发送到DDR，每个Driver按照16的byte倍数
		nlohmann::json AllDriverJson;
		AllDriverJson["FileType"] = "Driver";
		AllDriverJson["IsCompress"] = 0;

		QFile ddFile(devDrv);
		if (ddFile.open(QIODevice::ReadOnly)) {

			int driverSize = 0;
			SwitchICData_CRC(ddFile, Driver_IC_Data, driverSize, calCRC16);

			AllDriverJson["DevDrvDDRAddr"] = (QString("0x") + QString("%1").arg(MUStartAddr - PCMUSectorSize, 8, 16, QLatin1Char('0')).toUpper()).toStdString();
			AllDriverJson["DevDrvDataLen"] = (QString("0x") + QString("%1").arg(driverSize, 8, 16, QLatin1Char('0')).toUpper()).toStdString();
			AllDriverJson["DevDrvCRC16"] = (QString("0x") + QString("%1").arg(calCRC16, 4, 16, QLatin1Char('0')).toUpper()).toStdString();
		}
		else {
			ALOG_FATAL("Open Device Driver File: %s Failed.", "CU", "--", devDrv.toStdString().c_str());
			return -1;
		}
		ddFile.close();

		QFile mdFile(mstDrv);
		if (mdFile.open(QIODevice::ReadOnly)) {

			AllDriverJson["MstDrvDDRAddr"] = (QString("0x") + QString("%1").arg(MUStartAddr - PCMUSectorSize + Driver_IC_Data.size(), 8, 16, QLatin1Char('0')).toUpper()).toStdString();

			int driverSize = 0;
			SwitchICData_CRC(mdFile, Driver_IC_Data, driverSize, calCRC16);

			AllDriverJson["MstDrvDataLen"] = (QString("0x") + QString("%1").arg(driverSize, 8, 16, QLatin1Char('0')).toUpper()).toStdString();
			AllDriverJson["MstDrvCRC16"] = (QString("0x") + QString("%1").arg(calCRC16, 4, 16, QLatin1Char('0')).toUpper()).toStdString();
		}
		else {
			ALOG_FATAL("Open Master Driver File: %s Failed.", "CU", "--", mstDrv.toStdString().c_str());
			return -1;
		}
		mdFile.close();

		//QFile tempFIle(QCoreApplication::applicationDirPath() + "/testDriver_IC_Data.bin");
		//if (tempFIle.open(QIODevice::ReadWrite | QIODevice::Truncate))
		//{
		//	//tempFIle.write(AllDriverJson.dump().c_str());
		//	tempFIle.write(Driver_IC_Data);
		//}
		//tempFIle.close();

		//return;

		DeviceStu devInfo = ACDeviceManager::instance().getDevInfo(strIP, nHop);
		ALOG_INFO("Send driver file data to %s:%d DDR", "CU", "FP", strIP.toStdString().c_str(), nHop);
		int ret = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(strIP.toStdString(), "FIBER2DDR", nHop, 0, DDR_SharedMemeroyExchangePC2MUOffset_PL, Driver_IC_Data, devInfo.tMainBoardInfo.strHardwareSN, "DriverData");
		QThread::msleep(10);//提高UDP的稳定性，每发送一次命令包，停10ms
		if (ret == 0)
		{
			strInstallDriver = AllDriverJson.dump();
		}
		else
		{
			ALOG_FATAL("Send driver file data to %s:%d DDR failed(errorCode: %d)", "CU", "FP", strIP.toStdString().c_str(), nHop, ret);
			ret = -1;
		}
		return ret;
	}

	int AngKMainFrame::SendFPGACmd(QString strIP, int nHop, QString strFPGA, std::string& strInstallFPGAJson, uint32_t sktEn)
	{
		QByteArray FPGA_Data;
		QString chkFile;
		uint64_t FPGAFileSize;
		uint32_t calCRC32 = 0;

		//adrv驱动文件分为masterDriver和DeviceDriver所以两个合并发送到DDR，每个Driver按照16的byte倍数
		nlohmann::json FPGAJson;
		FPGAJson["FileType"] = "FPGA";
		FPGAJson["IsCompress"] = 0;

		QFile fpgaFile(strFPGA);
		if (fpgaFile.open(QIODevice::ReadOnly)) {

			int FPGASize = 0;
			SwitchICData_CRC(fpgaFile, FPGA_Data, FPGASize, calCRC32, false);

			FPGAJson["DDROffset"] = (QString("0x") + QString("%1").arg(0, 8, 16, QLatin1Char('0')).toUpper()).toStdString();
			FPGAJson["DataSize"] = (QString("0x") + QString("%1").arg(fpgaFile.size(), 8, 16, QLatin1Char('0')).toUpper()).toStdString();
			FPGAJson["CRC32"] = (QString("0x") + QString("%1").arg(calCRC32, 4, 16, QLatin1Char('0')).toUpper()).toStdString();
		}
		else {
			ALOG_FATAL("Open Device Driver File: %s Failed.", "CU", "--", strFPGA.toStdString().c_str());
			return -1;
		}
		fpgaFile.close();

		//QFile tempFIle(QCoreApplication::applicationDirPath() + "/testDriver_IC_Data.bin");
		//if (tempFIle.open(QIODevice::ReadWrite | QIODevice::Truncate))
		//{
		//	//tempFIle.write(AllDriverJson.dump().c_str());
		//	tempFIle.write(Driver_IC_Data);
		//}
		//tempFIle.close();

		//return;

		DeviceStu devInfo = ACDeviceManager::instance().getDevInfo(strIP, nHop);
		ALOG_INFO("Send FPGA file data to %s:%d DDR", "CU", "FP", strIP.toStdString().c_str(), nHop);
		int ret = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(strIP.toStdString(), "FIBER2DDR", nHop, 0, 0x0, FPGA_Data, devInfo.tMainBoardInfo.strHardwareSN, "FPGAData");
		QThread::msleep(10);//提高UDP的稳定性，每发送一次命令包，停10ms
		if (ret == 0)
		{
			strInstallFPGAJson = FPGAJson.dump();
		}
		else
		{
			ALOG_FATAL("Send FPGA file data to %s:%d DDR failed(errorCode: %d)", "CU", "FP", strIP.toStdString().c_str(), nHop, ret);
			ret = -1;
		}
		return ret;
	}


	void AngKMainFrame::onSlotAddProject()
	{
		//QString testDll = m_pDataBuffer->GetChipFile(checkFile);
		//QLibrary myLib(testDll);
		//bool bSuccess = myLib.load();
		if (m_pChkManager != nullptr) {
			bool bSuccess = m_pChkManager->LoadChk();
			int byte = m_pChkManager->GetCheckSumName();
			CheckSumParam testParam = { 0 };
			testParam.uiCheckSumType = 1;
			testParam.uiAgIc = 0;
			uchar* spcBuffer = new uchar[1024];
			memset((uchar*)spcBuffer, 3, 1024);
			testParam.pcSpcBuffer = spcBuffer;
			testParam.uiSpcBuffLen = 1024;
			testParam.pDataBuffer = m_pDataBuffer;
			testParam.pOutput = static_cast<IOutput*>(Utils::AngkLogger::logger());
			testParam.uiVChkSum = 0;
			testParam.blockJsonString = "";
			m_pChkManager->CalCheckSum(&testParam);
		}
	}

	void AngKMainFrame::onSlotRemoteCmdComplete(QString recvIP, uint32_t HopNum, uint32_t PortID, int32_t ResultCode, uint16_t CmdID, uint32_t SktEnValue, uint16_t nBPUID, QByteArray RespDataBytes, uint32_t RespDataSize)
	{

		//m_pRemoteCmdManager->StartTimer();
		QString strIPHop = recvIP + ":" + QString::number(HopNum);
		m_IPHop_projCommResultCode[strIPHop].push_back(ResultCode);
		//ALOG_INFO("onSlotRemoteCmdComplete thread id : %d", "CU", "--", QThread::currentThreadId());
		//auto boundFunc = std::bind(&AngKMainFrame::SyncRemoteCmdComplete, this, recvIP, HopNum, PortID, ResultCode, CmdID, SktEnValue, nBPUID, RespDataBytes);
		//std::future<void> result = std::async(std::launch::async, boundFunc);
		std::string fromDev = Utils::AngKCommonTools::GetLogFrom(nBPUID);
		switch ((eSubCmdID)CmdID)
		{
		case eSubCmdID::SubCmd_MU_InstallFPGA:
		{
			if (m_IPHop_projSemaphore[strIPHop] != nullptr)
				m_IPHop_projSemaphore[strIPHop]->release(1);
		}
		case eSubCmdID::SubCmd_MU_InstallDriver:
		{
			if (m_IPHop_projSemaphore[strIPHop] != nullptr)
				m_IPHop_projSemaphore[strIPHop]->release(1);
		}
		break;
		case eSubCmdID::SubCmd_MU_SetChipInfo:
		{
			if (m_IPHop_projSemaphore[strIPHop] != nullptr)
				m_IPHop_projSemaphore[strIPHop]->release(1);
		}
		break;
		case eSubCmdID::SubCmd_MU_SetDriverCommon:
		{
			if (m_IPHop_projSemaphore[strIPHop] != nullptr)
				m_IPHop_projSemaphore[strIPHop]->release(1);
		}
		break;
		case eSubCmdID::SubCmd_MU_SetDriverSelfPara:
		{
			if (m_IPHop_projSemaphore[strIPHop] != nullptr)
				m_IPHop_projSemaphore[strIPHop]->release(1);
		}
		break;
		case eSubCmdID::SubCmd_MU_SetBPUAttribute:
		{
			if (m_IPHop_projSemaphore[strIPHop] != nullptr)
				m_IPHop_projSemaphore[strIPHop]->release(1);
		}
		break;
		case eSubCmdID::SubCmd_MU_SetPartitionTableHeadAddr: {
			if (m_IPHop_projSemaphore[strIPHop] != nullptr)
				m_IPHop_projSemaphore[strIPHop]->release(1);
		}
		break;
		case eSubCmdID::SubCmd_MU_DoCmdSequence:
		{
			emit sgnRecvDoCmdResult(recvIP, HopNum, ResultCode, nBPUID);
			/*if (!bAutoConnect) */{
				uint64_t doCmdTimeDiff = QDateTime::currentDateTime().toTime_t() - m_nRecordDocmdTime;
				// 计算小时数
				qint64 hours = doCmdTimeDiff / 3600;
				// 计算剩余的秒数（去除小时后的秒数）
				qint64 remainingSeconds = doCmdTimeDiff % 3600;
				// 将时间间隔转换为分钟和秒
				int minutes = remainingSeconds / 60;
				int seconds = remainingSeconds % 60;
				std::string strCurCmd = AngKProjectOperation::TranslateProgOperType((OperationTagType)ui->toolWidget->GetSelectOperType());
				ALOG_INFO("Device %s:%d DoCmdSequence %s elapsed time : %d hours %d min %d seconds.", fromDev.c_str(), "CU", recvIP.toStdString().c_str(), HopNum, strCurCmd.c_str(), hours, minutes, seconds);
			}
		}
		break;
		case eSubCmdID::SubCmd_MU_GetBPUInfo:
		{
			//RespDataBytes

			QVector<BPU_SKT_VALUE> BPU_UseNum;
			for (int i = 0; i < 16; ++i)
			{
				if ((SktEnValue >> i) & 0x1)
				{
					uint32_t swValue = SktEnValue & (1 << i);
					BPU_UseNum.push_back((BPU_SKT_VALUE)swValue);
				}
			}

			//ALOG_INFO("GetBPUEnable = %d", BPU_UseNum.size());
			//ui->scrollAreaWidgetContents->layout()->addWidget(new AngKProgrammerWidget(BPU_UseNum));
		}
		break;
		case eSubCmdID::SubCmd_MU_UpdateFw:
		{
			emit sgnSetUpdateFw(recvIP, HopNum, ResultCode, RespDataBytes, RespDataSize);
		}
		break;
		case eSubCmdID::SubCmd_MU_RebootMU:
		{
			if (ResultCode != 0) {
				QString errorMsg = Utils::AngKCommonTools::TranslateErrorMsg(ResultCode);
				ALOG_FATAL("Reboot %s:%d Dev Failed, ResultCode = %d, Fail reason: %s.", "MU", "CU", recvIP.toStdString().c_str(), HopNum, ResultCode, errorMsg.toStdString().c_str());
				//TODO 根据不同的设备启动错误信息，做出对应的处理

			}
			else {
				std::map<std::string, DeviceStu> insertDev;
				AngKDeviceModel::instance().GetConnetDevMap(insertDev);
				std::string strIPHop = recvIP.toStdString() + ":" + QString::number(HopNum).toStdString();
				nlohmann::json ProgStatusChangeJson;
				ProgStatusChangeJson["ProgSN"] = insertDev[strIPHop].tMainBoardInfo.strHardwareSN;
				ProgStatusChangeJson["HopNum"] = HopNum;
				ProgStatusChangeJson["Status"] = "Reboot";
				ProgStatusChangeJson["Message"] = "Programmer Reboot By PC";
				EventLogger->SendEvent(EventBuilder->GetFWUpdate(ProgStatusChangeJson));
			}

		}
		break;
		case eSubCmdID::SubCmd_MU_GetRebootCause:
		{
			std::string strInfo = std::string(RespDataBytes.constData(), RespDataSize);
			try {
				nlohmann::json rebootFWJson = nlohmann::json::parse(strInfo);
				EventLogger->SendEvent(rebootFWJson.dump());
			}
			catch (const nlohmann::json::exception& e) {
				ALOG_FATAL("return GetRebootCause Json parse failed : %s.", "CU", "--", e.what());
			}
		}
		break;
		case eSubCmdID::SubCmd_MU_GetSktInfo:
		{
			std::string strInfo = std::string(RespDataBytes.constData(), RespDataSize);
			std::string strIPHop = recvIP.toStdString() + ":" + QString::number(HopNum).toStdString();
			emit sgnShowSktInfo(strInfo, strIPHop);
			emit sgnModifyDeviceInfo(strInfo, strIPHop);
		}
		break;
		case eSubCmdID::SubCmd_MU_GetSktInfoSimple:
		{
			std::string strInfo = std::string(RespDataBytes.constData(), RespDataSize);
			std::string strIPHop = recvIP.toStdString() + ":" + QString::number(HopNum).toStdString();
			emit sgnShowSktInfoSimple(strInfo, strIPHop);
		}
		break;
		case eSubCmdID::SubCmd_MU_GetSKTEnable:
		{
			std::string strInfo = std::string(RespDataBytes.constData(), RespDataSize);
			emit sgnUpdateSKTEnable(strInfo, recvIP.toStdString(), HopNum);
		}
		break;
		case eSubCmdID::SubCmd_MU_GetProgramSetting:
		{
			if (ResultCode != 0) {
				return;
			}
			std::string strInfo = std::string(RespDataBytes.constData(), RespDataSize);
			try {
				nlohmann::json ProgramSettingJson = nlohmann::json::parse(strInfo);
				std::string strMode = ProgramSettingJson["SKTNotifyMode"];
				std::string strValue = ProgramSettingJson["SKTNotifyValue"];
				std::string strAlias = ProgramSettingJson["ProgAlias"];
				std::string strIP = ProgramSettingJson["ProgIP"];
				std::string strMac = ProgramSettingJson["ProgMac"];
				emit sgnProgramSetting(QString::fromStdString(strMode).toInt(), QString::fromStdString(strValue).toInt());

				//默认配置中的需要重新设置
				DeviceStu devCopy = ACDeviceManager::instance().getDevInfo(recvIP, HopNum);
				//devCopy.strIP = strIP;
				devCopy.strSiteAlias = strAlias;
				devCopy.strMac = strMac;
				ACDeviceManager::instance().setDevInfo(recvIP, HopNum, devCopy);
			}
			catch (const nlohmann::json::exception& e) {
				ALOG_FATAL("return GetProgramSetting Json parse failed : %s.", "CU", "--", e.what());
			}
		}
		break;
		case eSubCmdID::SubCmd_MU_ProgrammerSelfTest:
		{
			std::string strInfo = std::string(RespDataBytes.constData(), RespDataSize);
			try {
				nlohmann::json ProgramSelfTestJson = nlohmann::json::parse(strInfo);
				std::string strSelfTest = ProgramSelfTestJson["Self Test"];
				emit sgnProgramSelfTestResult(QString::fromStdString(strSelfTest), recvIP, HopNum);
			}
			catch (const nlohmann::json::exception& e) {
				ALOG_FATAL("return ProgrammerSelfTest Json parse failed : %s.", "CU", "--", e.what());
			}
		}
		break;
		case eSubCmdID::SubCmd_MU_MasterChipAnalyze:
		{
			emit sgnMasterChipAnalyzeResult(ResultCode, fromDev);
			m_bAnalyzeProgress = false;
		}
		break;

		default:
			break;
		}
	}

	void AngKMainFrame::onSlotRemoteQueryDoCmd(QString recvIP, uint32_t HopNum, uint32_t PortID, uint32_t CmdFlag, uint16_t CmdID, uint16_t BPUID, QByteArray CmdDataBytes)
	{
		// 将 QByteArray 转换为 16 进制字符串
		QString hexString1 = CmdDataBytes.toHex().toUpper(); // 转换为大写，更常见
		// 可以在控制台打印 16 进制字符串
		QByteArray byteArray1 = hexString1.toLocal8Bit();
		const char* hexStringCStr1 = byteArray1.data();
		QDataStream DataStream(&CmdDataBytes, QIODevice::ReadOnly);
		DataStream.setByteOrder(QDataStream::LittleEndian);//设置为小端方式

		std::string fromDev = Utils::AngKCommonTools::GetLogFrom(BPUID);
		std::string cmdName = Utils::AngKCommonTools::TranslateMessageCmdID(CmdID);

		switch ((eSubCmdID)CmdID)
		{
		case eSubCmdID::SubCmd_MU_SetProgress:
		{
			uint32_t Current;
			uint32_t Total;

			DataStream >> Current >> Total;
			emit sgnSetProgress(recvIP, HopNum, Current, Total, BPUID, fromDev, cmdName);
		}
		break;
		case eSubCmdID::SubCmd_MU_SetLog:
		{
			uint32_t LogLevel;
			uint32_t LogFrom;
			uint16_t LogMsgSize;
			char LogMsg[1024];

			memset(LogMsg, 0, 1024);
			DataStream >> LogLevel >> LogFrom >> LogMsgSize;
			DataStream.readRawData(LogMsg, LogMsgSize);

			std::string filteredStr = LogMsg;

			// 使用STL的remove_if和erase函数来过滤末尾的\r\n
			filteredStr.erase(std::find_if(filteredStr.rbegin(), filteredStr.rend(),
				[](char c) {
					return (c != '\r' && c != '\n');
				}).base(), filteredStr.end());

			//LogLevel 需要进行比较 当前PC的等级
			SwitchLogPrint(LogLevel, fromDev, cmdName, recvIP.toStdString(), HopNum, filteredStr);


		}
		break;
		case eSubCmdID::SubCmd_MU_SetEvent:
		{
			//关键事件根据json字符串进行转换
			//例：烧录命令状态改变计算当前产生的时间
			uint16_t EventMsgSize;
			char EventMsg[1024];

			memset(EventMsg, 0, 1024);
			DataStream >> EventMsgSize;
			DataStream.readRawData(EventMsg, EventMsgSize);
			ALOG_INFO("Recv %s Message from %s:%d", fromDev.c_str(), "CU", cmdName.c_str(), recvIP.toStdString().c_str(), HopNum);
			std::string eventInfo = EventMsg;
			QString strIPHop = recvIP + ":" + QString::number(HopNum);
			emit sgnHandleEventInfo(strIPHop, eventInfo);
		}
		break;
		case eSubCmdID::SubCmd_MU_DoCustom: {
			//QByteArray recvCustomData(CmdDataBytes); 
		// 打印 customDataBytes 的内容 
			QString customDataBytesHex = CmdDataBytes.toHex().toUpper();
			// 直接使用 CmdDataBytes 作为 recvCustomData
			QByteArray recvCustomData = CmdDataBytes;
			QString strIPHop = recvIP + ":" + QString::number(HopNum);
			QString hexString = recvCustomData.toHex().toUpper();
			emit sgnRecvDoCustom(strIPHop, BPUID, recvCustomData);

			//connect(this, &AngKMainFrame::sgnRecvDoCustom, [this, strIPHop, BPUID, recvCustomData]() {
				//ALOG_INFO("Signal received!.", "CU", "--");
				//this->onSlotRecvDoCustom(strIPHop, BPUID, recvCustomData); // 手动调用槽
				//});
			//onSlotRecvDoCustom(strIPHop, BPUID, recvCustomData);
			// 以上lambada表达式or直接触发有用
			//QCoreApplication::processEvents(); // 立即处理事件,测试信号发送后强制进入主事件循环，经测试无用
			
		}
		break;
		default:
			break;
		}
	}

	void AngKMainFrame::onSlotOpenBufferTool()
	{
		AngKBufferHexEdit editDlg(m_pDataBuffer, this);
		editDlg.exec();
	}

	void AngKMainFrame::onSlotOpenSelectChipDlg()
	{
		if (m_pChipSelectDlg == nullptr) {
			m_pChipSelectDlg = new AngKChipDialog(this);
			m_pChipSelectDlg->SetTitle(tr("Select Chip"));
			m_pChipSelectDlg->InitChipData();

			//connect(&AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sgnSelectChipDataJson, this, &AngKMainFrame::onSlotSendChipInfo);

			connect(m_pChipSelectDlg, &AngKChipDialog::sgnSelectChipDataJson, this, &AngKMainFrame::onSlotSendChipInfo);
		}

		m_pChipSelectDlg->show();
	}

	void AngKMainFrame::onSlotSendChipInfo(ChipDataJsonSerial chipInfo)
	{
		//检查是否选择了同种芯片，同种芯片不重新加载
		std::string curChipName = m_projDataset->getChipData().getJsonValue<std::string>("chipName");
		if (!curChipName.empty()) {
			std::string curBoardName = m_projDataset->getChipData().getJsonValue<std::string>("bottomBoard");
			std::string curAdapterName = m_projDataset->getChipData().getJsonValue<std::string>("chipAdapter");

			std::string selectChipName = chipInfo.getJsonValue<std::string>("chipName");
			std::string selectBoardName = chipInfo.getJsonValue<std::string>("bottomBoard");
			std::string selectAdapterName = chipInfo.getJsonValue<std::string>("chipAdapter");

			//if (curChipName == selectChipName && curBoardName == selectBoardName && curAdapterName == selectAdapterName) {
			//	ACMessageBox::showWarning(this, tr("Warning"), tr("The currently selected chip matches the loaded chip"));
			//	return;
			//}
		}

		InitChipInfo(chipInfo);

		//工程Cmd操作Json需要同步更新
		//if (m_pProjCreateDlg != nullptr)
		//{
		//	m_pProjCreateDlg->InitOperationPage(m_projDataset->getChipData().getJsonValue<std::string>("chipOperCfgJson"));
		//}

		m_pChipSelectDlg->hide();
	}

	void AngKMainFrame::onSlotProperty2Mainframe(QString strProjName, int propIdx)
	{
		if (strProjName.isEmpty())
			return;
		
		AngKProjFile pFile(this, 0);
		if (E_FMTFILE_ERRER == pFile.LoadFile(strProjName)) {
			ALOG_FATAL("PropertyPage load project file error, strFile Name: %s.", "CU", "--", strProjName.toStdString().c_str());
			return;
		}
		
		
		{//更新操作Operator权限
			std::string chipInfo; 
			BaseOper baseInfo;
			if (E_FMTFILE_OK == pFile.GetTagData(TAG_CHIPDATA, chipInfo)) {
				pFile.dencodeBase64_Json(chipInfo);//base64需要解码

				try {//nlohmann解析失败会报异常需要捕获一下
					auto j3 = nlohmann::json::parse(chipInfo);
					ChipDataJsonSerial chipDataSer;
					chipDataSer.copyJson(j3);
					m_projDataset->setChipData(chipDataSer);
					std::string OperCfgJson = j3["chipOperCfgJson"].get<std::string>();

					if (!OperCfgJson.empty()) {
						nlohmann::json _CfgJson = nlohmann::json::parse(OperCfgJson);
						nlohmann::json baseOperJson = _CfgJson["baseOper"];
						if (!baseOperJson.is_null()) {
							baseInfo.bBlank = baseOperJson["blank"];
							baseInfo.bBlockProg = baseOperJson["blockProg"];
							baseInfo.bErase = baseOperJson["erase"];
							baseInfo.bFunction = baseOperJson["function"];
							baseInfo.bIllegalBit = baseOperJson["illegalBit"];
							baseInfo.bProg = baseOperJson["prog"];
							baseInfo.bRead = baseOperJson["read"];
							baseInfo.bSecure = baseOperJson["secure"];
							baseInfo.bVerify = baseOperJson["verify"];
						}
					}
				}
				catch (const nlohmann::json::exception& e) {
					ALOG_FATAL("onSlotProperty2Mainframe Operator Json parse failed : %s.", "CU", "--", e.what());
				}
			}
			ui->toolWidget->ShowButtonFromJson(baseInfo);
		}

		
	}

	void AngKMainFrame::onSlotAutomaticCheckSitesTask() {
		int allExpectNum = 0;
		bool bAbnormal = false;
		QString devListStr;
		for (auto obj : ui->scrollAreaWidgetContents->children()) {
			if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
				AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
				if (pro && pro->GetDevConnectState()) {

					uint32_t siteNum = pro->GetSitesChecked(allExpectNum);
					if (siteNum > 0) {
						DeviceStu devCopy = pro->GetDevInfo();
						if (devCopy.ProgEnvReady != ProEnvStatus::Success) {
							bAbnormal = true;
							devListStr += devCopy.strIP.c_str();
							devListStr += ":";
							devListStr += QString::number(devCopy.nHopNum);
							devListStr += " ";
						}
					}
				}
			}
		}

		if (bAbnormal) {
			QString warningStr = tr("Abnormal task attributes of  device %1").arg(devListStr) + tr(",Do you want to continue with the programming?");
			ACMessageBox::ACMsgType ret = ACMessageBox::showWarning(this, tr("Warning"), warningStr,
				ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);

			if (ret != ACMessageBox::ACMsgType::MSG_OK)
			{
				emit sgnContinueRunAutomatic(false);
				return;
			}
		}
		emit sgnContinueRunAutomatic(true);
	}



	void AngKMainFrame::onSlotStartBurnData()
	{
		// Demo模式直接返回
		if (g_AppMode == ConnectType::Demo) {
			return;
		}

		// 没有选择芯片则直接返回，不进行数据下发
		std::string chipDataName = m_projDataset->getChipData().getJsonValue<std::string>("chipName");
		std::string curManufacture = m_projDataset->getChipData().getJsonValue<std::string>("manufacture");
		if (chipDataName.empty()) {
			ACMessageBox::showWarning(this, tr("Warning"), tr("Please select the chip first, Project->Create->Chip..."));
			return;
		}
		// 没有选择烧录命令也提示报错
		if ((OperationTagType)ui->toolWidget->GetSelectOperType() == OperationTagType::None) {
			ACMessageBox::showWarning(this, tr("Warning"), tr("Please select the program command first."));
			return;
		}

		int chipCMD = 0;
		m_mapProgBPUChecked.clear();
		std::map<QString, DeviceStu> mapProgIP;
		int allExpectNum = 0;
		int nDevCount = 0;
		//统计选中site进行烧录
		for (auto obj : ui->scrollAreaWidgetContents->children()) {
			if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
				AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
				if (pro && pro->GetDevConnectState()) {

					uint32_t siteNum = pro->GetSitesChecked(allExpectNum);
					if (siteNum > 0) {
						nDevCount++;
						//判断烧录前的task是否都已经完成
						DeviceStu devCopy = pro->GetDevInfo();
						//判断烧录前的task是否都已经完成
						if (devCopy.ProgEnvReady == ProEnvStatus::Failed) {
							ACMessageBox::showError(this, tr("Error"), tr("Device %1:%2 load task failed, can't execute burning command.").arg(QString::fromStdString(devCopy.strIP)).arg(QString::number(devCopy.nHopNum)));
							continue;
						}

						if (devCopy.ProgEnvReady == ProEnvStatus::Idle) {
							ACMessageBox::showError(this, tr("Error"), tr("Device %1:%2 not loaded task, unable to execute burning command.").arg(QString::fromStdString(devCopy.strIP)).arg(QString::number(devCopy.nHopNum)));
							continue;
						}

						if (devCopy.ProgEnvReady == ProEnvStatus::Abnormal) {
							ACMessageBox::showWarning(this, tr("Warn"), tr("Device %1:%2 task is abnormal, needs to be download again.").arg(QString::fromStdString(devCopy.strIP)).arg(QString::number(devCopy.nHopNum)));
							continue;
						}
						if (devCopy.ProgEnvReady == ProEnvStatus::SuccessButCurTskChanged) {
							ACMessageBox::showWarning(this, tr("Warn"), tr("Device %1:%2 task is different with current task, needs to be download again.").arg(QString::fromStdString(devCopy.strIP)).arg(QString::number(devCopy.nHopNum)));
							continue;
						}

						m_mapProgBPUChecked[pro->GetDevProgramIndex()] = siteNum;
						mapProgIP[pro->GetDevProgramIndex()] = pro->GetDevInfo();
					}
				}
			}
		}

		if (m_mapProgBPUChecked.size() == 0) {
			if (nDevCount == 0) {
				ACMessageBox::showWarning(this, tr("Warning"), tr("No site needs to be burned."));
			}
			return;
		}

		//没有选择烧录座Site也提示报错
		uint32_t CountBPUChecked = 0;
		for (auto bpuChcek : m_mapProgBPUChecked) {
			CountBPUChecked += bpuChcek.second;

			if (CountBPUChecked == 0) {
				ACMessageBox::showWarning(this, tr("Warning"), tr("Device %1 Please select at least one site to program the chip.").arg(bpuChcek.first));
				return;
			}
		}

		//统计将要执行的cmd命令
		std::string docmdSeqJson;
		QStringList strList;
		ExecuteBurnCommand(docmdSeqJson, strList, ui->toolWidget->GetSelectOperType());

		if (docmdSeqJson == "null") {
			ACMessageBox::showWarning(this, tr("Warning"), tr("Program command abnormal, Please select command again."));
			return;
		}

		//执行命令流程展示
		QString processPath;
		for (int i = 0; i < strList.size(); ++i) {
			if (i == strList.size() - 1) {
				processPath += strList[i];
				break;
			}

			processPath += strList[i] + ">>";
		}
		ui->processPathWidget->setPath(processPath);

		if ((OperationTagType)ui->toolWidget->GetSelectOperType() == OperationTagType::Program)
		{
			QString projName;
			for (auto& projIter : m_pTaskManager->GetAllProjInfo().toStdMap()) {
				projName = projIter.first;
			}

			ACReportRecord& GReporter = GetReporter();
			GReporter.SetFuncmode(projName, "Program");
			GReporter.SetCmdSequence(projName, processPath);
		}

		ResetALLSiteStatus();
		ui->reportWidget->ResetExpectAndOutput();
		ui->reportWidget->SetExpectSlot(allExpectNum);

		std::string tmp = docmdSeqJson;
		if (!docmdSeqJson.empty()) {
			SetBurnStatus(true);
			for (auto iter : m_mapProgBPUChecked) {
				ALOG_INFO("Manual Mode: Processing device index=%s, sktEn=0x%X", "CU", "--", iter.first.toStdString().c_str(), iter.second);
				for (auto obj : ui->scrollAreaWidgetContents->children()) {
					if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
						AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
						int sitecout = 0;
						uint32_t siteChecked = pro->GetSitesChecked(sitecout);
						DeviceStu checkDev = pro->GetDevInfo();
						ALOG_INFO("Manual Mode: Checking widget device IP=%s, Hop=%d, siteChecked=0x%X, target IP=%s, Hop=%d", 
							"CU", "--", checkDev.strIP.c_str(), checkDev.nHopNum, siteChecked, mapProgIP[iter.first].strIP.c_str(), mapProgIP[iter.first].nHopNum);
						if ((chipDataName == "XSA300D" && curManufacture == "XT" && (OperationTagType)ui->toolWidget->GetSelectOperType() == OperationTagType::Program) ||
							(chipDataName == "MSI270" && curManufacture == "XT" && (OperationTagType)ui->toolWidget->GetSelectOperType() == OperationTagType::Program) ||
							(chipDataName == "XSG300D" && curManufacture == "XT" && (OperationTagType)ui->toolWidget->GetSelectOperType() == OperationTagType::Program))
						{
							ALOG_INFO("Manual Mode: Start UUID distribution for chip %s, siteChecked=0x%X", "CU", "--", chipDataName.c_str(), siteChecked);
							// 遍历16个skt（0-15）
							for (int skt = 0; skt < 16; ++skt)
							{
								// 检查当前skt是否使能 (从最低位skt0开始)
								if (siteChecked & (1u << skt))
								{
									std::string uid = realGenerator->getUID();
									realGenerator->setUIDResult(uid, 1);
									// 构造只包含当前skt使能的数值
									uint32_t singleSktNum = 1u << skt;
									ALOG_INFO("Manual Mode: Sending UUID for SKT[%d], UUID=%s, singleSktNum=0x%X, IP=%s, Hop=%d", 
										"CU", "--", skt, uid.c_str(), singleSktNum, mapProgIP[iter.first].strIP.c_str(), mapProgIP[iter.first].nHopNum);
									AngKMessageHandler::instance().Command_RemoteDoPTCmd(
										mapProgIP[iter.first].strIP,  // devIP
										mapProgIP[iter.first].nHopNum, // HopNum
										0,                            // PortID
										0,                            // CmdFlag
										(uint16_t)eSubCmdID::SubCmd_MU_DoCustom, // CmdID
										singleSktNum,                 // SKTNum (仅当前skt使能)
										8,                            // BPUID
										QByteArray(uid.c_str())        // CmdDataBytes
									);
								}
							}
							ALOG_INFO("Manual Mode: UUID distribution completed", "CU", "--");
						}
						DeviceStu proDev = pro->GetDevInfo();
						if (proDev.strIP == mapProgIP[iter.first].strIP && proDev.nHopNum == mapProgIP[iter.first].nHopNum) {
							pro->RestSiteStatus();
							pro->SetSiteStatus(iter.second, SiteStatus::Busy);
						}
					}
				}
				ALOG_INFO("Manual Mode: Sending DoCmdSequence, sktEn=0x%X, IP=%s, Hop=%d", 
					"CU", "--", iter.second, mapProgIP[iter.first].strIP.c_str(), mapProgIP[iter.first].nHopNum);
				AngKMessageHandler::instance().Command_RemoteDoPTCmd(mapProgIP[iter.first].strIP, mapProgIP[iter.first].nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_DoCmdSequence
					, iter.second, 8, QByteArray(docmdSeqJson.c_str()));
				m_nRecordDocmdTime = QDateTime::currentDateTime().toTime_t();
			}
		}
		else {
			ACMessageBox::showWarning(this, tr("Warning"), tr("The burning command set is not set."));
			return;
		}
	}

	void AngKMainFrame::onSlotUpdateFirmwareFile(QString devIP, int nHopNum, QString strFile, QString fromVer, QString toVer)
	{
		//固件升级文件先放到DDR，在下发命令升级
		QByteArray Firmware_Data;
		int firmwareSize = 0;
		uint32_t uCrc16chk = 0;
		QString strIPHop = devIP + ":" + QString::number(nHopNum);
		QFile firmFile(strFile);


		if (firmFile.open(QIODevice::ReadOnly)) {
			SwitchICData_CRC(firmFile, Firmware_Data, firmwareSize, uCrc16chk);
		}
		else {
			ACMessageBox::showWarning(this, tr("Warning"), tr("Open Update Firmware File failed"));
			ALOG_FATAL("Open Update Firmware File failed.", "CU", "--");
			return;
		}

		CalUpdateFWTime(true, strIPHop.toStdString());
		//固件升级文件比较大，可以直接放到0地址使用
		ALOG_INFO("Send Update Firmware File to %s:%d DDR", "CU", "FP", devIP.toStdString().c_str(), nHopNum);
		int ret = AngKMessageHandler::instance().Command_StoreDataToSSDorDDR(devIP.toStdString(), "FIBER2DDR", nHopNum, 0, 0x0, Firmware_Data, "", "FirmwareData");
		QThread::msleep(10);//提高UDP的稳定性，每发送一次命令包，停10ms
		nlohmann::json firmwareJson;

		firmwareJson["DDROffset"] = (QString("0x") + QString("%1").arg(0, 8, 16, QLatin1Char('0')).toUpper()).toStdString();
		firmwareJson["DataSize"] = (QString("0x") + QString("%1").arg(firmFile.size(), 8, 16, QLatin1Char('0')).toUpper()).toStdString();
		firmwareJson["CRC16"] = (QString("0x") + QString("%1").arg(uCrc16chk, 4, 16, QLatin1Char('0')).toUpper()).toStdString();
		firmwareJson["From"] = fromVer.toStdString();
		firmwareJson["To"] = toVer.toStdString();


		if (ret == 0)
		{
			//升级命令默认设置BPU0_SKT0，MU会根据CMD进行判断是否发送给BPU
			ret = AngKMessageHandler::instance().Command_RemoteDoPTCmd(devIP.toStdString(), nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_UpdateFw, ALL_MU, 8, QByteArray(firmwareJson.dump().c_str()));
			m_bUpdateProgress = true;
		}
		else {
			emit sgnUpdateFwStatus(devIP.toStdString(), nHopNum);
		}
	}

	void AngKMainFrame::onSlotLogWriteUI(QString text)
	{
		setLogText(text);
	}

	void AngKMainFrame::onSlotLinkStatusChanged(quint32 HopNum, quint16 LinkStatus, quint32 IsLastHop)
	{
		if (IsLastHop == 0)
			return;

		m_LinkHopNum = HopNum;
		//ALOG_INFO("onSlotLinkStatusChanged HopNum = %d", HopNum);
	}

	void AngKMainFrame::onSlotSetAllSelect(bool)
	{
		int selectNum = 0;
		for (auto prog : m_pProgWgtVec) {
			if (prog.second->GetProgramerCheck() || !prog.second->GetDevConnectState()) {
				selectNum++;
			}
		}

		//if (selectNum == 0)
		//	return;

		bool bCheck = false;
		if (selectNum == m_pProgWgtVec.size()) {
			bCheck = true;
		}
		ui->controlWidget->SetAllSelect(bCheck);
	}

	void AngKMainFrame::onSlotSendDrvPara(QString commonPara, QString selfPara, bool bLoad)
	{
		m_bLoadProjStatus = bLoad;

		std::map<std::string, DeviceStu> insertDev;
		AngKDeviceModel::instance().GetConnetDevMap(insertDev);
		for (auto iter : insertDev) {

			uint16_t sktEnNum = GetProgramSiteCheck(iter.second.strIP, iter.second.nHopNum);
			int ret = AngKMessageHandler::instance().Command_RemoteDoPTCmd(iter.second.strIP, iter.second.nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_SetDriverCommon, sktEnNum, 8, QByteArray(commonPara.toStdString().c_str()));
			stuCmdData setDriverSelf{ iter.second.strIP, (uint32_t)iter.second.nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_SetDriverSelfPara, sktEnNum ,8,QByteArray(selfPara.toStdString().c_str()) };
			m_pRemoteCmdManager->AddRemoteCmd(setDriverSelf);
		}
	}

	void AngKMainFrame::onSlotSupplyInstallDriver(QString ipHop, uint32_t sktEn)
	{
		std::string chipAlgoFileName = m_projDataset->getChipData().getJsonValue<std::string>("chipAlgoFile");
		chipAlgoFileName = chipAlgoFileName.substr(0, chipAlgoFileName.find_first_of("."));
		std::string chipMstFileName = m_projDataset->getChipData().getJsonValue<std::string>("chipMstkoFile");
		chipMstFileName = chipMstFileName.substr(0, chipMstFileName.find_first_of("."));
		QString drvFile = Utils::AngKPathResolve::localTempFolderPath() + QString::fromStdString(chipAlgoFileName) + ".adrv";
		QString mstFile = Utils::AngKPathResolve::localTempFolderPath() + QString::fromStdString(chipMstFileName) + ".adrv";

		std::string default;
		SendDriverCmd("", 0,drvFile, mstFile, default);
		m_supplyDriver = AngKDeviceModel::instance().GetConnetDevMapSize();
	}

	void AngKMainFrame::onSlotSplitterMoved(int nPos, int nIndex)
	{
		if(!m_log->isHidden())
			m_log->setFixedWidth(ui->operateWidget->width());
	}

	void AngKMainFrame::onSlotHandleEventInfo(QString strIPHop, std::string eventInfo)
	{
		try {
			nlohmann::json eventJson = nlohmann::json::parse(eventInfo);
			std::string eventName = eventJson["EName"];
			nlohmann::json eventData = eventJson["EData"];
			if (eventName == "JobStatusChange") {
				HandleEventJobStatusChange(strIPHop, eventJson["EData"]);
			}
			else if (eventName == "JobResult") {
				HandleEventJobResult(strIPHop, eventJson["EData"]);
			}
			else if (eventName == "SKTStatusChange") {
				HandleEventSKTStatusChange(strIPHop, eventJson["EData"]);
			}
			else if (eventName == "SKTAlarm") {
				HandleEventSKTAlarm(strIPHop, eventJson["EData"]);
			}
			else if (eventName == "DPS Alert") {
				HandleEventDPSAlert(strIPHop, eventJson["EData"]);
			}
			else if (eventName == "AnalyzeResult") {
				emit sgnHandleEventAnalyzeResult(eventData.dump());
			}
			else if (eventName == "AnalyzeInfo") {
				emit sgnHandleEventAnalyzeInfo(eventData.dump());
			}
			else if (eventName == "ExtCSDFetched") {
				emit sgnHandleEventExtCSDFetched(eventData.dump());
				emit sgnHandleEventTransmitExtCSDFetched(eventData.dump());
			}
			else if (eventName == "AnalyzeStatusChange") {
				emit sgnHandleEventAnalyzeStatusChange(eventData.dump());
			}
			else if (eventName == "UIDFetched") {
				emit sgnHandleEventUIDFetched(eventData.dump());
			}
			else if (eventName == "ChipIDFetched") {
				ALOG_INFO("Parse ChipIDFetched %s.", "CU", "--", eventData.dump().c_str());
				emit sgnHandleEventChipIDFetched(eventData.dump());
				emit sgnHandleEventTransmitChipIDFetched(eventData.dump());
				
			}
			else if (eventName == "ProgInitialized" || eventName == "Reboot") {
			}


			eventJson["ipHop"] = strIPHop.toStdString();
			EventLogger->SendEvent(eventJson.dump());
		}
		catch (const nlohmann::json::exception& e) {
			ALOG_FATAL("Parse eventJson failed : %s.", "CU", "--", e.what());
		}
	}

	void AngKMainFrame::onSlotQueryAdapterInfo(int nQueryMode, int nBPUEn, std::string strIP, int nHopNum)
	{
		nlohmann::json queryAdapterJson;
		queryAdapterJson["BPUEn"] = nBPUEn;

		uint16_t nCmdID = (uint16_t)eSubCmdID::SubCmd_MU_GetSktInfo;
		if (nQueryMode == 0) {
			nCmdID = (uint16_t)eSubCmdID::SubCmd_MU_GetSktInfoSimple;
		}

		AngKMessageHandler::instance().Command_RemoteDoPTCmd(strIP, nHopNum, 0, 0, nCmdID, ALL_MU, 8, QByteArray(queryAdapterJson.dump().c_str()));
	}


	void AngKMainFrame::onSlotCheckDevVFiles(QString strIP, int nHop, QString _projName) {

		QMap<QString, QPair<QString, ACProjManager*>> allProjInfo = m_pTaskManager->GetAllProjInfo();
		DeviceStu devCopy = ACDeviceManager::instance().getDevInfo(strIP, nHop);
		QString strIPHop = strIP + ":" + QString::number(nHop);

		for (auto& projInfo : allProjInfo.toStdMap()) {

			if (!projInfo.second.second->CheckEmmcFile(strIP, nHop)) {
				devCopy.ProgEnvReady = ProEnvStatus::SuccessButCurTskChanged;
				ACDeviceManager::instance().setDevInfo(strIP, nHop, devCopy);
				return;
			}
		}
		devCopy.ProgEnvReady = ProEnvStatus::Success;
		ACDeviceManager::instance().setDevInfo(strIP, nHop, devCopy);
	}

	void AngKMainFrame::onSlotDownloadProject(QString strIP, int nHop, QString _projName)
	{
		QMap<QString, QPair<QString, ACProjManager*>> allProjInfo = m_pTaskManager->GetAllProjInfo();
		DeviceStu devCopy = ACDeviceManager::instance().getDevInfo(strIP, nHop);


		QElapsedTimer timer;
		ProEnvStatus::Status nProgEnvReadyStatus = ProEnvStatus::Failed;
		QString strIPHop = strIP + ":" + QString::number(nHop);
		m_IPHop_TaskResultPromise[strIPHop] = std::promise<int>();
		for (auto& projInfo : allProjInfo.toStdMap()) {
			timer.restart();
			bool bOk;
			int RetCall = 0;
			emit sgnTaskInitChipInfo(projInfo.second.second->GetProjData()->getChipData());

			//临时赋值一下
			projMutex.lock();
			OpInfoList infoList = projInfo.second.second->GetProjData()->GetOperInfoList();
			m_projDataset->SetOperInfoList(infoList);
			m_mapDownloadSktEn[strIPHop] = projInfo.second.first.toInt(&bOk, 16);
			projMutex.unlock();
			//首先执行InstallFPGA，因为不同的驱动文件使用的版本不同
			{
				//std::string installFPGAJson;
				//RetCall = SendFPGACmd(strIP, nHop, Utils::AngKPathResolve::localFPGAFilePath("Normal_bitstream_mubpu_107A"), installFPGAJson);
				//if (RetCall != 0) {
				//	emit sgnProjectValue(-1);
				//	RecordTaskEvent(RetCall, _projName);
				//	break;
				//}

				//stuCmdData setInstallFPGA{ strIP.toStdString(), (uint32_t)nHop, 0, 0, (uint32_t)eSubCmdID::SubCmd_MU_InstallFPGA, (uint16_t)projInfo.second.first.toUInt(&bOk, 16) ,8, QByteArray(installFPGAJson.c_str()) };
				//m_pRemoteCmdManager->AddRemoteCmd(setInstallFPGA);
				//m_pRemoteCmdManager->DoCmd((uint32_t)eSubCmdID::SubCmd_MU_InstallFPGA, strIP.toStdString(), nHop);
				//return;

				//uint32_t selectBPU = Utils::AngKCommonTools::GetBPUCount(projInfo.second.first.toInt(&bOk, 16));
				//projSemaphore = new QSemaphore(selectBPU);
				//projSemaphore->acquire(selectBPU);
				////m_nRecv = selectBPU;
				//RetCall = WaitCmdExeQueueAvailable(1000 * 120, selectBPU);
				//if (RetCall != 0) {
				//	ALOG_FATAL("Execute %s WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", "Install FPGA", 3000 * selectBPU);
				//	RecordTaskEvent(RetCall, _projName);
				//	projSemaphore = nullptr;
				//	delete projSemaphore;
				//	emit sgnProjectValue(-1);
				//	break;
				//}
			}

			//安装驱动文件
			//projMutex.lock();
			std::string installDriverJson;
			RetCall = SendDriverCmd(strIP, nHop, projInfo.second.second->GetProjDataBuffer()->GetChipFile(deviceDriver), projInfo.second.second->GetProjDataBuffer()->GetChipFile(masterDriver), installDriverJson, projInfo.second.first.toInt(&bOk, 16));
			//projMutex.unlock();
			if (RetCall != 0) {
				emit sgnProjectValue(-1, strIP, nHop);
				RecordTaskEvent(strIP, nHop, RetCall, _projName);
				nProgEnvReadyStatus = ProEnvStatus::Failed;
				break;
			}

			emit sgnProjectValue(10, strIP, nHop);
			if (!installDriverJson.empty()) {
				RetCall = AngKMessageHandler::instance().Command_RemoteDoPTCmd(strIP.toStdString(), (uint32_t)nHop, 0, 0, (uint32_t)eSubCmdID::SubCmd_MU_InstallDriver, (uint16_t)projInfo.second.first.toUInt(&bOk, 16), 8, QByteArray(installDriverJson.c_str()));
			}

			uint32_t selectBPU = Utils::AngKCommonTools::GetBPUCount(projInfo.second.first.toInt(&bOk, 16));
			projMutex.lock();
			m_IPHop_projSemaphore[strIPHop] = new QSemaphore(selectBPU);
			projMutex.unlock();
			m_IPHop_projSemaphore[strIPHop]->acquire(selectBPU);
			RetCall = WaitCmdExeQueueAvailable(strIPHop, 4000 * selectBPU, selectBPU);
			if (RetCall != 0) {
				ALOG_FATAL("Device %s:%d Execute %s WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", strIP.toStdString().c_str(), nHop, "Install Driver", 4000 * selectBPU);
				RecordTaskEvent(strIP, nHop, RetCall, _projName);
				projMutex.lock();
				delete m_IPHop_projSemaphore[strIPHop];
				m_IPHop_projSemaphore[strIPHop] = nullptr;
				m_IPHop_projSemaphore.erase(strIPHop);
				projMutex.unlock();
				emit sgnProjectValue(-1, strIP, nHop);
				nProgEnvReadyStatus = ProEnvStatus::Failed;
				break;
			}

			emit sgnProjectValue(20, strIP, nHop);
			std::string ChipInfoJson;
			GetCurSelectICInfo(ChipInfoJson, projInfo.second.second->GetProjData()->getChipData());
			//stuCmdData setChipInfo{ strIP.toStdString(), (uint32_t)nHop, 0, 0, (uint32_t)eSubCmdID::SubCmd_MU_SetChipInfo, (uint16_t)projInfo.second.first.toUInt(&bOk, 16) ,8, QByteArray(ChipInfoJson.c_str()) };
			//m_pRemoteCmdManager->AddRemoteCmd(setChipInfo);
			//m_pRemoteCmdManager->DoCmd((uint32_t)eSubCmdID::SubCmd_MU_SetChipInfo, strIP.toStdString(), nHop);
			RetCall = AngKMessageHandler::instance().Command_RemoteDoPTCmd(strIP.toStdString(), (uint32_t)nHop, 0, 0, (uint32_t)eSubCmdID::SubCmd_MU_SetChipInfo, (uint16_t)projInfo.second.first.toUInt(&bOk, 16), 8, QByteArray(ChipInfoJson.c_str()));

			m_IPHop_projSemaphore[strIPHop]->acquire(selectBPU);
			RetCall = WaitCmdExeQueueAvailable(strIPHop, 3000 * selectBPU, selectBPU);
			if (RetCall != 0) {
				ALOG_FATAL("Device %s:%d Execute %s WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", strIP.toStdString().c_str(), nHop, "SetChipInfo", 3000 * selectBPU);
				RecordTaskEvent(strIP, nHop, RetCall, _projName);
				projMutex.lock();
				delete m_IPHop_projSemaphore[strIPHop];
				m_IPHop_projSemaphore[strIPHop] = nullptr;
				m_IPHop_projSemaphore.erase(strIPHop);
				projMutex.unlock();
				nProgEnvReadyStatus = ProEnvStatus::Failed;
				emit sgnProjectValue(-1, strIP, nHop);
				break;
			}
			emit sgnProjectValue(30, strIP, nHop);

			QString drvCommonJson;
			QString drvSelfJson;
			drvCommonJson = projInfo.second.second->GetProjData()->GetCommonDrvParaJson();

			//stuCmdData setComonDrvInfo{ strIP.toStdString(), (uint32_t)nHop, 0, 0, (uint32_t)eSubCmdID::SubCmd_MU_SetDriverCommon, (uint16_t)projInfo.second.first.toUInt(&bOk, 16) ,8, QByteArray(drvCommonJson.toStdString().c_str()) };
			//m_pRemoteCmdManager->AddRemoteCmd(setComonDrvInfo);
			//m_pRemoteCmdManager->DoCmd((uint32_t)eSubCmdID::SubCmd_MU_SetDriverCommon, strIP.toStdString(), nHop);
			RetCall = AngKMessageHandler::instance().Command_RemoteDoPTCmd(strIP.toStdString(), (uint32_t)nHop, 0, 0, (uint32_t)eSubCmdID::SubCmd_MU_SetDriverCommon, (uint16_t)projInfo.second.first.toUInt(&bOk, 16), 8, QByteArray(drvCommonJson.toStdString().c_str()));

			m_IPHop_projSemaphore[strIPHop]->acquire(selectBPU);
			RetCall = WaitCmdExeQueueAvailable(strIPHop, 2000 * selectBPU, selectBPU);
			if (RetCall != 0) {
				ALOG_FATAL("Device %s:%d Execute %s WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", strIP.toStdString().c_str(), nHop, "SetDriverCommon", 2000 * selectBPU);
				RecordTaskEvent(strIP, nHop, RetCall, _projName);
				projMutex.lock();
				delete m_IPHop_projSemaphore[strIPHop];
				m_IPHop_projSemaphore[strIPHop] = nullptr;
				m_IPHop_projSemaphore.erase(strIPHop);
				projMutex.unlock();
				nProgEnvReadyStatus = ProEnvStatus::Failed;
				emit sgnProjectValue(-1, strIP, nHop);
				break;
			}
			emit sgnProjectValue(40, strIP, nHop);

			drvSelfJson = projInfo.second.second->GetProjData()->GetSelfDrvParaJson();

			//stuCmdData setSelfDrvInfo{ strIP.toStdString(), (uint32_t)nHop, 0, 0, (uint32_t)eSubCmdID::SubCmd_MU_SetDriverSelfPara, (uint16_t)projInfo.second.first.toUInt(&bOk, 16) ,8, QByteArray(drvSelfJson.toStdString().c_str()) };
			//m_pRemoteCmdManager->AddRemoteCmd(setSelfDrvInfo);
			//m_pRemoteCmdManager->DoCmd((uint32_t)eSubCmdID::SubCmd_MU_SetDriverSelfPara, strIP.toStdString(), nHop);
			RetCall = AngKMessageHandler::instance().Command_RemoteDoPTCmd(strIP.toStdString(), (uint32_t)nHop, 0, 0, (uint32_t)eSubCmdID::SubCmd_MU_SetDriverSelfPara, (uint16_t)projInfo.second.first.toUInt(&bOk, 16), 8, QByteArray(drvSelfJson.toStdString().c_str()));

			m_IPHop_projSemaphore[strIPHop]->acquire(selectBPU);
			RetCall = WaitCmdExeQueueAvailable(strIPHop, 2000 * selectBPU, selectBPU);
			if (RetCall != 0) {
				ALOG_FATAL("Device %s:%d Execute %s WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", strIP.toStdString().c_str(), nHop, "SetDriverSelfPara", 2000 * selectBPU);
				RecordTaskEvent(strIP, nHop, RetCall, _projName);
				projMutex.lock();
				delete m_IPHop_projSemaphore[strIPHop];
				m_IPHop_projSemaphore[strIPHop] = nullptr;
				m_IPHop_projSemaphore.erase(strIPHop);
				projMutex.unlock();
				nProgEnvReadyStatus = ProEnvStatus::Failed;
				emit sgnProjectValue(-1, strIP, nHop);
				break;
			}
			emit sgnProjectValue(50, strIP, nHop);

			QByteArray _bufCheckAttr = projInfo.second.second->GetBufferCheckBPUAttribute();
			//stuCmdData setbufCheckAttr{ strIP.toStdString(), (uint32_t)nHop, 0, 0, (uint32_t)eSubCmdID::SubCmd_MU_SetBPUAttribute, (uint16_t)projInfo.second.first.toUInt(&bOk, 16) ,8, _bufCheckAttr };
			//m_pRemoteCmdManager->AddRemoteCmd(setbufCheckAttr);
			//m_pRemoteCmdManager->DoCmd((uint32_t)eSubCmdID::SubCmd_MU_SetBPUAttribute, strIP.toStdString(), nHop);
			RetCall = AngKMessageHandler::instance().Command_RemoteDoPTCmd(strIP.toStdString(), (uint32_t)nHop, 0, 0, (uint32_t)eSubCmdID::SubCmd_MU_SetBPUAttribute, (uint16_t)projInfo.second.first.toUInt(&bOk, 16), 8, _bufCheckAttr);

			m_IPHop_projSemaphore[strIPHop]->acquire(selectBPU);
			RetCall = WaitCmdExeQueueAvailable(strIPHop, 2000 * selectBPU, selectBPU);
			if (RetCall != 0) {
				ALOG_FATAL("Device %s:%d Execute %s WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", strIP.toStdString().c_str(), nHop, "SetBPUAttribute", 2000 * selectBPU);
				RecordTaskEvent(strIP, nHop, RetCall, _projName);
				projMutex.lock();
				delete m_IPHop_projSemaphore[strIPHop];
				m_IPHop_projSemaphore[strIPHop] = nullptr;
				m_IPHop_projSemaphore.erase(strIPHop);
				projMutex.unlock();
				nProgEnvReadyStatus = ProEnvStatus::Failed;
				emit sgnProjectValue(-1, strIP, nHop);
				break;
			}
			emit sgnProjectValue(60, strIP, nHop);

			ACSSDProjManager ssdProjManager(strIP, nHop);

			connect(&ssdProjManager, &ACSSDProjManager::sgnUpdateProjWriteProcess, this, &AngKMainFrame::sgnProjectValue, Qt::QueuedConnection);



			QString tmpJsonStr = "{\"CurPartitionTableHeadAddr\":\"0x00\"}";
			AngKMessageHandler::instance().Command_RemoteDoPTCmd(strIP.toStdString(), nHop, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_SetPartitionTableHeadAddr
				, (uint16_t)projInfo.second.first.toUInt(&bOk, 16), 8, tmpJsonStr.toUtf8());

			m_IPHop_projSemaphore[strIPHop]->acquire(selectBPU);
			m_IPHop_projCommResultCode[strIPHop].clear();
			RetCall = WaitCmdExeQueueAvailable(strIPHop, 2000 * selectBPU, selectBPU);
			if (RetCall != 0) {
				ALOG_FATAL("Device %s:%d Execute %s WaitCmdAvailableQueue Timeout at Start, Timemoutms:%d.", "CU", "--", strIP.toStdString().c_str(), nHop, "CurPartitionTableHeadAddr", 2000 * selectBPU);
				RecordTaskEvent(strIP, nHop, RetCall, _projName);
				projMutex.lock();
				delete m_IPHop_projSemaphore[strIPHop];
				m_IPHop_projSemaphore[strIPHop] = nullptr;
				m_IPHop_projSemaphore.erase(strIPHop);
				projMutex.unlock();
				nProgEnvReadyStatus = ProEnvStatus::Failed;
				emit sgnProjectValue(-1, strIP, nHop);
				break;
			}
			else {
				bool bBPUDriverCompatible = true;
				for (auto it : m_IPHop_projCommResultCode[strIPHop]) {
					//if (it != 0) {
					//	ALOG_FATAL("Device %s:%d BPU driver version incompatible", "CU", "--", strIP.toStdString().c_str(), nHop);
					//	RecordTaskEvent(strIP, nHop, -1, _projName);
					//	projMutex.lock();
					//	delete m_IPHop_projSemaphore[strIPHop];
					//	m_IPHop_projSemaphore[strIPHop] = nullptr;
					//	m_IPHop_projSemaphore.erase(strIPHop);
					//	projMutex.unlock();
					//	nProgEnvReadyStatus = ProEnvStatus::Failed;
					//	emit sgnProjectValue(-1, strIP, nHop);
					//	bBPUDriverCompatible = false;
					//	break;
					//}
				}
				if (!bBPUDriverCompatible)
					break;
			}

			//RetCall = projInfo.second.second->LoadeMMCFile(strIP, nHop);
			bool downloadRet = ssdProjManager.downLoadProject(projInfo.second.second);
			if (!downloadRet) {
				RecordTaskEvent(strIP, nHop, -1, _projName);
				projMutex.lock();
				delete m_IPHop_projSemaphore[strIPHop];
				m_IPHop_projSemaphore[strIPHop] = nullptr;
				m_IPHop_projSemaphore.erase(strIPHop);
				projMutex.unlock();
				nProgEnvReadyStatus = ProEnvStatus::Failed;
				emit sgnProjectValue(-1, strIP, nHop);
				disconnect(&ssdProjManager, &ACSSDProjManager::sgnUpdateProjWriteProcess, this, &AngKMainFrame::sgnProjectValue);
				break;
			}
			//ssdProjManager.createSSDJsonFile();

			projMutex.lock();
			delete m_IPHop_projSemaphore[strIPHop];
			m_IPHop_projSemaphore[strIPHop] = nullptr;
			m_IPHop_projSemaphore.erase(strIPHop);
			projMutex.unlock();
			emit sgnProjectValue(100, strIP, nHop);

			disconnect(&ssdProjManager, &ACSSDProjManager::sgnUpdateProjWriteProcess, this, &AngKMainFrame::sgnProjectValue);

			tmpJsonStr = "{\"CurPartitionTableHeadAddr\":\"0x" + QString::number(ssdProjManager.getCurWritePartitionHeadAddr(),16) + "\"}";
			AngKMessageHandler::instance().Command_RemoteDoPTCmd(strIP.toStdString(), nHop, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_SetPartitionTableHeadAddr
				, (uint16_t)projInfo.second.first.toUInt(&bOk, 16), 8, tmpJsonStr.toUtf8());

			nProgEnvReadyStatus = ProEnvStatus::Success;

			//emit sgnTaskSetProjBindBPU(projInfo.second.first.toInt(&bOk, 16), strIP, nHop, projInfo.second.second->GetProjData()->getChipData().getJsonValue<std::string>("chipAdapterID"));

			RecordTaskEvent(strIP, nHop, RetCall, _projName);

			//每次创建工程时，重新添加设置多工程
			projMutex.lock();
			ACReportRecord& GReporter = GetReporter();
			GReporter.SetProjName(projInfo.first);
			GReporter.AddSite(projInfo.first, &devCopy, 8);
			SetProjReport(projInfo.second.second);
			projMutex.unlock();

			qint64 elapsedTime = timer.elapsed();  // 获取经过的时间（毫秒）
			// 将毫秒转换成秒
			qint64 totalSeconds = elapsedTime / 1000;
			// 计算小时数
			qint64 hours = totalSeconds / 3600;
			// 计算剩余的秒数（去除小时后的秒数）
			qint64 remainingSeconds = totalSeconds % 3600;
			// 获取完整的分钟数
			int minutes = static_cast<int>(remainingSeconds / 60);
			// 获取剩余的秒数
			int seconds = static_cast<int>(remainingSeconds % 60);
			ALOG_INFO("Device %s:%d Load Project File : %s Total Time : %d hours %d min %d seconds.", "CU", "--", strIP.toStdString().c_str(), nHop, projInfo.first.toStdString().c_str(), hours, minutes, seconds);
		}

		devCopy.ProgEnvReady = nProgEnvReadyStatus;
		ACDeviceManager::instance().setDevInfo(strIP, nHop, devCopy);
	}

	void AngKMainFrame::onSlotProgrammgerTest(int nCheckEnable)
	{
		QVector<DeviceStu> vecDevice = ACDeviceManager::instance().getAllDevInfo();

		for (auto devInfo : vecDevice) {
			nlohmann::json selfTest;
			selfTest["Test Items"] = QString("0x%1").arg(nCheckEnable, 8, 16, QLatin1Char('0')).toStdString();
			AngKMessageHandler::instance().Command_RemoteDoPTCmd(devInfo.strIP, devInfo.nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_ProgrammerSelfTest, ALL_MU, 8, QByteArray(selfTest.dump().c_str()));
		}
	}

	void AngKMainFrame::onSlotRecvDoCmdResult(QString strIP, uint32_t nHopNum, uint32_t resultCode, uint32_t nBPUID)
	{
		uint32_t nTrueSktEn = 0x3 << nBPUID * 2;
		if (resultCode != 0) {
			for (auto obj : ui->scrollAreaWidgetContents->children()) {
				if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
					AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
					DeviceStu proDev = pro->GetDevInfo();
					if (proDev.strIP == strIP.toStdString() && proDev.nHopNum == nHopNum) {
						//pro->SetSiteStatus(nTrueSktEn, SiteStatus::Failed);
						MarkBurnSiteMap(strIP + ":" + QString::number(nHopNum), nBPUID, 0);
						MarkBurnSiteMap(strIP + ":" + QString::number(nHopNum), nBPUID, 1);

					}
				}
			}
		}
	}

	void AngKMainFrame::onSlotRecvDoCustom(QString strIPHop, uint16_t BPUID, QByteArray customData)
	{
		// 将 QByteArray 转换为 16 进制字符串
		QString hexString = customData.toHex().toUpper(); // 转换为大写，更常见
		// 可以在控制台打印 16 进制字符串
		QByteArray byteArray = hexString.toLocal8Bit();
		const char* hexStringCStr = byteArray.data();
		if (customData.size() < 12) { // At least need 4 bytes tagid + 4 bytes length + 4 bytes crc16
			ALOG_ERROR("Invalid custom info size: %d", "CU", "--", customData.size());
			return;
		}
		uint32_t tagId = *reinterpret_cast<const uint32_t*>(customData.constData());
		if ((eCustomTagID)tagId == eCustomTagID::SubCmd_CustomTag_MT422) {
			CustomMessageHandler::instance()->OnRecvDoCustom(strIPHop, BPUID, customData);
		}
		else {
			ALOG_ERROR("Unsupported custom tag id: %08X", "CU", "--", tagId);
		}
	}

	void AngKMainFrame::onSlotAutomatic_ChipInPlace(int siteIndex, uint32_t slotEn, std::string strSiteSn)
	{
		slotEn = slotEn & 0xFFFF;  // 只保留低16位;
		ALOG_INFO("ChipInPlace,slotEn=0x%X", "CU", "--", slotEn);
		for (auto obj : ui->scrollAreaWidgetContents->children()) {
			if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
				AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
				if (pro) {

					auto devInfo = pro->GetDevInfo();
					if (g_AppMode == ConnectType::Demo && strSiteSn == devInfo.tMainBoardInfo.strHardwareSN){
						int softSktEn = Utils::AngKCommonTools::SwapSKTIdx_Auto2Soft(slotEn, 16);
						
						qDebug() << "999999" << slotEn;
						qDebug() << "1000000" << softSktEn;
						BurnRecord stuRecord;
						m_mapAutoRecord.insert(make_pair(pro->GetIpHop().toStdString(), std::pair<int, BurnRecord>(slotEn, stuRecord)));

						for (int i = 0; i < 16; ++i) {
							if ((0x1 << i) && slotEn) {
								int nBpuID = i / 2;
								int SktIdx = i % 2;

								ui->reportWidget->AddOutputNum();
								CheckAutoBurnRecord(QString::fromStdString(devInfo.strIP), devInfo.nHopNum, nBpuID, SktIdx, "4000");
							}
						}
					}
					else {
						if (strSiteSn == devInfo.tMainBoardInfo.strHardwareSN) {

							//判断烧录前的task是否都已经完成
							if (devInfo.ProgEnvReady == ProEnvStatus::Failed) {
								ACMessageBox::showError(this, tr("Error"), tr("Device %1:%2 load task failed, can't execute programing command.").arg(QString::fromStdString(devInfo.strIP)).arg(QString::number(devInfo.nHopNum)));
								emit sgnAutomicOver();
								ALOG_FATAL("Auto Program Stop. Reason: Device %s:%d load task failed, can't execute programing command.", "CU", "--", devInfo.strIP, devInfo.nHopNum);
								return;
							}

							if (devInfo.ProgEnvReady == ProEnvStatus::Idle) {
								ACMessageBox::showError(this, tr("Error"), tr("Device %1:%2 not loaded task, unable to execute programing command.").arg(QString::fromStdString(devInfo.strIP)).arg(QString::number(devInfo.nHopNum)));
								emit sgnAutomicOver();
								ALOG_FATAL("Auto Program Stop. Reason: Device %s:%d not loaded task, unable to execute programing command.", "CU", "--", devInfo.strIP, devInfo.nHopNum);
								return;
							}


							if (devInfo.ProgEnvReady == ProEnvStatus::SuccessButCurTskChanged) {
								ACMessageBox::showWarning(this, tr("Warn"), tr("Device %1:%2 task is different with current task, needs to be download again.").arg(QString::fromStdString(devInfo.strIP)).arg(QString::number(devInfo.nHopNum)));
								emit sgnAutomicOver();
								ALOG_FATAL("Auto Program Stop. Reason: Device %s:%d task is different with current task, unable to execute programing command.", "CU", "--", devInfo.strIP, devInfo.nHopNum);
								return;
							}
							//序列号能匹配上，则直接进行烧录
							std::string docmdSeqJson;
							QStringList processList;
							std::string chipDataName = m_projDataset->getChipData().getJsonValue<std::string>("chipName");
							std::string curManufacture = m_projDataset->getChipData().getJsonValue<std::string>("manufacture");
							ExecuteBurnCommand(docmdSeqJson, processList, (int)OperationTagType::Program);
							
							// 关键区别：自动模式的sktEn来自于slotEn参数
							int softSktEn = Utils::AngKCommonTools::SwapSKTIdx_Auto2Soft(slotEn, 16);
							
							// UUID下发逻辑，与手动模式的核心处理方式保持一致
							if ((chipDataName == "XSA300D" && curManufacture == "XT") ||
								(chipDataName == "MSI270" && curManufacture == "XT") ||
								(chipDataName == "XSG300D" && curManufacture == "XT")) {
								ALOG_INFO("Auto Mode: Start UUID distribution for chip %s, softSktEn=0x%X,slotEn=0x%X", "CU", "--", chipDataName.c_str(), softSktEn, slotEn);
								// 遍历16个skt
								for (int skt = 0; skt < 16; ++skt)
								{
									// 检查当前skt是否在本次事件的slotEn中
									if (slotEn & (1u << skt))
									{
										std::string uid = realGenerator->getUID();
										realGenerator->setUIDResult(uid, 1);
										// 构造只包含当前skt使能的数值
										uint32_t singleSktNum = 1u << skt;
										ALOG_INFO("Auto Mode: Sending UUID for SKT[%d], UUID=%s, singleSktNum=0x%X, IP=%s, Hop=%d", 
											"CU", "--", skt, uid.c_str(), singleSktNum, devInfo.strIP.c_str(), devInfo.nHopNum);
										AngKMessageHandler::instance().Command_RemoteDoPTCmd(
											devInfo.strIP,
											devInfo.nHopNum,
											0,
											0,
											(uint16_t)eSubCmdID::SubCmd_MU_DoCustom,
											singleSktNum,
											8,
											QByteArray(uid.c_str())
										);
									}
								}
								ALOG_INFO("Auto Mode: UUID distribution completed", "CU", "--");
							}
							ui->toolWidget->SetButtonEnable(OperationTagType::Program);
							if (!docmdSeqJson.empty()) {
								ALOG_INFO("Auto Mode: Sending DoCmdSequence, slotEn=0x%X, IP=%s, Hop=%d", 
									"CU", "--", slotEn, devInfo.strIP.c_str(), devInfo.nHopNum);
								// DoCmdSequence命令使用本次事件完整的slotEn
								AngKMessageHandler::instance().Command_RemoteDoPTCmd(devInfo.strIP, devInfo.nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_DoCmdSequence
									, slotEn, 8, QByteArray(docmdSeqJson.c_str()));
								m_nRecordDocmdTime = QDateTime::currentDateTime().toTime_t();
								for (auto obj : ui->scrollAreaWidgetContents->children()) {
									if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
										AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
										DeviceStu proDev = pro->GetDevInfo();
										if (proDev.strIP == devInfo.strIP && proDev.nHopNum == devInfo.nHopNum) {
											pro->RestSiteStatus();
											pro->SetSiteStatus(slotEn, SiteStatus::Busy);
										}
									}
								}

								BurnRecord stuRecord;
								m_mapAutoRecord.insert(make_pair(pro->GetIpHop().toStdString(), std::pair<int, BurnRecord>(slotEn, stuRecord)));
							}
							else {
								ALOG_ERROR("DoCmdSequence Json is Null，Chip In Place to Execute Burn failed.", "CU", "--");
							}
						}
					}
				}
			}
		}
	}

	void AngKMainFrame::onSlotModifyDeviceInfo(std::string strInfo, std::string strIPHop)
	{
		if (m_mapDownloadSktEn[QString::fromStdString(strIPHop)] == 0) {
			return;
		}

		QStringList IPHopList = QString::fromStdString(strIPHop).split(":");
		try {
			ClearBPUChcekState(IPHopList[0].toStdString(), IPHopList[1].toInt());
			nlohmann::json getSktInfoJson = nlohmann::json::parse(strInfo);
			DeviceStu devShowInfo = ACDeviceManager::instance().getDevInfo(IPHopList[0], IPHopList[1].toInt());
			int nBPUCount = getSktInfoJson["BPUInfo"].size();
			for (int i = 0; i < nBPUCount; ++i) {
				nlohmann::json BPUInfoJson = getSktInfoJson["BPUInfo"][i];
				int nBPUIdx = BPUInfoJson["BPUIdx"];
				nlohmann::json SKTInfoJson = BPUInfoJson["SKTInfo"];
				std::string strUID = SKTInfoJson["UID"];
				std::string strID = SKTInfoJson["ID"];
				int nLifeCycleCnt = SKTInfoJson["LifeCycleCnt"];
				SingleBPU sBPUInfo;
				sBPUInfo.idx = nBPUIdx;
				sBPUInfo.SktCnt = devShowInfo.nSingleBpuSiteNum;
				sBPUInfo.AdapterID = strID;
				devShowInfo.bpuInfoArr.push_back(sBPUInfo);
				ACDeviceManager::instance().setDevInfo(IPHopList[0], IPHopList[1].toInt(), devShowInfo);

				for (auto obj : ui->scrollAreaWidgetContents->children()) {
					if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
						AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
						if (pro) {
							auto devInfo = pro->GetDevInfo();
							if (devInfo.strIP == IPHopList[0].toStdString() && devInfo.nHopNum == IPHopList[1].toInt())
							{
								for (int bpuIdx = 0; bpuIdx < 2; ++bpuIdx) {
									uint32_t nTrueSktEn = ((0x1 << (nBPUIdx * 2)) << bpuIdx) & m_mapDownloadSktEn[QString::fromStdString(strIPHop)];
									pro->SetBPUCntState(nTrueSktEn, nLifeCycleCnt, strUID);
									pro->SetProjBindBPUState(nTrueSktEn, strID);
								}
							}
						}
					}
				}
			}
		}
		catch (const nlohmann::json::exception& e) {
			ALOG_FATAL("return GetSktInfo Json : %s.", "CU", "--", strInfo.c_str());
			//ALOG_FATAL("return GetSktInfo Json parse failed : %s.", "CU", "--", e.what());
		}
	}

	void AngKMainFrame::onSlotAutoTellDevReady(int nPassLot)
	{
		m_mapAutoRecord.clear();
		auto autoPlugin = ACAutomaticManager::instance()->GetAutomaticPlugin();
		if (autoPlugin == nullptr)
		{
			ALOG_FATAL("Get Automatic Plugin failed.", "CU", "--");
			return;
		}

		nlohmann::json readyJson, ProjInfo;
		ProjInfo["AdapterNum"] = 16;
		int allExpectNum = 0;
		for (auto obj : ui->scrollAreaWidgetContents->children()) {
			if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
				AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
				if (pro) {
					uint32_t nSktRdy = pro->GetSitesChecked(allExpectNum);
					qDebug() << "888888" << nSktRdy;
					uint32_t nAutoSktRdy = Utils::AngKCommonTools::SwapSKTIdx_Soft2Auto(nSktRdy, 16);
					auto devInfo = pro->GetDevInfo();
					nlohmann::json SiteReady;
					SiteReady["SiteSN"] = devInfo.tMainBoardInfo.strHardwareSN;
					SiteReady["SiteAlias"] = devInfo.strSiteAlias;
					SiteReady["SiteEnvRdy"] = 1;
					SiteReady["SKTRdy"] = QString("0x%1").arg(nAutoSktRdy, 4, 16, QLatin1Char('0')).toStdString();//"0xff";
					qDebug() << "7777777" << QString("0x%1").arg(nAutoSktRdy, 4, 16, QLatin1Char('0'));
					std::string test = SiteReady.dump();
					readyJson["SiteReady"].push_back(SiteReady);
				}
			}
		}
		readyJson["ProjInfo"] = ProjInfo;
		std::string test = readyJson.dump();

		if (autoPlugin->TellDevReady(readyJson.dump())) {
			emit sgnAutomicOver();	// 和自动机通信失败
			bAutoConnect = false;
			ALOG_FATAL("Communction with automatic failed!", "CU", "--");
		}
		else
			bAutoConnect = true;
		ui->reportWidget->SetExpectSlot(nPassLot);
	}

	void AngKMainFrame::onSlotSetProgress(QString recvIP, int HopNum, int Current, int Total, int BPUID, std::string fromDev, std::string cmdName)
	{
		std::string percentProgress = Utils::AngKCommonTools::calculatePercentage(Current, Total);
		if (percentProgress != "Invalid operation") {
			ALOG_DEBUG("Recv %s Message from %s:%d, Current Progress:%s.", fromDev.c_str(), "CU", cmdName.c_str(), recvIP.toStdString().c_str(), HopNum, percentProgress.c_str());
		}
		else {
			ALOG_FATAL("Recv %s Message from %s:%d, Total Invalid.", fromDev.c_str(), "CU", cmdName.c_str(), recvIP.toStdString().c_str(), HopNum);
		}

		float nValue = (static_cast<float>(Current) / Total) * 100;
		if (m_bUpdateProgress)
		{
			emit sgnUpdateFPGAValue(recvIP.toStdString(), HopNum, nValue);
		}
		else if (m_bAnalyzeProgress)
		{
			emit sgnUpdateAnalyzeValue(recvIP.toStdString(), HopNum, nValue);
		}
		else
		{
			//烧录进度
			for (auto obj : ui->scrollAreaWidgetContents->children()) {
				if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
					AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
					DeviceStu proDev = pro->GetDevInfo();
					if (proDev.strIP == recvIP.toStdString() && proDev.nHopNum == HopNum) {
						//当前单个AG06可以这样，不涉及串联
						float nValue = (static_cast<float>(Current) / Total) * 100;
						pro->setBPUProgress(BPUID, nValue);
					}
				}
			}

			ui->reportWidget->UpdateCurrentProgress();
		}
	}

	void AngKMainFrame::onSlotSetUpdateFw(QString recvIP, uint32_t HopNum, uint32_t ResultCode, QByteArray RespDataBytes, uint32_t RespDataSize)
	{
		QString strIPHop = recvIP + ":" + QString::number(HopNum);
		bool bSuccess = true;
		if (ResultCode != 0) {
			QString errorMsg = Utils::AngKCommonTools::TranslateErrorMsg(ResultCode);
			ALOG_FATAL("Update %s:%d Firmware Failed, ResultCode = %d, Fail reason: %s.", "MU", "CU", recvIP.toStdString().c_str(), HopNum, ResultCode, errorMsg.toStdString().c_str());
			emit sgnUpdateFwStatus(recvIP.toStdString(), HopNum);
			bSuccess = false;
		}
		else {
			ALOG_INFO("Update %s:%d Firmware Successfully!", "MU", "CU", recvIP.toStdString().c_str(), HopNum);

			m_bUpdateProgress = false;

			ALOG_INFO("Start reboot %s:%d Dev.", "CU", "MU", recvIP.toStdString().c_str(), HopNum);
			//m_pRemoteCmdManager->DoCmd((uint32_t)eSubCmdID::SubCmd_MU_RebootMU, recvIP.toStdString().c_str(), HopNum);
			nlohmann::json rebootMUjson;
			rebootMUjson["ResetCommand"] = 0;
			rebootMUjson["DelayTime"] = 1000;

			AngKMessageHandler::instance().Command_RemoteDoPTCmd(recvIP.toStdString(), HopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_RebootMU, ALL_MU, 8, QByteArray(rebootMUjson.dump().c_str()));

			if (--AngKUpdateFirmware::m_UpdateFwNum == 0) {
				QTimer::singleShot(10000, []() {
					qApp->exit(MessageType::MESSAGE_RESTART); 
				});
				ACMessageBox::showInformation(this, "Restart", "The software will restart...", ACMessageBox::ACMsgButton::MSG_NO_BTN);
			}


		}
		CalUpdateFWTime(false, strIPHop.toStdString(), bSuccess);

		std::string strInfo = std::string(RespDataBytes.constData(), RespDataSize);
		std::string fromVer, toVer;
		try {
			nlohmann::json updateFWJson = nlohmann::json::parse(strInfo);
			fromVer = updateFWJson["From"];
			toVer = updateFWJson["To"];
		}
		catch (const nlohmann::json::exception& e) {
			ALOG_FATAL("return updateFW Json parse failed : %s.", "CU", "--", e.what());
		}
		nlohmann::json EDataJson;
		EDataJson["From"] = fromVer;
		EDataJson["To"] = toVer;
		EDataJson["RetCode"] = ResultCode;
		EDataJson["RetInfo"] = ResultCode != 0 ? "Update Failed" : "Update Success";
		EventLogger->SendEvent(EventBuilder->GetFWUpdate(EDataJson));
	}

	void AngKMainFrame::onSlotDevOffLine(QString ipStr, int hop)
	{
		QString tmpIpHop = ipStr + ":" + QString::number(hop);
		for (auto obj : ui->scrollAreaWidgetContents->children()) {
			if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
				AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
				if (pro && pro->GetIpHop() == tmpIpHop) {
					ClearBPUChcekState(ipStr.toStdString(), hop);
					pro->SetDevConnectState(false);
				}
			}
		}

		MarkBurnSiteMap(tmpIpHop);


		//std::thread testthread([this](QString ipStr, int hop) {
		//	DeviceStu devStu = ACDeviceManager::instance().getDevInfo(ipStr, hop);
		//	int retryHeart = AngKGlobalInstance::ReadValue("DeviceComm", "Retransmission").toInt();
		//	while (!devStu.bOnLine && retryHeart > 0) {
		//		retryHeart--;
		//		AngKMessageHandler::instance().Command_LinkScan(ipStr.toStdString(), hop);
		//		Sleep(5000);
		//	}

		//	//if (!devStu.bOnLine)
		//	//{
		//	//	ACDeviceManager::instance().delDev(ipStr, hop);
		//	//}

		//}, ipStr, hop);

		//testthread.detach();
	}

	void AngKMainFrame::onSlotDevOnLine(QString ipStr, int hop)
	{
		QString tmpIpHop = ipStr + ":" + QString::number(hop);
		for (auto obj : ui->scrollAreaWidgetContents->children()) {
			if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
				AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
				if (pro && pro->GetIpHop() == tmpIpHop) {
					pro->SetDevConnectState(true);
				}
			}
		}
	}

	void AngKMainFrame::onSlotTaskInitChipInfo(ChipDataJsonSerial chipJson)
	{
		InitChipInfo(chipJson);
	}

	void AngKMainFrame::onSlotTaskSetProjBindBPU(int nSktEn, QString strIP, int nHop, std::string strAdapterID)
	{
		SetProjBindBPU(nSktEn, strIP, nHop, strAdapterID);
	}

	void AngKMainFrame::onSlotHandleReportRequested(QTcpSocket* client)
	{
		emit sgnAutomicOver();
		bAutoConnect = false;
	}

	void AngKMainFrame::onSlotClearLotData(int expect) {
		ResetALLSiteStatus();
		ui->reportWidget->ResetExpectAndOutput();
		ui->reportWidget->SetExpectSlot(expect);
	}

	void AngKMainFrame::onSlotUpdateTaskInfo()
	{
		QMap<QString, QPair<QString, ACProjManager*>> allProjInfo = m_pTaskManager->GetAllProjInfo();

		ui->propetryWidget->SetProjManagerInfo(allProjInfo);
		ui->propetryWidget->UpdatePropertyUI();
	}

	void AngKMainFrame::onSlotRequestSKTInfo(std::string strIP, int nHop)
	{
		nlohmann::json getSktInfojson;
		getSktInfojson["BPUEn"] = 0xFF;
		AngKMessageHandler::instance().Command_RemoteDoPTCmd(strIP, nHop, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_GetSKTEnable, ALL_MU, 8, QByteArray(getSktInfojson.dump().c_str()));
	}

	/**
	 * @brief 更新SKT使能状态，收集3秒内的结果后统一发送
	 * @param sktInfoJson SKT信息JSON字符串
	 * @param recvIP 接收IP地址
	 * @param nHop 跳数
	 */
	void AngKMainFrame::onSlotUpdateSKTEnable(std::string sktInfoJson, std::string recvIP, int nHop)
	{
		QString strIPHop = QString::fromStdString(recvIP) + ":" + QString::number(nHop);
		if (m_mapDownloadSktEn[strIPHop] == 0) {
			return;
		}
		
		// 解析JSON提取BPUEn的值
		nlohmann::json getSktInfoJson = nlohmann::json::parse(sktInfoJson);
		int nDevSktEnable = getSktInfoJson["BPUEn"];

		// 构造拼接字符串
		QString result = QString("BPUEn:%1 recvIP:%2 nHop:%3")
			.arg(nDevSktEnable)
			.arg(QString::fromStdString(recvIP))
			.arg(nHop);
		
		// 将结果添加到收集列表中
		m_sktResultList.append(result);
		
		// 如果定时器还未启动，则启动定时器
		if (!m_bSKTTimerStarted) {
			m_bSKTTimerStarted = true;
			m_pSKTResultTimer->start();
		}
		
		//ALOG_INFO("return GetSKTEnable Json : %s.", "CU", "--", sktInfoJson.c_str());
		QStringList IPHopList = strIPHop.split(":");
		try {
			ClearBPUChcekState(IPHopList[0].toStdString(), IPHopList[1].toInt());
			nlohmann::json getSktInfoJson = nlohmann::json::parse(sktInfoJson);
			int nDevSktEnable = getSktInfoJson["BPUEn"];
			for (auto obj : ui->scrollAreaWidgetContents->children()) {
				if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
					AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
					if (pro) {
						auto devInfo = pro->GetDevInfo();
						if (devInfo.strIP == IPHopList[0].toStdString() && devInfo.nHopNum == IPHopList[1].toInt())
						{
							pro->SetProjBindBPUState(nDevSktEnable);
						}
					}
				}
			}
		}
		catch (const nlohmann::json::exception& e) {
			ALOG_FATAL("return GetSKTEnable Json : %s.", "CU", "--", sktInfoJson.c_str());
		}
		//ALOG_FATAL("return GetSKTEnable Json : %s.", "CU", "--", sktInfoJson.c_str());
	}

	void AngKMainFrame::GetCurSelectICInfo(std::string& ICInfoJson, ChipDataJsonSerial dataJson)
	{
		//if (nullptr == m_projDataset)
		//	return;

		//ChipDataJsonSerial dataJson = m_projDataset->getChipData();
		try {
			nlohmann::json ICJson;

			ICJson["ChipName"] = dataJson.getJsonValue<std::string>("chipName");
			ICJson["Manufacture"] = dataJson.getJsonValue<std::string>("manufacture");
			ICJson["Package"] = dataJson.getJsonValue<std::string>("chipPackage");
			ICJson["Type"] = dataJson.getJsonValue<std::string>("chipType");
			try {
				ICJson["ID"] = QString::number(dataJson.getJsonValue<ulong>("chipId")).toStdString();
			} catch (...) {
				ICJson["ID"] = dataJson.getJsonValue<std::string>("chipId");
			}
			std::string acxmlChipID = dataJson.getJsonValue<std::string>("chipIdACXML");
			if (ICJson["ID"].empty() || ICJson["ID"] == DEFAULT_CHIPID) {
				if (!acxmlChipID.empty()) {
					ICJson["ID"] = acxmlChipID;
				}
				else {
					ICJson["ID"] = DEFAULT_CHIPID;
				}
			}
			ICJson["Adapter"] = dataJson.getJsonValue<std::string>("chipAdapter");
			ICJson["AdapterID"] = dataJson.getJsonValue<std::string>("chipAdapterID");
			if (((dataJson.getJsonValue<int>("chipProgType") >> 24) & 0xFF) == 0x05) {
				ICJson["ProgType"] = "AP9900";
			}
			else { //Default.
				ICJson["ProgType"] = "AP9900";
			}
			//ICJson["ProgType"] = "AP9900|AP100";
			ICJson["AlgoIndex"] = QString::number(dataJson.getJsonValue<ulong>("chipDrvParam")).toStdString();

			ICInfoJson = ICJson.dump();
		}
		catch (const nlohmann::json::exception& e) {
			ALOG_FATAL("return GetSKTEnable Json : %s.", "CU", "--", ICInfoJson.c_str());
		}
	}

	void AngKMainFrame::InitToolWidget(ChipDataJsonSerial chipJson)
	{
		BaseOper baseInfo;
		std::string cfgJson = chipJson.getJsonValue<std::string>("chipOperCfgJson");

		if (cfgJson.empty())
			goto __end;

		try {//nlohmann解析失败会报异常需要捕获一下
			
			nlohmann::json _CfgJson = nlohmann::json::parse(cfgJson);
			nlohmann::json baseOperJson = _CfgJson["baseOper"];
			if (!baseOperJson.is_null()) {
				baseInfo.bBlank = baseOperJson["blank"];
				baseInfo.bBlockProg = baseOperJson["blockProg"];
				baseInfo.bErase = baseOperJson["erase"];
				baseInfo.bFunction = baseOperJson["function"];
				baseInfo.bIllegalBit = baseOperJson["illegalBit"];
				baseInfo.bProg = baseOperJson["prog"];
				baseInfo.bRead = baseOperJson["read"];
				baseInfo.bSecure = baseOperJson["secure"];
				baseInfo.bVerify = baseOperJson["verify"];
			}
		}
		catch (const nlohmann::json::exception& e) {
			ALOG_FATAL("chipOperCfg Json parse failed : %s.", "CU", "--", e.what());
		}

	__end:
		ui->toolWidget->ShowButtonFromJson(baseInfo);
	}

	void AngKMainFrame::SetRemoteManager(AngKRemoteCmdManager* remoteManager)
	{
		m_pRemoteCmdManager = remoteManager;
	}

	void AngKMainFrame::SetDeviceModel()
	{
		if (AngKDeviceModel::instance().GetConnetDevMapSize() > 0) {
			m_threadComplete.start();
			AngKMessageHandler::instance().GetACCmdHandler()->moveToThread(&m_threadComplete);

			connect(AngKMessageHandler::instance().GetACCmdHandler(), &CACCmdHandler::sigRemoteCmdComplete, this, &AngKMainFrame::onSlotRemoteCmdComplete, Qt::DirectConnection);
			connect(AngKMessageHandler::instance().GetACCmdHandler(), &CACCmdHandler::sigRemoteQueryDoCmd, this, &AngKMainFrame::onSlotRemoteQueryDoCmd, Qt::DirectConnection);
		}

		nlohmann::json systemTimejson;
		uint32_t timeStamp = QDateTime::currentSecsSinceEpoch();
		systemTimejson["CurrentTime"] = QString("0x%1").arg(timeStamp, 8, 16, QLatin1Char('0')).toStdString();

		nlohmann::json getSktInfojson;
		getSktInfojson["BPUEn"] = 0xFF;

		std::map<std::string, DeviceStu> insertDev;
		AngKDeviceModel::instance().GetConnetDevMap(insertDev);
		for (auto iter : insertDev) {
			AngKMessageHandler::instance().Command_RemoteDoPTCmd(iter.second.strIP, iter.second.nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_UpdateDeviceTime, ALL_MU, 8, QByteArray(systemTimejson.dump().c_str()));
			AngKMessageHandler::instance().Command_RemoteDoPTCmd(iter.second.strIP, iter.second.nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_GetRebootCause, ALL_MU, 8, QByteArray());
			//AngKMessageHandler::instance().Command_RemoteDoPTCmd(iter.second.strIP, iter.second.nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_GetSktInfo, ALL_MU, 8, QByteArray(getSktInfojson.dump().c_str()));
			ALOG_INFO("Device %s:%d Connected, Firmware Version : %s, MUAPP Version : %s, FPGA Version : %s", "CU", "--", iter.second.strIP.c_str(), iter.second.nHopNum
				, iter.second.strFirmwareVersion.c_str(), iter.second.strMUAPPVersion.c_str(), iter.second.strFPGAVersion.c_str());
		}
	}

	std::map<std::string, uint16_t> AngKMainFrame::GetCurProgCheckState()
	{
		std::map<std::string, uint16_t> curProgCheck;

		std::map<std::string, DeviceStu> insertDev;
		AngKDeviceModel::instance().GetConnetDevMap(insertDev);
		for (auto iter : insertDev) {

			uint16_t sktEnNum = GetProgramSiteCheck(iter.second.strIP, iter.second.nHopNum);
			curProgCheck[iter.first] = sktEnNum;
		}


		return curProgCheck;
	}

	void AngKMainFrame::MoveLogArea(QSize evSize)
	{
		if (!m_log->isHidden()) {
			if (!m_log->GetExpandCheck()) {
				m_log->setFixedHeight(m_log->GetUIMinHeight());
				m_log->move(0, ui->operateWidget->height() - (m_oldMargins.bottom() + 100));
			}
			else {
				m_log->setFixedHeight(ui->operateWidget->height() - (ui->processPathWidget->height() + ui->operateWidget->layout()->spacing() + m_oldMargins.top()));
				//m_log->move(0, ui->operateWidget->height() - (ui->processPathWidget->height() + ui->operateWidget->layout()->spacing() + m_oldMargins.top()));
			}
		}
	}

	void AngKMainFrame::SwitchICData_CRC(QFile& pFile, QByteArray& driverData, int& driverSize, uint32_t& crc, bool bCrc16)
	{
		int nByte = pFile.size();
		int a_Blk = 0;

		if (nByte % DDRMULTIPLE == 0)
			a_Blk = nByte / DDRMULTIPLE;
		else
			a_Blk = (nByte / DDRMULTIPLE) + 1;

		QByteArray filedata = pFile.readAll();
		driverSize = a_Blk * DDRMULTIPLE;

		if (bCrc16) {
			ushort crc16 = 0;
			calc_crc16sum((uchar*)(filedata.data()), nByte, &crc16);
			crc = crc16;
		}
		else {
			CCrc32Comm crc32Class;
			crc32Class.CalcSubRoutine((uint8_t*)filedata.data(), nByte);
			crc32Class.GetChecksum((uint8_t*)&crc, 4);
		}

		QByteArray ddFileByte;
		ddFileByte.resize(driverSize);
		ddFileByte.fill(0);

		ddFileByte.replace(0, nByte, filedata);

		driverData.append(ddFileByte);
	}

	void AngKMainFrame::TranslateOperType2Cmd(OperationTagType nType, int& chipOperCmd)
	{
		switch (nType)
		{
		case OperationTagType::None:
			break;
		case OperationTagType::Erase:
			chipOperCmd = ChipOperCfgCmdID::Erase;
			break;
		case OperationTagType::Blank:
			chipOperCmd = ChipOperCfgCmdID::BlankCheck;
			break;
		case OperationTagType::Program:
			chipOperCmd = ChipOperCfgCmdID::Program;
			break;
		case OperationTagType::Verify:
			chipOperCmd = ChipOperCfgCmdID::Verify;
			break;
		case OperationTagType::Secure:
			chipOperCmd = ChipOperCfgCmdID::Secure;
			break;
		case OperationTagType::Read:
			chipOperCmd = ChipOperCfgCmdID::Read;
			break;
		case OperationTagType::Self:
			chipOperCmd = ChipOperCfgCmdID::Custom;
			break;
		case OperationTagType::CheckSum:
			break;
		default:
			break;
		}
	}

	void AngKMainFrame::TranslateSubCmd2String(ChipOperCfgSubCmdID subCmd, std::string& subCmdStr)
	{
		switch (subCmd)
		{
		case UnEnable:
			break;
		case CheckID:
			subCmdStr = "CheckID";
			break;
		case PinCheck:
			subCmdStr = "PinCheck";
			break;
		case InsertionCheck:
			subCmdStr = "InsertionCheck";
			break;
		case DevicePowerOn:
			break;
		case DevicePowerOff:
			break;
		case PowerOn:
			break;
		case PowerOff:
			break;
		case SubProgram:
			subCmdStr = "Program";
			break;
		case SubErase:
			subCmdStr = "Erase";
			break;
		case SubVerify:
			subCmdStr = "Verify";
			break;
		case SubBlankCheck:
			subCmdStr = "BlankCheck";
			break;
		case SubSecure:
			subCmdStr = "Secure";
			break;
		case SubIllegalCheck:
			break;
		case SubRead:
			subCmdStr = "Read";
			break;
		case EraseIfBlankCheckFailed:
			subCmdStr = "Erase If BlankCheck Failed";
			break;
		case LowVerify:
			break;
		case HighVerify:
			break;
		case ChecksumCompare:
			subCmdStr = "Checksum Compare";
			break;
		case SubReadChipUID:
			subCmdStr = "ReadChipUID";
			break;
		case High_Low_Verify:
			break;
		default:
			break;
		}
	}

	void AngKMainFrame::UpdateMainFrameUI()
	{
		ui->propetryWidget->UpdatePropertyUI();
	}

	void AngKMainFrame::CheckBPUAdapter(std::string adapterID)
	{
		std::map<std::string, BPUInfo>& mapBPUInfo = m_projDataset->GetBPUInfo();
		std::vector<std::string> keysToRemove;

		for (auto it = mapBPUInfo.begin(); it != mapBPUInfo.end(); ++it) {
			for (int i = 0; i < it->second.vecInfos.size(); ++i) {
				bool bOK = false;
				uint32_t nBPUAdapterID = QString::fromStdString(it->second.vecInfos[i].AdapterID).toUInt(&bOK, 16);
				uint32_t nChipAdapterID = QString::fromStdString(adapterID).toUInt(&bOK, 16);
				if (nBPUAdapterID != nChipAdapterID) {
					keysToRemove.push_back(it->first);
				}
			}
		}

		// 删除之前记录的键
		for (auto key : keysToRemove) {
			mapBPUInfo.erase(key);
		}
	}

	uint16_t AngKMainFrame::GetProgramSiteCheck(std::string progIP, int nHopNum)
	{
		uint16_t nSktNum = 0;
		int allExpectNum = 0;
		for (auto prog : m_pProgWgtVec) {
			DeviceStu devInfo = prog.second->GetDevInfo();
			if (devInfo.strIP == progIP && devInfo.nHopNum == nHopNum) {
				nSktNum = prog.second->GetSitesChecked(allExpectNum);
				break;
			}
		}
		return nSktNum;
	}

	uint32_t AngKMainFrame::CheckIfSelectSite()
	{
		uint32_t CountBPUChecked = 0;
		int allExpectNum = 0;
		for (auto obj : ui->scrollAreaWidgetContents->children()) {
			if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
				AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
				if (pro != nullptr) {
					CountBPUChecked += pro->GetSitesChecked(allExpectNum);
				}
			}
		}

		return CountBPUChecked;
	}
	void AngKMainFrame::InitChipInfo(ChipDataJsonSerial chipInfo)
	{
		m_projDataset->setChipData(chipInfo);

		ALOG_INFO("Select chip : %s %s [%s].", "CU", "--", chipInfo.getJsonValue<std::string>("manufacture").c_str(),
			chipInfo.getJsonValue<std::string>("chipName").c_str(), chipInfo.getJsonValue<std::string>("chipAdapter").c_str());

		//更新芯片选择的同时将driverFile下发命令给MU
		//InstallDriver(masterDrvList, deviceDrvList, chipInfo);

		//更新操作栏按钮
		InitToolWidget(chipInfo);

		//记录芯片选择事件
		nlohmann::json chipSelectJson;
		chipSelectJson["ChipName"] = chipInfo.getJsonValue<std::string>("chipName");
		chipSelectJson["AdatperName"] = chipInfo.getJsonValue<std::string>("chipAdapter");
		chipSelectJson["ChipAlgo"] = chipInfo.getJsonValue<ulong>("chipDrvParam");

		EventLogger->SendEvent(EventBuilder->GetChipSelect(chipSelectJson));
	}

	void AngKMainFrame::InitDemoMode()
	{
		BPUInfo infos;
		infos.strBPUEn = 0xFF;
		for (int i = 0; i < 8; ++i)
		{
			SingleBPU sBPU;
			sBPU.idx = i;
			sBPU.SktCnt = 2;
			sBPU.AdapterID = "0x0001";
			infos.vecInfos.push_back(sBPU);
		}
		DeviceStu tmp;
		tmp.strIP = "127.0.0.1";
		tmp.strPort = "10";
		tmp.bpuInfoArr = infos.vecInfos;
		tmp.strSiteAlias = "Site01";
		tmp.tMainBoardInfo.strHardwareSN = "1111";
		//AngKProgrammerWidget* progWgt = new AngKProgrammerWidget(infos.vecInfos, "DemoProg");
		//progWgt->setProgramerName("DemoProg");
		ACDeviceManager::instance().addDev("127.0.0.1", 0, tmp);

		 
		tmp.strPort = "11";
		tmp.strSiteAlias = "Site02";
		tmp.nHopNum = 1;
		tmp.tMainBoardInfo.strHardwareSN = "2222";
		ACDeviceManager::instance().addDev("127.0.0.1", 1, tmp);

		AngKProgrammerWidget* progWgt = new AngKProgrammerWidget(0, "127.0.0.1:0");
		m_pProgWgtVec["DemoProg"] = progWgt;
		progWgt->setProgramerName("Site01");
		ui->scrollAreaWidgetContents->layout()->addWidget(progWgt);

		AngKProgrammerWidget* progWgt2 = new AngKProgrammerWidget(1, "127.0.0.1:1");
		m_pProgWgtVec["DemoProg"] = progWgt2;
		progWgt2->setProgramerName("Site02");
		ui->scrollAreaWidgetContents->layout()->addWidget(progWgt2);
	}

	void AngKMainFrame::CalUpdateFWTime(bool bAdd, std::string strIpHop, bool bSuccess)
	{
		if (bAdd) {
			m_mapUpdateRecord[strIpHop] = QDateTime::currentSecsSinceEpoch();


			if (m_mapFwUpdateOutTime.find(QString::fromStdString(strIpHop)) == m_mapFwUpdateOutTime.end()) {
				QTimer* newTimer = new QTimer();
				newTimer->setInterval(1000 * 60 * 20);
				m_mapFwUpdateOutTime.insert(QString::fromStdString(strIpHop), newTimer);
				connect(newTimer, &QTimer::timeout, [this, newTimer, strIpHop]() {
					delete newTimer;
					this->m_mapFwUpdateOutTime.remove(QString::fromStdString(strIpHop));
					QStringList tmpIpHop = QString::fromStdString(strIpHop).split(":");
					if (tmpIpHop.length() == 2)
						emit this->sgnUpdateFwStatus(tmpIpHop[0].toStdString(), tmpIpHop[1].toInt());
				});
				newTimer->start();

			}
			else {
				m_mapFwUpdateOutTime.find(QString::fromStdString(strIpHop)).value()->stop();
				m_mapFwUpdateOutTime.find(QString::fromStdString(strIpHop)).value()->start();
			}
		}
		else {
			auto it = m_mapUpdateRecord.find(strIpHop); // 假设我们要删除键为1的元素
			if (it != m_mapUpdateRecord.end()) {
				if(bSuccess)
					ALOG_INFO("Update firmware complete, total time %d s", "CU", "--", QDateTime::currentSecsSinceEpoch() - it->second);
				m_mapUpdateRecord.erase(it);
			}

			if (m_mapFwUpdateOutTime.find(QString::fromStdString(strIpHop)) != m_mapFwUpdateOutTime.end()) {
				m_mapFwUpdateOutTime.find(QString::fromStdString(strIpHop)).value()->stop();
				delete m_mapFwUpdateOutTime.find(QString::fromStdString(strIpHop)).value();
				m_mapFwUpdateOutTime.remove(QString::fromStdString(strIpHop));
			}
		}
	}
	void AngKMainFrame::HandleEventJobStatusChange(QString strIPHop, nlohmann::json eventData)
	{

	}

	void AngKMainFrame::HandleEventJobResult(QString strIPHop, nlohmann::json eventData)
	{
		// 打印完整的eventData内容
		ALOG_INFO("完整eventData内容: %s", "CU", "--", eventData.dump(4).c_str());

		QStringList IPhopList = strIPHop.split(":");
		for (auto obj : ui->scrollAreaWidgetContents->children()) {
			if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
				AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
				if (pro /*&& pro->GetProgramerCheck()*/) {
					DeviceStu devInfo = pro->GetDevInfo();
					if (IPhopList[0].toStdString() == devInfo.strIP && IPhopList[1].toInt() == devInfo.nHopNum) {
						int nBPUIdx = eventData["BPUIdx"];
						std::string fromDev = Utils::AngKCommonTools::GetLogFrom(nBPUIdx);

						// 也可以在这里打印更详细的解析信息
						ALOG_INFO("设备:%s, BPUIdx:%d, ResultInfo数量:%d", "CU", "--",
							strIPHop.toStdString().c_str(), nBPUIdx, (int)eventData["ResultInfo"].size());

						for (int i = 0; i < eventData["ResultInfo"].size(); ++i) {
							nlohmann::json nSktInfo = eventData["ResultInfo"][i];
							int nSKTIdx = nSktInfo["SKTIdx"];
							QString RetCode = QString::fromStdString(nSktInfo["RetCode"]);
							std::string strExtMsg = nSktInfo["ExtMsg"];

							// 打印每个SKT的详细信息
							ALOG_INFO("  SKT[%d]: RetCode=%s, ExtMsg=%s", "CU", "--",
								nSKTIdx, RetCode.toStdString().c_str(), strExtMsg.c_str());

							pro->SetBPUResultStatus(nBPUIdx, nSKTIdx, RetCode, strExtMsg);
							if (RetCode == "4000" && strExtMsg != "Unused") {
								ui->reportWidget->AddOutputNum();
							}
							else if (RetCode != "4000" && strExtMsg != "Unused") {
								ui->reportWidget->AddFailedNum();
							}
							ALOG_INFO("bAutoConnect:%d", "CU", "--", bAutoConnect);
							if (bAutoConnect) {
								CheckAutoBurnRecord(IPhopList[0], IPhopList[1].toInt(), nBPUIdx, nSKTIdx, RetCode);
							}

							// 判断烧录是否完成
							{
								MarkBurnSiteMap(strIPHop, nBPUIdx, nSKTIdx);
							}
							nlohmann::json JobResult;
							JobResult["strip"] = IPhopList[0].toStdString().c_str();
							JobResult["nHopNum"] = IPhopList[1].toInt();
							JobResult["BPUID"] = fromDev.c_str();
							JobResult["SKTIdx"] = nSKTIdx;
							JobResult["result"] = strExtMsg.c_str();
							emit SendNotification("setDoJobResult", JobResult);
							ALOG_INFO("Device %s:%d Execute Programming Command finish. SKT[%d] Result:%s.", "CU", fromDev.c_str(), IPhopList[0].toStdString().c_str(), IPhopList[1].toInt()
								, nSKTIdx, strExtMsg.c_str());
						}
					}
				}
			}
		}
		ui->reportWidget->UpdateCurrentProgress();
	}

	void AngKMainFrame::HandleEventSKTStatusChange(QString strIPHop, nlohmann::json eventData)
	{
		try {
			int bpu = eventData["BPUIdx"];
			std::string status = eventData["Status"];
			std::string uid = eventData.at("AdapterInfo").at("UID");
			QStringList IPHopList = strIPHop.split(":");
			ALOG_INFO("Device %s:%d SKT status change. Idx: %d, AdUID: %s, Status: %s.", "CU", "--", IPHopList[0].toStdString().c_str(), IPHopList[1].toInt(), bpu, uid.c_str(), status.c_str());
		}
		catch (const nlohmann::json::exception& e) {
			ALOG_FATAL("Parse SKTStatus Json failed : %s", "CU", "--", e.what());
		}

	}

	void AngKMainFrame::HandleEventSKTAlarm(QString strIPHop, nlohmann::json eventData)
	{
	}

	void AngKMainFrame::HandleEventDPSAlert(QString strIPHop, nlohmann::json eventData) {
		try {
			for (const auto& item : eventData) {
				int bpu = item["BPU"];
				std::string alert = item["Alert"];
				QString errStr, fromDev;
				fromDev.append("B").append(QString::number(bpu));
				errStr.append("Alert: ").append(alert.c_str());
				QStringList IPHopList = strIPHop.split(":");
				ALOG_WARN("Device %s:%d %s", "CU", fromDev.toStdString().c_str(), IPHopList[0].toStdString().c_str(), IPHopList[1].toInt(), errStr.toStdString().c_str());
				NotifyManager::instance().notify(tr("Notify"), tr("Device %1 Alert: %2").arg(strIPHop).arg(errStr));
			}
		}
		catch (const nlohmann::json::exception& e) {
			ALOG_FATAL("Parse DPSAlert Json failed : %s", "CU", "--", e.what());
		}
	}

	void AngKMainFrame::ResetALLSiteStatus()
	{
		for (auto obj : ui->scrollAreaWidgetContents->children()) {
			if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
				AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
				if (pro /*&& pro->GetProgramerCheck()*/) {
					pro->RestSiteStatus();
				}
			}
		}

		ui->reportWidget->SetCurrentProgress(0);
	}

	void AngKMainFrame::CheckSktUseState()
	{
		m_bCheckSktUseInfo = true;

		//nlohmann::json queryAdapterJson;
		//queryAdapterJson["BPUEn"] = nBPUEn;

		//AngKMessageHandler::instance().Command_RemoteDoPTCmd(strIP, nHopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_GetSktInfoSimple, ALL_MU, 8, QByteArray(queryAdapterJson.dump().c_str()));
	}

	void AngKMainFrame::SetProjReport(ACProjManager* _projMgr)
	{
		ACReportRecord& GReporter = GetReporter();
		QString projPath = _projMgr->GetProjName();
		QFileInfo fInfo(projPath);
		GReporter.SetBulidVer(projPath, AngKGlobalInstance::instance()->ReadValue("Version", "BuildVer").toString());
		GReporter.SetProjCreateTime(projPath, fInfo.birthTime().toString("yyyy/MM/dd hh:mm:ss"));
		GReporter.SetDevType(projPath, "AP9900");
		GReporter.SetPCLocation(projPath, "");
		GReporter.SetProjDescriptor(projPath, "");

		ChipDataJsonSerial chipInfo = _projMgr->GetProjData()->getChipData();
		GReporter.SetICManu(projPath, QString::fromStdString(chipInfo.getJsonValue<std::string>("manufacture")));
		GReporter.SetICName(projPath, QString::fromStdString(chipInfo.getJsonValue<std::string>("chipName")));
		GReporter.SetICPack(projPath, QString::fromStdString(chipInfo.getJsonValue<std::string>("chipPackage")));
		GReporter.SetAdapterName(projPath, QString::fromStdString(chipInfo.getJsonValue<std::string>("chipAdapter")));

		GReporter.SetChksumType(projPath, "Byte");
		GReporter.SetChksum(projPath, QString::fromStdString(_projMgr->GetProjData()->GetProjProperty().checksum));

		std::vector<eMMCFileDataJsonSerial>& vecFile = _projMgr->GetProjData()->geteMMCFileData();
		for (int i = 0; i < vecFile.size(); ++i) {
			eMMCFileInfo eInfo;
			vecFile[i].deserialize(eInfo);

			GReporter.AddProjInfo_BinFile(projPath, QString::fromStdString(eInfo.sFilePath));
		}
	}

	void AngKMainFrame::SwitchLogPrint(int nLevel, std::string fromDev, std::string cmdName, std::string recvIP, int HopNum, std::string filteredStr)
	{
		switch ((Utils::AngkLogger::LogLevel)nLevel)
		{
		case Utils::AngkLogger::LogLevel::DEBUG:
			ALOG_DEBUG("Recv %s Message from %s:%d(%s)", fromDev.c_str(), "CU", cmdName.c_str(), recvIP.c_str(), HopNum, filteredStr.c_str());
			break;
		case Utils::AngkLogger::LogLevel::INFO:
			ALOG_INFO("Recv %s Message from %s:%d(%s)", fromDev.c_str(), "CU", cmdName.c_str(), recvIP.c_str(), HopNum, filteredStr.c_str());
			break;
		case Utils::AngkLogger::LogLevel::WARN:
			ALOG_WARN("Recv %s Message from %s:%d(%s)", fromDev.c_str(), "CU", cmdName.c_str(), recvIP.c_str(), HopNum, filteredStr.c_str());
			break;
		case Utils::AngkLogger::LogLevel::Error:
			ALOG_ERROR("Recv %s Message from %s:%d(%s)", fromDev.c_str(), "CU", cmdName.c_str(), recvIP.c_str(), HopNum, filteredStr.c_str());
			break;
		case Utils::AngkLogger::LogLevel::FATAL:
			ALOG_FATAL("Recv %s Message from %s:%d(%s)", fromDev.c_str(), "CU", cmdName.c_str(), recvIP.c_str(), HopNum, filteredStr.c_str());
			break;
		default:
			break;
		}
	}

	void AngKMainFrame::ClearBPUChcekState(std::string strIP, int nHop)
	{
		for (auto obj : ui->scrollAreaWidgetContents->children()) {
			if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
				AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
				if (pro) {
					auto devInfo = pro->GetDevInfo();
					if (devInfo.strIP == strIP && devInfo.nHopNum == nHop)
					{
						pro->SetSiteStatus(ALL_SKT, SiteStatus::Normal);
						pro->ResetSiteCheck(0);
					}
				}
			}
		}
	}


	void AngKMainFrame::SetBurnStatus(bool burnStatus) {

		for (auto obj : ui->scrollAreaWidgetContents->children()) {
			if (obj->metaObject()->className() == QStringLiteral("AngKProgrammerWidget")) {
				AngKProgrammerWidget* pro = qobject_cast<AngKProgrammerWidget*>(obj);
				pro->SetDevBurnStatus(burnStatus);
			}
		}
		ui->controlWidget->setEnabled(!burnStatus);
		ui->toolWidget->setEnabled(!burnStatus);

		m_bBurning = burnStatus;
		if (burnStatus)
			m_sDownloadResult.clear();

	}

	void AngKMainFrame::MarkBurnSiteMap(const QString& ipHop, int nBPUIdx, int nSKTIdx) {
		


		auto iter = m_mapProgBPUChecked.find(ipHop);
		if (iter != m_mapProgBPUChecked.end()) {
			if (nBPUIdx == -1 && nSKTIdx == -1)
				iter->second = 0;
			else
				iter->second &= ~(uint32_t)(BPUCalVector[(nBPUIdx * 2) + nSKTIdx]);
		}
		auto it = m_mapProgBPUChecked.begin();
		for (; it != m_mapProgBPUChecked.end(); it++) {
			if (it->second != 0) {
				break;
			}
		}
		if (it == m_mapProgBPUChecked.end()) {
			SetBurnStatus(false);
		}

	}
}
