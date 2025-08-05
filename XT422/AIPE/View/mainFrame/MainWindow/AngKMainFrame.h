#pragma once

#include <QtWidgets/QWidget>
#include <QFile>
#include <QMap>
#include "AngKProjDataset.h"
#include "ACTaskManager.h"
#include "AnimWidgetManager.h"
#include "ChipOperatorCfgDefine.h"
#include "ACAutomaticManager.h"
#include <QMutex>
#include <QThread>
#include <QTcpSocket>
#include <QSemaphore>
#include <QTimer>
#include <QStringList>
#include <memory>
#include "UUID/mtuidgenerator.h"
namespace Ui { class AngKMainFrame; };



class AngKChkManager;
class AngKDeviceModel;
class AngKDataBuffer;
class AngKShowLogArea;
class AngKLogManager;
class AngKRemoteCmdManager;
class AngKProgrammerWidget;
class AngKChipDialog;
class AutoMessageServer;
namespace AcroView
{
	struct BurnRecord
	{
		int curEnable;
		std::map<int, QString> maskDone;	//烧录成功为1，不同失败为2、3、4，对应不同的结果

		BurnRecord() {
			curEnable = 0;
			maskDone.clear();
		}

		bool operator==(const BurnRecord& v) const {
			return v.curEnable == this->curEnable && v.maskDone == this->maskDone;
		}
	};

	class AngKMainFrame : public QWidget
	{
		Q_OBJECT

	public:
		AngKMainFrame(QWidget* parent, AngKProjDataset* _proj, std::shared_ptr<ACTaskManager> _pTaskMgr);
		~AngKMainFrame();

		void InitAnimWgt();

		/// <summary>
		/// 初始化控制窗口
		/// </summary>
		void InitControlWgt();

		/// <summary>
		/// 初始化日志区域窗口
		/// </summary>
		void InitScrollArea();

		/// <summary>
		/// 初始化属性窗口
		/// </summary>
		void InitPropetryWidget();

		/// <summary>
		/// 初始化通信连接
		/// </summary>
		void InitMessgaeConnect();

		void showToolWidget(bool isShow);

		void showLogArea(bool state);

		void setPropetryAreaShow(WinActionType state, bool bSHow);

		/// <summary>
		/// 初始化日志标题栏
		/// </summary>
		/// <param name="text">初始化日志标题</param>
		void setLogText(QString text);

		void setProjDataset(AngKProjDataset* pDataSet);

		AngKProjDataset* getProjDataset() { return m_projDataset; };

		void setDataBuffer(AngKDataBuffer* _dataBuffer);

		void InstallDriver(QStringList mstDrvList, QStringList devDrvList, ChipDataJsonSerial chipInfo);

		int SendDriverCmd(QString strIP, int nHop, QString devDrv, QString mstDrv, std::string& strInstallDriver, uint32_t sktEn = 0);

		int SendFPGACmd(QString strIP, int nHop, QString strFPGA, std::string& strInstallFPGAJson, uint32_t sktEn = 0);

		/// <summary>
		/// 根据IC设置来进行命令下发
		/// </summary>
		void InitToolWidget(ChipDataJsonSerial chipJson);

		int GetHopNumUse() const { return m_LinkHopNum; }

		void SetRemoteManager(AngKRemoteCmdManager* remoteManager);

		void SetDeviceModel();

		std::map<std::string, uint16_t> GetCurProgCheckState();

		void MoveLogArea(QSize evSize);

		//设置报告参数
		void SetProjReport(ACProjManager* _projMgr);

		void SwitchLogPrint(int nLevel, std::string fromDev, std::string cmdName, std::string recvIP, int HopNum, std::string filteredStr);

		void SetAnalyzeFlag(bool bFlag) { m_bAnalyzeProgress = bFlag; }

		void ClearBPUChcekState(std::string strIP, int nHop);

		void SetBurnStatus(bool burnStatus);

		bool GetBurnStatus() { return m_bBurning; };

		void MarkBurnSiteMap(const QString& ipHop, int nBPUIdx = -1, int nSKTIdx = -1);
		std::map<QString, uint32_t> GetBurnMap() { return m_mapProgBPUChecked; };
	signals:
		//HopNum, ProgressValue
		void sgnUpdateFPGAValue(std::string, int, int);

		void sgnUpdateFwStatus(std::string, int);

		void sgnLoadProjFinish();

		void sgnSupplyInstallDriver(QString, uint32_t);

		void sgnHandleEventInfo(QString, std::string);

		void sgnShowSktInfo(std::string, std::string);

		void sgnShowSktInfoSimple(std::string, std::string);

		void sgnProgramSetting(int, int);

		void sgnProjectValue(int, QString, int);

		void sgnProgramSelfTestResult(QString, QString, int);

		void sgnRecvDoCmdResult(QString, uint32_t, uint32_t, uint32_t);

		void sgnRecvDoCustom(QString, uint16_t, QByteArray);

