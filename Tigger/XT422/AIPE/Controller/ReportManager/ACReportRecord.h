#pragma once

#include <QObject>
#include <QMutex>
#include "ACHtmlLogWriter.h"
#include "DeviceModel.h"
#include "DevErrCode.h"
#include "ErrTypes.h"
#include "json.hpp"
#define ADAPTER_MAXNUM	(8)

typedef struct tagSktReport {
	uint32_t PassCnt;						///Pass个数
	uint32_t FailCnt;						///Fail个数
	QString SktUID;						///适配板的UID
	std::map<uint32_t, uint32_t> mapErrCode;  ////错误代码与错误次数
	uint32_t AdpLimitCnt;				///当前适配板板的限制次数
	uint32_t AdpCurrentCnt;				///当前适配板的使用次数
	uint32_t AdpFailCnt;				///当前适配板的总失败次数
}tSktReport;



typedef struct tagSiteReport {
	QString strSiteSN;		///站点序列号
	QString strSiteAlias;	///站点别名
	int AdpCnt;			////当前的适配板个数
	uint32_t PassCnt;		///当前站点成功总数
	uint32_t FailCnt;		///当前站点失败总数
	MainBoardInfo BtmBoardInfo;		///底板的信息
	std::vector<tSktReport> vArrSktReport[ADAPTER_MAXNUM];///保存所有用过座子,每个SKT位置會对应一组使用的座子，
														///实际使用的时候在某一个时刻只有一个适配板放置在座子位置上
														//如果座子接触不好，可能操作员会换另外一个适配板到该座子上
	tSktReport* SktReport[ADAPTER_MAXNUM]; ////SKT位置上对应使用的当前适配板
}tSiteReport;

typedef struct tagReporter {
	QString strBulidVer;			///创建工程使用的Build版本号
	QString strProjName;			///工程名称
	QString strProjCreateTime;		///工程创建时间
	QString strDevType;				///创建工程的设备类型 
	QString strPCLocation;			///创建工程的PC名称
	QString strProjDescriptor;		///工程描述
	QString strICManu;				///IC厂商
	QString strICName;				///IC名称
	QString strICPack;				///IC封装
	QString strAdapterName;			///适配板名称
	QString strChksum;				///校验值
	QString strChksumType;			///检验类型
	QString strProjLoadTime;		///工程加载时间
	QString strCurTime;				///当前的查询时间
	QString strCurFuncmode;			///当前执行的命令
	QString strExecPC;				///执行任务的PC
	QString strCmdSequence;			///命令序列
	std::map<QString, tSiteReport*> mapSiteReport;  ////站点信息情况
	std::vector<std::string> vecBinFile;			///档案加载信息
}tReporter;


typedef struct sncInfoReporter {
	QString			strSncFileName;         ///SNC文件名称
	unsigned long   StartIndex;			    ///起始索引
	unsigned long   TotalNumber;			///总数
	QString			strChipName;			///芯片名称
	QString			strSNGenName;			///设置SNGEN模式名称
	unsigned long   ProgramIndex;			///程序索引
	unsigned long   ReuseCnt;				///重用数量
	QString			strReuseIndex;			///重用队列索引
}sReporter;


////////////////////////////////////////////////////////////////////
typedef struct tAdpSNInfo {			////记录每个座子信息
	QString strAdpSN;				////座子序列号
	QString strPass;				////座子成功数
	QString strFail;				////座子失败数
	QString strAdpName;				////适配版名称
	QString strWriteTime;			////写入文件时间
}tAdpInfo;
////////////////////////////////////////////////////////////////////



class ACReportRecord : public QObject
{
	Q_OBJECT

public:	
	ACReportRecord(QObject* parent = nullptr);
	~ACReportRecord();
	//void SetLogMsg(CLogMsg* pLogMsg) { m_pLogMsg = pLogMsg; }
	void SetBulidVer(const QString& projName, QString& BulidVer) { m_mapReport[projName.toStdString()].strBulidVer = BulidVer; }
	void SetProjName(const QString& ProjName);
	void SetProjCreateTime(const QString& projName, const QString& ProjCreateTime) { m_mapReport[projName.toStdString()].strProjCreateTime = ProjCreateTime; }
	void SetDevType(const QString& projName, const QString& DevType) { m_mapReport[projName.toStdString()].strDevType = DevType; }
	void SetPCLocation(const QString& projName, const QString& PCLocation) { m_mapReport[projName.toStdString()].strPCLocation = PCLocation; }
	void SetProjDescriptor(const QString& projName, const QString& ProjDescriptor) { m_mapReport[projName.toStdString()].strProjDescriptor = ProjDescriptor; }
	void SetICManu(const QString& projName, const QString& ICManu) { m_mapReport[projName.toStdString()].strICManu = ICManu; }
	void SetICName(const QString& projName, const QString& ICName) { m_mapReport[projName.toStdString()].strICName = ICName; }
	void SetICPack(const QString& projName, const QString& ICPack) { m_mapReport[projName.toStdString()].strICPack = ICPack; }
	void SetAdapterName(const QString& projName, const QString& AdapterName) { m_mapReport[projName.toStdString()].strAdapterName = AdapterName; }
	void SetChksum(const QString& projName, const QString& Chksum) { m_mapReport[projName.toStdString()].strChksum = Chksum; }
	void SetChksumType(const QString& projName, const QString& ChksumType) { m_mapReport[projName.toStdString()].strChksumType = ChksumType; }
	void SetFuncmode(const QString& projName, const QString& Funcmode) { m_mapReport[projName.toStdString()].strCurFuncmode = Funcmode; }
	void SetCmdSequence(const QString& projName, const QString& CmdSequence) { m_mapReport[projName.toStdString()].strCmdSequence = CmdSequence; }
	void SetBtmBoardInfo(const QString& projName, const QString strSiteSN, const MainBoardInfo& BtmBoardInfo);
	void AddProjInfo_BinFile(const QString& projName, const QString strBinFile) { m_mapReport[projName.toStdString()].vecBinFile.push_back(strBinFile.toStdString()); }