		void sgnModifyDeviceInfo(std::string, std::string);

		void sgnSetProgress(QString, int, int, int, int, std::string, std::string);

		void sgnSetUpdateFw(QString, uint32_t, uint32_t, QByteArray, uint32_t);

		void sgnTaskInitChipInfo(ChipDataJsonSerial chipJson);

		void sgnTaskSetProjBindBPU(int, QString, int, std::string);

		void sgnMasterChipAnalyzeResult(uint32_t, std::string);

		void sgnHandleEventAnalyzeResult(std::string resultJson);
		void sgnHandleEventExtCSDFetched(std::string resultJson);
		void sgnHandleEventAnalyzeStatusChange(std::string resultJson);
		void sgnHandleEventAnalyzeInfo(std::string resultJson);
		void sgnHandleEventUIDFetched(std::string resultJson);
		void sgnHandleEventChipIDFetched(std::string resultJson);
		void sgnHandleEventTransmitChipIDFetched(std::string resultJson);
		void sgnHandleEventTransmitExtCSDFetched(std::string resultJson);
		void sgnUpdateAnalyzeValue(std::string, int, int);

		void sgnAutomicOver();

		void sgnUpdateSKTEnable(std::string, std::string, int);

		void sgnContinueRunAutomatic(bool);
		//send to jsonRpcServer
		void SendNotification(const QString& method, const nlohmann::json& result);
	public slots:
		void onSlotLogAreaExpand(int);
		void onSlotAddProject();

		void onSlotRemoteCmdComplete(QString, uint32_t, uint32_t, int32_t, uint16_t, uint32_t, uint16_t, QByteArray, uint32_t);
		void onSlotRemoteQueryDoCmd(QString, uint32_t, uint32_t, uint32_t, uint16_t, uint16_t, QByteArray);

		//打开HexEdit
		void onSlotOpenBufferTool();

		//打开选择芯片
		void onSlotOpenSelectChipDlg();
		void onSlotSendChipInfo(ChipDataJsonSerial);

		//更新界面相关工程属性配置UI
		void onSlotProperty2Mainframe(QString, int);

		//执行烧录，fix:考虑是否放入线程中进行？
		void onSlotStartBurnData();

		// 检测勾选烧录的设备其task是否与最近一次下发task一致
		void onSlotAutomaticCheckSitesTask();

		void onSlotUpdateFirmwareFile(QString, int, QString, QString, QString);

		void onSlotLogWriteUI(QString);

		void onSlotLinkStatusChanged(quint32 HopNum, quint16 LinkStatus, quint32 IsLastHop);

		void onSlotSetAllSelect(bool);

		void onSlotSendDrvPara(QString, QString, bool);

		void onSlotSupplyInstallDriver(QString, uint32_t);

		void onSlotSplitterMoved(int, int);

		void onSlotHandleEventInfo(QString, std::string);

		void onSlotQueryAdapterInfo(int, int, std::string, int);

		void onSlotDownloadProject(QString, int, QString);
		void onSlotCheckDevVFiles(QString, int, QString);

		void onSlotProgrammgerTest(int);

		void onSlotRecvDoCmdResult(QString, uint32_t, uint32_t, uint32_t);

		void onSlotRecvDoCustom(QString, uint16_t, QByteArray);

		void onSlotAutomatic_ChipInPlace(int, uint32_t, std::string);

		void onSlotModifyDeviceInfo(std::string, std::string);

		void onSlotAutoTellDevReady(int);

		void onSlotSetProgress(QString, int, int, int, int, std::string, std::string);

		void onSlotSetUpdateFw(QString, uint32_t, uint32_t, QByteArray, uint32_t);

		void onSlotDevOffLine(QString ipStr, int hop);

		void onSlotDevOnLine(QString ipStr, int hop);

		void onSlotTaskInitChipInfo(ChipDataJsonSerial chipJson);

		void onSlotTaskSetProjBindBPU(int, QString, int, std::string);

		void onSlotHandleReportRequested(QTcpSocket* client);

		void onSlotClearLotData(int expect);

		void onSlotUpdateTaskInfo();

		void onSlotRequestSKTInfo(std::string, int);

		void onSlotUpdateSKTEnable(std::string, std::string, int);
	private:
		void GetCurSelectICInfo(std::string& ICInfoJson, ChipDataJsonSerial jsonSer);

		void SwitchICData_CRC(QFile& pFile, QByteArray& driverData, int& driverSize, uint32_t& crc16, bool bCrc16 = true);

		void TranslateOperType2Cmd(OperationTagType nType, int& chipOperCmd);

		void TranslateSubCmd2String(ChipOperCfgSubCmdID subCmd, std::string& subCmdStr);

		void UpdateMainFrameUI();

		/// <summary>
		/// 检查BPU烧录座与选择的芯片是否匹配，匹配则直接置为Check状态
		/// </summary>
		/// <param name="adapterID">适配烧录座ID</param>
		void CheckBPUAdapter(std::string adapterID);

		/// <summary>
		/// 根据设备IP获取当前设备的Site选择情况
		/// </summary>
		/// <param name="progIP">查询的IP</param>
		/// <returns>烧录座的选择情况，即SktEnable</returns>
		uint16_t GetProgramSiteCheck(std::string progIP, int nHopNum);

		uint32_t CheckIfSelectSite();

		void InitChipInfo(ChipDataJsonSerial chipInfo);

		void InitDemoMode();

		void CalUpdateFWTime(bool bAdd, std::string strIpHop, bool bSuccess = true);

		//关键事件处理函数
		void HandleEventJobStatusChange(QString strIPHop, nlohmann::json eventData);
		void HandleEventJobResult(QString strIPHop, nlohmann::json eventData);
		void HandleEventSKTStatusChange(QString strIPHop, nlohmann::json eventData);
		void HandleEventSKTAlarm(QString strIPHop, nlohmann::json eventData);
		void HandleEventDPSAlert(QString strIPHop, nlohmann::json eventData);

		/// <summary>
		/// 烧录前重置所有Site的状态
		/// </summary>
		void ResetALLSiteStatus();

		void CheckSktUseState();

		void DevRootPartCheck();

		int WaitCmdExeQueueAvailable(QString strIPHop, int Timeout, int AvailableCnt = 1);

		void SetProjBindBPU(int nSktEn, QString strIP, int nHop, std::string strAdapterID);

		void RecordTaskEvent(QString strIP, int nHop, int bResult, QString taskFile);

		void ExecuteBurnCommand(std::string& docmdSeqJson, QStringList& processList, int OperType);

		void CheckAutoBurnRecord(QString strIP, int nHop, int nBPUIdx, int nSktIdx, QString resultCode);

		template<typename K, typename V>
		typename std::multimap<K, V>::const_iterator find_item_by_key_and_value(const std::multimap<K, V>& mm, K k, V v) {
			auto range = mm.equal_range(k);
			return std::find_if(range.first, range.second, [v](const auto& p) {return p.second == v; });
		}

		template<typename K, typename V>
		void erase_item_by_key_and_value(std::multimap<K, V>& mm, K k, V v) {
			auto iter = find_item_by_key_and_value(mm, k, v);
			if (iter != mm.end())
				mm.erase(iter);
		}

	private:
		Ui::AngKMainFrame* ui;
		bool m_isExpend;
		int m_logWgtMinHeight;
		int m_logWgtMaxHeight;
		bool m_bUpdateProgress;
		bool m_bAnalyzeProgress;
		int	m_LinkHopNum;
		bool m_bLoadProjStatus;
		bool m_bCheckSktUseInfo;
		int m_supplyDriver;

		bool m_bBurning;
		MTUIDGenerator *realGenerator;
		int64_t	m_nRecordDocmdTime;
		AnimWidgetManager*	m_animWgt;
		AngKShowLogArea*	m_log;
		QMargins			m_oldMargins;
		AngKProjDataset*	m_projDataset;
		AngKDataBuffer*		m_pDataBuffer;
		AngKRemoteCmdManager* m_pRemoteCmdManager;
		std::map<QString, AngKProgrammerWidget*> m_pProgWgtVec;
		AngKChkManager*		m_pChkManager;
		AngKChipDialog*		m_pChipSelectDlg;
		std::string			m_curLoadProj;
		uint32_t			m_curSelectSite;
		std::map<std::string, qint64>	m_mapUpdateRecord;
		QString				m_openProjPath;
		QString				m_openTaskPath;

		QMap<QString, QTimer*> m_mapFwUpdateOutTime;
		QMutex projMutex;
		QSemaphore* projSemaphore;
		std::map<QString, QSemaphore*>			m_IPHop_projSemaphore;
		std::map<QString, QList<int>>			m_IPHop_projCommResultCode;
		std::map<QString, std::promise<int>>	m_IPHop_TaskResultPromise;
		std::shared_ptr<ACTaskManager>		m_pTaskManager;

		std::map<QString, uint32_t> m_mapProgBPUChecked;
		QString m_sDownloadResult;


		//自动化配置相关
		//key: ipHop; value:一批次下发的Enable-已经完成的Enbale
		std::multimap<std::string, std::pair<int, BurnRecord>> m_mapAutoRecord;
		
		QThread m_threadComplete;
		std::unique_ptr<AutoMessageServer> m_pAutoMsgServer;
		//key: ipHop; value: SktEnable
		std::map<QString, int> m_mapDownloadSktEn;

		// SKT结果收集相关成员变量
		QTimer* m_pSKTResultTimer;           // 定时器，用于3秒后发送结果
		QStringList m_sktResultList;         // 收集的结果列表
		bool m_bSKTTimerStarted;             // 标记定时器是否已启动
	};
}