	//SNC 文件信息赋值
	void SetSncFile(const QString& projName, const QString& FileName) { m_sncReporter.strSncFileName = FileName; }
	void SetStartIndex(const QString& projName, const ulong& StaIndex) { m_sncReporter.StartIndex = StaIndex; }
	void SetTotalNumber(const QString& projName, const ulong& TotalNum) { m_sncReporter.TotalNumber = TotalNum; }
	void SetstrChipName(const QString& projName, const QString& ChipName) { m_sncReporter.strChipName = ChipName; }
	void SetstrSNGenName(const QString& projName, const QString& SNGenName) { m_sncReporter.strSNGenName = SNGenName; }
	void SetProgramIndex(const QString& projName, const ulong& ProgramIdx) { m_sncReporter.ProgramIndex = ProgramIdx; }
	void SetReuseCnt(const QString& projName, const ulong& ReuseCount) { m_sncReporter.ReuseCnt = ReuseCount; }
	void SetstrReuseIndex(const QString& projName, const QString& ReuseIdx) { m_sncReporter.strReuseIndex = ReuseIdx; }


	int AddSite(const QString& projName, const QString strSiteSN, const QString strAlias, const int AdpCnt);
	int AddSite(const QString& projName, DeviceStu* pDevInfo, const int AdpCnt);

	int UpdateSite(const QString& projName, const QString strSiteSN, uint ErrCode[], tSktInfo SktInfo[], int AdpCnt, uchar AdpActive[]);
	int UpdateAdapterCounter(const QString& projName, const QString strSiteSN, uint64_t AdpID, uint32_t LimitCnt, uint32_t CurrentCnt);
	int UpdateAdapterCounter(const QString& projName, const QString strSiteSN, uint32_t TotalCnt, uint32_t FailedCnt);

	//int DumpReport(IReportlog* pReportlog);
	void ReInit();
	int ClearStatistic(); ///清除统计数据

	//int GetRunStatusReport(IReportlog* pReportlog, bool bIsDetail);

	/////WriteLog To Html
	int WriteReportToLog(ACHtmlLogWriter* m_pLogWriter);
	int WriteReportToAloneFile(ACHtmlLogWriter* m_pLogWriter);

	int SaveReprotToJsonStream(std::string& strReport);

	int SerialReportBasicInfor2Json(std::string& strJson);
	//int GetSysLogLoadPrjEventJson(QString strProjectID, std::string& strJson, CSysLog* pSysLog);

	int DumpAdpStaticsToJson(nlohmann::json& SktCounter);

	int BuildCommitSysLogJsonData(QString strProjectID, QString strSNCID, QString strFinishFlag, QString& strJsonOut);

	int GetSiteReportCounter(const QString strSiteSN, int& PassCnt, int& FailCnt);

	/////////////////////////////////////////////
	bool GetAdpSNPassFail();/////////////////////
	bool TraverseFileCreate();///////////////////
private:
	int DeleteAllSite();
	tSktReport* GetSktReport(tSiteReport* pSiteReport, tSktInfo* pSktInfo, int AdpIdx);
	int GetSktCounter(tSiteReport* pSiteReport, int SktIdx, uint32_t& PassCnt, uint32_t& FailCnt);

	void ReportBegin(ACHtmlLogWriter* m_pLogWriter);
	void ReportEnd(ACHtmlLogWriter* m_pLogWriter);
	void ReportDumpProjectInfo(ACHtmlLogWriter* m_pLogWriter);
	void ReportDumpProjOrgFile(ACHtmlLogWriter* m_pLogWriter);
	bool ReportDumpOverAllStatistic(ACHtmlLogWriter* m_pLogWriter);
	bool ReportDumpSiteStatistic(ACHtmlLogWriter* m_pLogWriter);


	void ReportBeginAlone(ACHtmlLogWriter* m_pLogWriter);
	void ReportEndAlone(ACHtmlLogWriter* m_pLogWriter);
	int ReportDumpAdpStatics(ACHtmlLogWriter* pLogWriter);

	bool ReportDumpSncInfo(ACHtmlLogWriter* m_pLogWriter);//Report html 文件下添加一栏SNC文件信息

	bool DumpOverAllStatisticToJson(nlohmann::json& OverAllStatistic);
	bool DumpSiteStatisticToJson(nlohmann::json& OverallStatistic);

	bool GetOverAllStatistics_EachErr(std::map<QString, uint32_t>& ErrInfoStatistis);
	ACReportRecord& operator=(const ACReportRecord&);
	ACReportRecord(const ACReportRecord&);
	tReporter m_Reporter;
	sReporter m_sncReporter;  //SNC
	std::map<std::string, tReporter> m_mapReport;
	//CLogMsg* m_pLogMsg;
	QMutex m_Mutex; ///互斥条件

	std::vector<tAdpInfo> m_mapAdpSNInfo;
};

ACReportRecord& GetReporter();