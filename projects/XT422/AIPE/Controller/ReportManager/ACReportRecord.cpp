#include "ACReportRecord.h"
#include "AngkLogger.h"
#include <QHostInfo>
#include <QTime>

ACReportRecord& GetReporter()
{
	static ACReportRecord GReporter;
	return GReporter;
}

void ACReportRecord::SetProjName(const QString& ProjName)
{
	tReporter newReporter;
	newReporter.strProjName = ProjName;
	newReporter.strProjLoadTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");

	newReporter.strExecPC = QHostInfo::localHostName();

	m_mapReport[ProjName.toStdString()] = newReporter;
}

void ACReportRecord::ReInit()
{
	DeleteAllSite();
}

int ACReportRecord::DeleteAllSite()
{
	ClearStatistic();
	m_mapReport.clear();
	for (auto& projIter : m_mapReport) {
		projIter.second.strBulidVer = "";			///创建工程使用的Build版本号
		projIter.second.strProjName = "";			///工程名称
		projIter.second.strProjCreateTime = "";		///工程创建时间
		projIter.second.strDevType = "";				///创建工程的设备类型 
		projIter.second.strPCLocation = "";			///创建工程的PC名称
		projIter.second.strProjDescriptor = "";		///工程描述
		projIter.second.strICManu = "";				///IC厂商
		projIter.second.strICName = "";				///IC名称
		projIter.second.strAdapterName = "";			///适配板名称
		projIter.second.strChksum = "";				///校验值
		projIter.second.strChksumType = "";			///检验类型
		projIter.second.strProjLoadTime = "";		///工程加载时间
		projIter.second.strCurTime = "";				///当前的查询时间
		projIter.second.strExecPC = "";			///当前执行命令的PC
		projIter.second.strCmdSequence = "";		///命令执行序列
	}
	m_Reporter.strBulidVer = "";			///创建工程使用的Build版本号
	m_Reporter.strProjName = "";			///工程名称
	m_Reporter.strProjCreateTime = "";		///工程创建时间
	m_Reporter.strDevType = "";				///创建工程的设备类型 
	m_Reporter.strPCLocation = "";			///创建工程的PC名称
	m_Reporter.strProjDescriptor = "";		///工程描述
	m_Reporter.strICManu = "";				///IC厂商
	m_Reporter.strICName = "";				///IC名称
	m_Reporter.strAdapterName = "";			///适配板名称
	m_Reporter.strChksum = "";				///校验值
	m_Reporter.strChksumType = "";			///检验类型
	m_Reporter.strProjLoadTime = "";		///工程加载时间
	m_Reporter.strCurTime = "";				///当前的查询时间
	m_Reporter.strExecPC = "";			///当前执行命令的PC
	m_Reporter.strCmdSequence = "";		///命令执行序列
	m_Reporter.vecBinFile.clear();
	return ACERR_OK;
}

ACReportRecord::ACReportRecord(QObject *parent)
	: QObject(parent)
{
}

ACReportRecord::~ACReportRecord()
{
}

void ACReportRecord::SetBtmBoardInfo(const QString& projName, const QString strSiteSN, const MainBoardInfo& BtmBoardInfo)
{
	int Ret = ACERR_OK;
	m_Mutex.lock();
	std::map<QString, tSiteReport*>::iterator it;
	it = m_mapReport[projName.toStdString()].mapSiteReport.find(strSiteSN);
	if (it != m_mapReport[projName.toStdString()].mapSiteReport.end()) {///存在
		tSiteReport* pSiteReport = it->second;
		if (pSiteReport->BtmBoardInfo.strHardwareUID != BtmBoardInfo.strHardwareUID)
			memcpy(&pSiteReport->BtmBoardInfo, &BtmBoardInfo, sizeof(MainBoardInfo));
	}
	m_Mutex.unlock();
}

int ACReportRecord::AddSite(const QString& projName, DeviceStu* pDevInfo, const int AdpCnt)
{
	return AddSite(projName, QString::fromStdString(pDevInfo->tMainBoardInfo.strHardwareSN), QString::fromStdString(pDevInfo->strSiteAlias), AdpCnt);
}

int ACReportRecord::AddSite(const QString& projName, const QString strSiteSN, const QString strAlias, const int AdpCnt)
{
	int Ret = ACERR_OK;
	m_Mutex.lock();
	std::map<QString, tSiteReport*>::iterator it;
	it = m_mapReport[projName.toStdString()].mapSiteReport.find(strSiteSN);
	if (it != m_mapReport[projName.toStdString()].mapSiteReport.end()) {///之前已经添加过则不需要

	}
	else {
		int i;
		tSiteReport* pSiteReport = NULL;
		std::pair<QString, tSiteReport*> newSite;

		pSiteReport = new tSiteReport;
		if (pSiteReport == NULL) {
			Ret = ACERR_MEMALLOC; goto __end;
		}
		///初始化站点信息
		pSiteReport->strSiteSN = strSiteSN;
		pSiteReport->strSiteAlias = strAlias;
		pSiteReport->AdpCnt = AdpCnt;
		pSiteReport->PassCnt = 0;
		pSiteReport->FailCnt = 0;
		for (i = 0; i < AdpCnt; ++i) {
			pSiteReport->SktReport[i] = NULL;
		}
		pSiteReport->BtmBoardInfo.strHardwareOEM = "";
		pSiteReport->BtmBoardInfo.strHardwareUID = "";
		newSite.first = strSiteSN;
		newSite.second = pSiteReport;
		m_mapReport[projName.toStdString()].mapSiteReport.insert(newSite);
	}

__end:
	m_Mutex.unlock();
	return Ret;
}


tSktReport* ACReportRecord::GetSktReport(tSiteReport* pSiteReport, tSktInfo* pSktInfo, int AdpIdx)
{
	int i = 0;
	std::vector<tSktReport>& vSktReport = pSiteReport->vArrSktReport[AdpIdx];
	for (i = 0; i < (int)vSktReport.size(); ++i) {///在队列中命中这个座子,返回使用的座子信息
		if (vSktReport[i].SktUID == pSktInfo->UID) {
			return &vSktReport[i];
		}
	}
	if (pSktInfo->UID .isEmpty()) {///如果返回的全是FF,表示无效座子不进行放置
		return NULL;
	}
	///将新座子放到最后，并返回新座子的Report
	tSktReport SktReport;
	SktReport.PassCnt = 0;
	SktReport.FailCnt = 0;
	SktReport.SktUID = pSktInfo->UID;
	SktReport.mapErrCode.clear();
	vSktReport.push_back(SktReport);
	return &vSktReport[i];
}

int ACReportRecord::UpdateAdapterCounter(const QString& projName, const QString strSiteSN, uint64_t AdpID, uint32_t LimitCnt, uint32_t CurrentCnt)
{
	int Ret = ACERR_OK;
	m_Mutex.lock();
	std::map<QString, tSiteReport*>::iterator it;
	it = m_mapReport[projName.toStdString()].mapSiteReport.find(strSiteSN);
	if (it != m_mapReport[projName.toStdString()].mapSiteReport.end()) {///必须保证之前已经存在
	//	INT i;
		tSiteReport* pSiteReport = it->second;
		//	pSiteReport->vArrSktReport
	}
	else {
		Ret = ACERR_FAIL;
	}
	m_Mutex.unlock();
	return Ret;
}

int ACReportRecord::UpdateAdapterCounter(const QString& projName, const QString strSiteSN, uint32_t TotalCnt, uint32_t FailedCnt)
{
	int Ret = ACERR_OK;
	m_Mutex.lock();
	std::map<QString, tSiteReport*>::iterator it;
	it = m_mapReport[projName.toStdString()].mapSiteReport.find(strSiteSN);
	if (it != m_mapReport[projName.toStdString()].mapSiteReport.end()) {///必须保证之前已经存在
	//	INT i;
		tSiteReport* pSiteReport = it->second;
		pSiteReport->FailCnt = FailedCnt;
		pSiteReport->PassCnt = TotalCnt - FailedCnt;
		//	pSiteReport->vArrSktReport
	}
	else {
		Ret = ACERR_FAIL;
	}
	m_Mutex.unlock();
	return Ret;
}

int ACReportRecord::GetSiteReportCounter(const QString strSiteSN, int& PassCnt, int& FailCnt)
{
	int Ret = ACERR_OK;
	tSiteReport* pSiteReport = NULL;
	m_Mutex.lock();
	std::map<QString, tSiteReport*>::iterator it;
	it = m_Reporter.mapSiteReport.find(strSiteSN);
	if (it != m_Reporter.mapSiteReport.end()) {///必须保证之前已经存在
		int i;
		tSiteReport* pSiteReport = it->second;
		if (pSiteReport) {
			PassCnt = pSiteReport->PassCnt;
			FailCnt = pSiteReport->FailCnt;
		}
		else {
			Ret = ACERR_PARA;
			ALOG_ERROR("SiteReport=NULL, SiteSN:%s.", "CU", "--", strSiteSN);
		}
	}
	else {
		ALOG_INFO("[%s]:Not Add in ReportMap.", "CU", "--", strSiteSN);
		Ret = ACERR_PARA; goto __end;
	}


__end:
	m_Mutex.unlock();
	return Ret;
}


int ACReportRecord::UpdateSite(const QString& projName, const QString strSiteSN, uint32_t ErrCode[], tSktInfo SktInfo[], int AdpCnt, uchar AdpActive[])
{
	int Ret = ACERR_OK;
	m_Mutex.lock();
	std::map<QString, tSiteReport*>::iterator it;
	it = m_mapReport[projName.toStdString()].mapSiteReport.find(strSiteSN);
	if (it != m_mapReport[projName.toStdString()].mapSiteReport.end()) {///必须保证之前已经存在
		tSiteReport* pSiteReport = it->second;
		for (int i = 0; i < AdpCnt; ++i) {
			pSiteReport->SktReport[i] = GetSktReport(pSiteReport, SktInfo + i, i);
			if (pSiteReport->SktReport[i] == NULL) {
				//m_pLogMsg->PrintLog(LOGLEVEL_LOG,"[%s][%d]not exist",strSiteSN, i);
				ALOG_ERROR("Adapter Report Pointer=NULL, SKT_UID[%d]:0x%I64X.", "CU", "--", i + 1, SktInfo[i].UID);

				if (AdpActive[i] == 1) { //使能的也要加上计数
					ALOG_WARN("SiteSN:%s, Skt[%d] is enable, but uid is null, result:0x%04X.", "CU", "--", strSiteSN, i + 1, ErrCode[i]);
					if (ErrCode[i] == DEVERR_PASS) {
						pSiteReport->PassCnt++;
					}
					else {
						pSiteReport->FailCnt++;
					}
				}

				continue;
			}
			//m_pLogMsg->PrintLog(LOGLEVEL_WARNING,"Report ErroCode[%d]=0x%X",i,ErrCode[i]);
			pSiteReport->SktReport[i]->AdpLimitCnt = SktInfo[i].LimitCnt;
			pSiteReport->SktReport[i]->AdpCurrentCnt = SktInfo[i].CurCnt;
			pSiteReport->SktReport[i]->AdpFailCnt = SktInfo[i].FailCnt;

			if (ErrCode[i] == DEVERR_PASS) {
				pSiteReport->SktReport[i]->PassCnt++;
				pSiteReport->PassCnt++;
				ALOG_INFO("[%s][%d]Result: Pass.", "CU", "--", strSiteSN, i);
			}
			else {
				ALOG_INFO("[%s][%d]Result: NG.", "CU", "--", strSiteSN, i);
				if (ErrCode[i] & ECODE_ID) {
					std::map<uint32_t, uint32_t>::iterator it;
					pSiteReport->SktReport[i]->FailCnt++;
					pSiteReport->FailCnt++;
					ALOG_INFO("[%s][%d]Result: Fail.", "CU", "--", strSiteSN, i);

					it = pSiteReport->SktReport[i]->mapErrCode.find(ErrCode[i]);
					if (it != pSiteReport->SktReport[i]->mapErrCode.end()) {///已经加入到错误列表中
						it->second++;///错误次数自增
					}
					else {///新增到错误列表中
						std::pair<uint32_t, uint32_t> ErrPair;
						ErrPair.first = ErrCode[i];
						ErrPair.second = 1;
						pSiteReport->SktReport[i]->mapErrCode.insert(ErrPair);
					}
				}
				else {
					ALOG_INFO("[%s][%d]Result: Error.", "CU", "--", strSiteSN, i);
					if (ErrCode[i] != 0) {
						ALOG_WARN("Reporter UpdateSite %s, SKT[%d] ErrCodeNotSupport=0x%X.", "CU", "--",
							pSiteReport->strSiteAlias, i + 1, ErrCode[i]);
					}
				}
			}

		}
	}
	else {
		ALOG_ERROR("Reporter UpdateSite Fail, %s Not Add in ReportMap.", "CU", "--", strSiteSN);
		Ret = ACERR_PARA; goto __end;
	}


__end:
	m_Mutex.unlock();
	return Ret;
}

int ACReportRecord::ClearStatistic()
{
	int Ret = ACERR_OK;
	m_Mutex.lock();

	ALOG_WARN("Clear ReportMap.", "CU", "--");

	for (auto& projIter : m_mapReport) {
		std::map<QString, tSiteReport*>::iterator it;
		if (projIter.second.mapSiteReport.size() > 0) {
			for (it = projIter.second.mapSiteReport.begin(); it != projIter.second.mapSiteReport.end(); it++) {
				tSiteReport* pSiteReport = it->second;
				if (pSiteReport) {
					for (int i = 0; i < pSiteReport->AdpCnt; ++i) {
						std::vector<tSktReport>& vSktReport = pSiteReport->vArrSktReport[i];
						pSiteReport->SktReport[i] = NULL;
						vSktReport.clear();
					}
					delete pSiteReport;
				}
			}
		}
		projIter.second.mapSiteReport.clear();
		projIter.second.strCurFuncmode = "";
		projIter.second.strCmdSequence = "";
	}

	m_Mutex.unlock();
	return Ret;
}

int ACReportRecord::GetSktCounter(tSiteReport* pSiteReport, int SktIdx, uint32_t& PassCnt, uint32_t& FailCnt)
{
	std::vector<tSktReport>& vSktReport = pSiteReport->vArrSktReport[SktIdx];
	PassCnt = 0;
	FailCnt = 0;
	for (int i = 0; i < vSktReport.size(); ++i) {
		if (vSktReport[i].SktUID.isEmpty()) {//有非法座子，不进行统计
			ALOG_WARN("AdapterSN is null, vectorsize:%d, index:%d, SktIdx[%d]: passcnt:%u, failcnt:%u.", "CU", "--",
				vSktReport.size(), i, SktIdx + 1, vSktReport[i].PassCnt, vSktReport[i].FailCnt);
		}
		PassCnt += vSktReport[i].PassCnt;
		FailCnt += vSktReport[i].FailCnt;
	}
	return 0;
}

void ACReportRecord::ReportBegin(ACHtmlLogWriter* m_pLogWriter)
{
	QString strTime;
	strTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
	QString strLog = QString("\
		<div id = \"projectreport\">\r\n\
		<p class='logtime'>[%1] &nbsp;</p><p class = 'SectionHeader button'><span>+</span>Project Report</p><br/>\r\n\
		<div style = \"display:none; \">\r\n\
		<table><tr><td style='font:12pt Consolas; font-weight:bold;'>Project Report %s </td></tr><tr><td><hr/></td></tr></table>\r\n\
		").arg(strTime);

	m_pLogWriter->WriteLog(strLog);
}

void ACReportRecord::ReportBeginAlone(ACHtmlLogWriter* m_pLogWriter)
{
	QString strTime;
	strTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
	QString strLog = QString("\
		<div id = \"projectreport\">\r\n\
		<table><tr><td style='font:12pt Consolas; font-weight:bold;'>Project Report %1 </td></tr><tr><td><hr/></td></tr></table>\r\n\
		").arg(strTime);

	m_pLogWriter->WriteLog(strLog);
}


void ACReportRecord::ReportDumpProjectInfo(ACHtmlLogWriter* m_pLogWriter)
{
	QString strTmp;
	QString strBegin = "\
		<table><tr><td style ='font:10pt Consolas;font-weight:bold;'>Project Basic Information</td></tr><tr><td>\
		<hr style = 'background:#0189B4;height:3px;width:90%;text-align:left;float:left;display:block;'></td></tr></table>\r\n\
		<table>\r\n\
		";
	m_pLogWriter->WriteLog(strBegin);

	for (auto& projIter : m_mapReport) {
		projIter.second.strCurTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
	
	strTmp = QString("\
		<tr><td>Project Name:</td><td>%1</td></tr>\r\n\
		<tr><td>Build Version:</td><td>%2</td></tr>\r\n\
		<tr><td>Build Time:</td><td>%3</td></tr>\r\n\
		<tr><td>Time When Loaded:</td><td>%4</td></tr>\r\n\
		<tr><td>Current Time:</td><td>%5</td></tr>\r\n\
		<tr><td>Build On:</td><td>%6</td></tr>\r\n\
		<tr><td>ChipName:</td><td>%7</td></tr>\r\n\
		<tr><td>Manufacturer:</td><td>%8</td></tr>\r\n\
		<tr><td>Package:</td><td>%9</td></tr>\r\n\
		<tr><td>Adapter:</td><td>%10</td></tr>\r\n\
		<tr><td>CheckSum:</td><td>%11</td></tr>\r\n\
		<tr><td>CheckSum Type:</td><td>%12</td></tr>\r\n\
		<tr><td>Description:</td><td>%13</td></tr>\r\n\
		<tr><td>Author:</td><td>%14</td></tr>\r\n\
		<tr><td>FuncMode:</td><td>%15</td></tr>\r\n\
		<tr><td>ExecPC:</td><td>%16</td></tr>\r\n\
		<tr><td>Command Sequence:</td><td>%17</td></tr>\r\n\
		").arg(
		projIter.second.strProjName,
		projIter.second.strBulidVer,
		projIter.second.strProjCreateTime,
		projIter.second.strProjLoadTime,
		projIter.second.strCurTime,
		projIter.second.strDevType,
		projIter.second.strICName,
		projIter.second.strICManu,
		projIter.second.strICPack,
		projIter.second.strAdapterName,
		projIter.second.strChksum,
		projIter.second.strChksumType,
		projIter.second.strProjDescriptor,
		projIter.second.strPCLocation,
		projIter.second.strCurFuncmode,
		projIter.second.strExecPC,
		projIter.second.strCmdSequence);

	m_pLogWriter->WriteLog(strTmp);


	}	

	QString strEnd = "\
		</table><br/>\r\n\
		";

	m_pLogWriter->WriteLog(strEnd);
}

void ACReportRecord::ReportDumpProjOrgFile(ACHtmlLogWriter* m_pLogWriter)
{
	QString strTmp;
	QString strBegin = "\
		<table><tr><td style ='font:10pt Consolas;font-weight:bold;'>Original Files Imported</td></tr><tr><td><hr style = 'background:#0189B4;height:3px;width:90%;text-align:left;float:left;display:block;'></td></tr></table>\r\n\
		<table>\r\n\
		";
	m_pLogWriter->WriteLog(strBegin);

	for (auto& projIter : m_mapReport) {
		for (int i = 0; i < projIter.second.vecBinFile.size(); ++i) {
			strTmp = QString("<td style ='font:8pt Consolas;font-weight:bold;'>Original File[%1]: %2</td></tr><tr>").arg(i).arg(QString::fromStdString(projIter.second.vecBinFile[i]));
			m_pLogWriter->WriteLog(strTmp);
		}
	}
	QString strEnd = "\
	</table><br/>\r\n\
	";
	m_pLogWriter->WriteLog(strEnd);
}

void ACReportRecord::ReportEndAlone(ACHtmlLogWriter* m_pLogWriter)
{
	QString strEnd = "\
		<tr><td><hr/></td></tr>\r\n\
		<br/>\r\n\
		</div>\r\n\
		";
	m_pLogWriter->WriteLog(strEnd);
}

void ACReportRecord::ReportEnd(ACHtmlLogWriter* m_pLogWriter)
{
	QString strEnd = "\
		<tr><td><hr/></td></tr>\r\n\
		</div>\r\n\
		<br/>\r\n\
		</div>\r\n\
		";
	m_pLogWriter->WriteLog(strEnd);
}

bool ACReportRecord::GetOverAllStatistics_EachErr(std::map<QString, uint32_t>& ErrInfoStatistis)
{
	bool Ret = true;
	int AdpIdx = 0;
	std::map<uint32_t, uint32_t> map_Temp; ///错误码统计，UINT1为错误码，UINT为个数
	std::map<uint32_t, uint32_t>::iterator map_Temp_it;
	std::map<QString, tSiteReport*>::iterator it;
	ErrInfoStatistis.clear();
	///遍历所有站点
	for (auto& projIter : m_mapReport) {
		for (it = projIter.second.mapSiteReport.begin(); it != projIter.second.mapSiteReport.end(); ++it) {
			tSiteReport* pSiteReporter = it->second;
			///遍历每个站点的SKT位置
			for (AdpIdx = 0; AdpIdx < pSiteReporter->AdpCnt; AdpIdx++) {
				std::vector<tSktReport>& SktReport = pSiteReporter->vArrSktReport[AdpIdx];
				///遍历每个SKT的报告
				for (int i = 0; i < SktReport.size(); ++i) {
					std::map<uint32_t, uint32_t>::iterator itErr;
					std::map<uint32_t, uint32_t>& TempmapErrCode = SktReport[i].mapErrCode;
					for (itErr = TempmapErrCode.begin(); itErr != TempmapErrCode.end(); ++itErr) {
						map_Temp_it = map_Temp.find(itErr->first);
						if (map_Temp_it != map_Temp.end()) {///已经找到
							map_Temp_it->second += itErr->second; ///把错误次数加到对应统计次数上面
						}
						else {///没有找到，则需要新增
							map_Temp.insert(std::pair<uint32_t, uint32_t>(itErr->first, itErr->second));
						}
					}

				}
			}
		}
	}

	for (map_Temp_it = map_Temp.begin(); map_Temp_it != map_Temp.end(); ++map_Temp_it) {
		QString strMsg = GetErrMsg(map_Temp_it->first);
		ErrInfoStatistis.insert(std::pair<QString, uint32_t>(strMsg, map_Temp_it->second));
	}
	return Ret;
}


bool ACReportRecord::ReportDumpOverAllStatistic(ACHtmlLogWriter* m_pLogWriter)
{
	std::map<QString, uint32_t>::iterator ErrInfo_It;
	std::map<QString, uint32_t>ErrInfoStatistis;
	QString strBegin = "\
		<table><tr><td style ='font:10pt Consolas;font-weight:bold;'>Overall Statistics</td></tr><tr><td><hr style = 'background:#0189B4;height:3px;width:90%;text-align:left;float:left;display:block;'></td></tr></table>\r\n\
		<table>\r\n\
		";
	m_pLogWriter->WriteLog(strBegin);

	uint32_t AllSitePass = 0, AllSiteFail = 0;
	uint32_t TotalCnt;
	std::map<QString, tSiteReport*>::iterator it;
	GetOverAllStatistics_EachErr(ErrInfoStatistis);
	for (auto& projIter : m_mapReport) {
		for (it = projIter.second.mapSiteReport.begin(); it != projIter.second.mapSiteReport.end(); ++it) {
			tSiteReport* pSiteReporter = it->second;
			AllSitePass += pSiteReporter->PassCnt;
			AllSiteFail += pSiteReporter->FailCnt;
		}

		float YieldRate;
		QString strTmp;
		QString strErrInfo;
		TotalCnt = AllSiteFail + AllSitePass;
		YieldRate = (TotalCnt == 0) ? float(0.000) : (float(AllSitePass) * 100 / float(TotalCnt));
		strTmp = QString("\
		<tr><td width=\"15%%\" >Pass:</td><td>%1</td></tr>\r\n\
		<tr><td width=\"15%%\">Fail:</td><td>%2</td></tr>\r\n\
		<tr><td width=\"15%%\">Total:</td><td>%3</td></tr>\r\n\
		<tr><td width=\"15%%\">YieldRate:</td><td>%4</td></tr>\r\n\
		").arg(AllSitePass).arg(AllSiteFail).arg(TotalCnt).arg(YieldRate, 0, 'f', 3);

		m_pLogWriter->WriteLog(strTmp);

		QString strEnd = "\
		</table><br/>\r\n\
		";
		m_pLogWriter->WriteLog(strEnd);

		if (ErrInfoStatistis.size() > 0) {
			/////创建SKT错误统计信息表
			strTmp = "<table><tr><td style ='font:8pt Consolas;font-weight:bold;'>All Failure Information</td></tr>\
			   <tr><td><hr style = 'background:#0189B4;height:1px;width:60%;text-align:left;float:left;'></td></tr></table>\r\n\
			   <table class='subtbl'>";
			m_pLogWriter->WriteLog(strTmp);

			strTmp = "<tr><th>ErrMsg</th><th>Counter</th></tr>\r\n";
			m_pLogWriter->WriteLog(strTmp);

			for (ErrInfo_It = ErrInfoStatistis.begin(); ErrInfo_It != ErrInfoStatistis.end(); ErrInfo_It++) {
				strTmp = QString("<tr><td bgcolor=#DDDDDD>%1</td><td bgcolor=#DDDDDD>%2</td></tr>\r\n").arg(ErrInfo_It->first, ErrInfo_It->second);
				m_pLogWriter->WriteLog(strTmp);
			}
			strEnd = "</table><br/>\r\n";
			m_pLogWriter->WriteLog(strEnd);
		}
	}
	return true;
}


bool ACReportRecord::DumpOverAllStatisticToJson(nlohmann::json& OverAllStatistic)
{
	uint32_t AllSitePass = 0, AllSiteFail = 0;
	uint32_t TotalCnt;
	QString strTmp;
	std::map<QString, tSiteReport*>::iterator it;
	for (it = m_Reporter.mapSiteReport.begin(); it != m_Reporter.mapSiteReport.end(); ++it) {
		tSiteReport* pSiteReporter = it->second;
		AllSitePass += pSiteReporter->PassCnt;
		AllSiteFail += pSiteReporter->FailCnt;
	}
	float YieldRate;
	TotalCnt = AllSiteFail + AllSitePass;
	YieldRate = (TotalCnt == 0) ? float(0.000) : (float(AllSitePass) * 100 / float(TotalCnt));

	OverAllStatistic["Pass"] = AllSitePass;
	OverAllStatistic["Fail"] = AllSiteFail;
	OverAllStatistic["Total"] = TotalCnt;

	strTmp = QString("%1").arg(YieldRate, 0, 'f', 3);
	OverAllStatistic["YieldRate"] = strTmp.toStdString();
	return true;
}

bool ACReportRecord::ReportDumpSiteStatistic(ACHtmlLogWriter* m_pLogWriter)
{
	QString strEnd;
	std::map<QString, tSiteReport*>::iterator it;
	for (auto& projIter : m_mapReport) {
		for (it = projIter.second.mapSiteReport.begin(); it != projIter.second.mapSiteReport.end(); ++it) {
			QString strBegin;
			int AdpIdx;
			tSiteReport* pSiteReporter = it->second;
			uint32_t SktPassCnt[ADAPTER_MAXNUM], SktFailCnt[ADAPTER_MAXNUM];

			/////创建站点信息表
			strBegin = QString("\
		<table><tr><td style ='font:10pt Consolas;font-weight:bold;'>SiteSN:%1&nbsp&nbsp&nbsp&nbsp&nbspSitiAlias:%2 &nbsp&nbsp&nbsp&nbsp&nbspStatistics</td></tr>\
		<tr><td><hr style = 'background:#0189B4;height:3px;width:90%%;text-align:left;float:left;display:block;'></td></tr>\
		</table>\r\n\
		<table>\r\n\
		").arg(pSiteReporter->strSiteSN, pSiteReporter->strSiteAlias);

			m_pLogWriter->WriteLog(strBegin);

			QString strTmp;
			uint32_t TotalCnt;
			float YieldRate;
			TotalCnt = pSiteReporter->PassCnt + pSiteReporter->FailCnt;
			YieldRate = (TotalCnt == 0) ? float(0.000) : (float(pSiteReporter->PassCnt) * 100 / float(TotalCnt));
			strTmp = QString("\
		<tr><td width=\"15%%\" >Pass:</td><td>%1</td></tr>\r\n\
		<tr><td width=\"15%%\">Fail:</td><td>%2</td></tr>\r\n\
		<tr><td width=\"15%%\">Total:</td><td>%3</td></tr>\r\n\
		<tr><td width=\"15%%\">YieldRate:</td><td>%4</td></tr>\r\n\
		").arg(pSiteReporter->PassCnt).arg(pSiteReporter->FailCnt).arg(TotalCnt).arg(YieldRate, 0, 'f', 3);
			m_pLogWriter->WriteLog(strTmp);

			//if (pSiteReporter->BtmBoardInfo.strHardwareUID != "") {
			//	strTmp = QString("<tr><td>Bottom Board:</td><td>%1[%2]</td></tr>\r\n<tr><td>&nbsp</td><td>&nbsp</td></tr>\r\n")
			//		.arg(QString::fromStdString(pSiteReporter->BtmBoardInfo.strHardwareOEM), QString::fromStdString(pSiteReporter->BtmBoardInfo.strHardwareUID));
			//}
			//else {
			//	strTmp = "<tr><td>Bottom Board:</td><td>None</td></tr>\r\n<tr><td>&nbsp</td><td>&nbsp</td></tr>\r\n";
			//}
			//m_pLogWriter->WriteLog(strTmp);
			strEnd = "</table>\r\n";
			m_pLogWriter->WriteLog(strEnd);

			if (false) {
				/////创建SKT信息表
				strTmp = "<table><tr><td style ='font:8pt Consolas;font-weight:bold;'>SKT Statistics</td></tr>\
<tr><td><hr style = 'background:#0189B4;height:1px;width:60%;text-align:left;float:left;'></td></tr></table>\r\n\
<table class='subtbl'>";
				m_pLogWriter->WriteLog(strTmp);

				strTmp = "<tr><th>SKT Index</th><th>AdapterSN</th><th>Pass</th><th>Failed</th><th>Total</th></tr>\r\n";
				m_pLogWriter->WriteLog(strTmp);

				for (AdpIdx = 0; AdpIdx < pSiteReporter->AdpCnt; AdpIdx++) {
					std::vector<tSktReport>& SktReport = pSiteReporter->vArrSktReport[AdpIdx];
					GetSktCounter(pSiteReporter, AdpIdx, SktPassCnt[AdpIdx], SktFailCnt[AdpIdx]);

					strTmp = QString("<tr><td bgcolor=#DDDDDD>SKT[%1]</td><td bgcolor=#DDDDDD>&nbsp</td><td bgcolor=#DDDDDD>%2</td><td bgcolor=#DDDDDD>%3</td><td bgcolor=#DDDDDD>%4</td></tr>\r\n").arg(
						QString::number(AdpIdx + 1), QString::number(SktPassCnt[AdpIdx]), QString::number(SktFailCnt[AdpIdx]), QString::number(SktFailCnt[AdpIdx] + SktPassCnt[AdpIdx]));
					m_pLogWriter->WriteLog(strTmp);
					for (int i = 0; i < SktReport.size(); ++i) {
						if (SktReport[i].SktUID.isEmpty()) {///有非法座子，不进行统计
							continue;
						}
						strTmp = QString("<tr><td>&nbsp</td><td>ADP[%1]</td><td>%2</td><td>%3</td><td>%4</td></tr>\r\n")
							.arg(SktReport[i].SktUID, QString::number(SktReport[i].PassCnt), QString::number(SktReport[i].FailCnt), QString::number(SktReport[i].PassCnt + SktReport[i].FailCnt));
						m_pLogWriter->WriteLog(strTmp);
					}
				}

				strEnd = "</table><br/>\r\n";
				m_pLogWriter->WriteLog(strEnd);

				strTmp = "<table><tr><td style ='font:8pt Consolas;font-weight:bold;'>Fail Status</td></tr>\
<tr><td><hr style = 'background:#0189B4;height:1px;width:60%;text-align:left;float:left;'></td></tr></table>\r\n\
<table class='subtbl'>";
				m_pLogWriter->WriteLog(strTmp);

				strTmp = "<tr><th>SKT Index</th><th>AdapterSN</th><th>ErrNo</th><th>Counter</th><th>ErrMsg</th></tr>\r\n";
				m_pLogWriter->WriteLog(strTmp);


				for (AdpIdx = 0; AdpIdx < pSiteReporter->AdpCnt; AdpIdx++) {
					std::vector<tSktReport>& vSktReport = pSiteReporter->vArrSktReport[AdpIdx];
					if (SktFailCnt[AdpIdx] == 0) {///没有错误，不需要
						continue;
					}
					strTmp = QString("<tr><td bgcolor=#DDDDDD>SKT[%d]</td><td bgcolor=#DDDDDD>&nbsp</td><td bgcolor=#DDDDDD>&nbsp</td><td bgcolor=#DDDDDD>&nbsp</td><td bgcolor=#DDDDDD>&nbsp</td></tr>\r\n").arg(
						AdpIdx + 1);
					m_pLogWriter->WriteLog(strTmp);
					for (int j = 0; j < vSktReport.size(); ++j) {
						tSktReport& SktReport = vSktReport[j];
						std::map<uint32_t, uint32_t>::iterator itErr;
						if (SktReport.mapErrCode.size()) {
							strTmp = QString("<tr><td>&nbsp</td><td bgcolor=#DDDDDD>ADP[%I64X]</td><td bgcolor=#DDDDDD>&nbsp</td><td bgcolor=#DDDDDD>&nbsp</td><td bgcolor=#DDDDDD>&nbsp</td></tr>\r\n").arg(
								SktReport.SktUID);
							m_pLogWriter->WriteLog(strTmp);
						}
						for (itErr = SktReport.mapErrCode.begin(); itErr != SktReport.mapErrCode.end(); itErr++) {
							strTmp = QString("<tr><td>&nbsp</td><td>&nbsp</td><td>0x%04X</td><td>%d</td><td>%s</td></tr>\r\n").arg(
								QString::number(itErr->first), QString::number(itErr->second), GetErrMsg(itErr->first));
							m_pLogWriter->WriteLog(strTmp);
						}
					}

				}
				strEnd = "</table><br/>\r\n";
				m_pLogWriter->WriteLog(strEnd);
			}
		}
	}
	return true;
}


bool ACReportRecord::DumpSiteStatisticToJson(nlohmann::json& OverallStatistic)
{
	int nMatchIdx = 0;
	//CDevMng& GDevMng = GetGlobalDevMng();

	std::map<QString, tSiteReport*>::iterator it;
	for (it = m_Reporter.mapSiteReport.begin(); it != m_Reporter.mapSiteReport.end(); ++it) {
		nlohmann::json OnesiteStatistic;
		QString strTmp;
		int AdpIdx;
		tSiteReport* pSiteReporter = it->second;
		uint32_t SktPassCnt[ADAPTER_MAXNUM], SktFailCnt[ADAPTER_MAXNUM];
		uint32_t TotalCnt;
		float YieldRate;
		TotalCnt = pSiteReporter->PassCnt + pSiteReporter->FailCnt;
		YieldRate = (TotalCnt == 0) ? float(0.000) : (float(pSiteReporter->PassCnt) * 100 / float(TotalCnt));

		/////创建站点信息表
		OnesiteStatistic["SiteSN"] = pSiteReporter->strSiteSN.toStdString();
		OnesiteStatistic["SiteAlias"] = pSiteReporter->strSiteAlias.toStdString();
		OnesiteStatistic["Pass"] = pSiteReporter->PassCnt;
		OnesiteStatistic["Fail"] = pSiteReporter->FailCnt;
		OnesiteStatistic["Total"] = TotalCnt;

		strTmp = QString("%1").arg(YieldRate, 0, 'f', 3);
		OnesiteStatistic["YieldRate"] = strTmp.toStdString();


		if (pSiteReporter->BtmBoardInfo.strHardwareUID != "") {
			strTmp = QString::fromStdString(pSiteReporter->BtmBoardInfo.strHardwareUID);
		}
		else {
			strTmp = "None";
		}
		OnesiteStatistic["BottomBoard"] = strTmp.toStdString();

		/////创建SKT信息表

		///////////////
		//if (GDevMng.m_GlbSetting.nCntInfoEn == TRUE) {
		//	bool bMatch = false;
		//	DEVINFO* pDevInfo = NULL;
		//	int nAttaCnt = GDevMng.GetDevCnt(CDevMng::GROUP_ATTACNED);
		//	RECT DlgRect;
		//	for (int nAtta = 0; nAtta < nAttaCnt; nAtta++) {
		//		pDevInfo = GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED, nAtta);
		//		if (pDevInfo == NULL) {
		//			continue;
		//		}
		//		if (pSiteReporter->strSiteSN.CompareNoCase(pDevInfo->DevSN) == 0) {
		//			bMatch = true;
		//			nMatchIdx = nAtta;
		//			break;
		//		}
		//	}
		//	if (bMatch) {
		//		pDevInfo->pAprogDev->GetBtmBoardInfo(NULL);
		//		if (pDevInfo->pAprogDev->IsLargerThanVersion(VERSION_0_75)) {
		//			pDevInfo->pAprogDev->DoCustom(TAG_GET_ALLSKBSMPINFO_EXT, NULL, 0);
		//		}
		//		else {
		//			pDevInfo->pAprogDev->DoCustom(TAG_GETALLSKBINFO, NULL, 0);
		//		}
		//	}

		//}
		//////////////

		//for (AdpIdx = 0; AdpIdx < pSiteReporter->AdpCnt; AdpIdx++) {
		//	Json::Value OneSKTStatistics;
		//	std::vector<tSktReport>& SktReport = pSiteReporter->vArrSktReport[AdpIdx];
		//	INT i;
		//	GetSktCounter(pSiteReporter, AdpIdx, SktPassCnt[AdpIdx], SktFailCnt[AdpIdx]);

		//	OneSKTStatistics["SKTIdx"] = Json::Value(AdpIdx + 1);
		//	OneSKTStatistics["Pass"] = Json::Value(SktPassCnt[AdpIdx]);
		//	OneSKTStatistics["Fail"] = Json::Value(SktFailCnt[AdpIdx]);

		//	if (GDevMng.m_GlbSetting.nCntInfoEn == TRUE) {
		//		DEVINFO* pDevInfo = GDevMng.GetDevInfo(CDevMng::GROUP_ATTACNED, nMatchIdx);
		//		if (pDevInfo) {
		//			OneSKTStatistics["Current"] = pDevInfo->SktInfo[AdpIdx].CurCnt;
		//			OneSKTStatistics["Limited"] = pDevInfo->SktInfo[AdpIdx].LimitCnt;
		//			OneSKTStatistics["FailCnt"] = pDevInfo->SktInfo[AdpIdx].FailCnt;
		//		}
		//	}

		//	for (i = 0; i < (INT)SktReport.size(); ++i) {
		//		Json::Value OneSKT;
		//		CString strTmp1;
		//		strTmp1.Format("%016I64X", SktReport[i].SktUID);
		//		OneSKT["AdpSN"] = Json::Value(strTmp1);
		//		OneSKT["Pass"] = Json::Value(SktReport[i].PassCnt);
		//		OneSKT["Fail"] = Json::Value(SktReport[i].FailCnt);
		//		OneSKTStatistics["AdpInfo"].append(OneSKT);
		//	}

		//	OnesiteStatistic["SKTStatistics"].append(OneSKTStatistics);
		//}

		/////录入失败信息
		//for (AdpIdx = 0; AdpIdx < pSiteReporter->AdpCnt; AdpIdx++) {

		//	INT j;
		//	std::vector<tSktReport>& vSktReport = pSiteReporter->vArrSktReport[AdpIdx];
		//	if (SktFailCnt[AdpIdx] == 0) {///没有错误，不需要
		//		continue;
		//	}
		//	for (j = 0; j < (INT)vSktReport.size(); ++j) {
		//		tSktReport& SktReport = vSktReport[j];
		//		std::map<UINT, UINT>::iterator itErr;
		//		for (itErr = SktReport.mapErrCode.begin(); itErr != SktReport.mapErrCode.end(); itErr++) {
		//			Json::Value OneFailStatus;
		//			OneFailStatus["SKTIdx"] = Json::Value(AdpIdx + 1);
		//			strTmp.Format("%016I64X", SktReport.SktUID);
		//			OneFailStatus["ADPSN"] = Json::Value(strTmp);

		//			strTmp.Format("0x%04X", itErr->first);
		//			OneFailStatus["ErrNo"] = Json::Value(strTmp);
		//			OneFailStatus["Counter"] = Json::Value(itErr->second);
		//			OneFailStatus["ErrMsg"] = Json::Value(GetErrMsg(itErr->first));
		//			OnesiteStatistic["FailStatus"].append(OneFailStatus);
		//		}
		//	}

		//}
		//OverallStatistic["SiteStatistics"].append(OnesiteStatistic);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
bool ACReportRecord::GetAdpSNPassFail()
{
	tAdpInfo AdpInfo;
	//CDevMng& GDevMng = GetGlobalDevMng();

	QString strTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");

	for (auto& projIter : m_mapReport) {
		std::map<QString, tSiteReport*>::iterator it;
		for (it = projIter.second.mapSiteReport.begin(); it != projIter.second.mapSiteReport.end(); ++it) {
			tSiteReport* pSiteReporter = it->second;

			for (int AdpIdx = 0; AdpIdx < pSiteReporter->AdpCnt; AdpIdx++) {
				std::vector<tSktReport>& SktReport = pSiteReporter->vArrSktReport[AdpIdx];
				for (int i = 0; i < SktReport.size(); ++i) {

					AdpInfo.strAdpSN = SktReport[i].SktUID;
					AdpInfo.strPass.QString::number(SktReport[i].PassCnt);
					AdpInfo.strFail = QString::number(SktReport[i].FailCnt);
					AdpInfo.strAdpName = m_Reporter.strAdapterName;
					AdpInfo.strWriteTime = strTime;
					m_mapAdpSNInfo.push_back(AdpInfo);
				}
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
bool ACReportRecord::TraverseFileCreate()
{
	//std::vector<tAdpInfo>::iterator it;
	//CString appPath = _T("");
	//CDevMng& GDevMng = GetGlobalDevMng();
	//SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appPath.GetBuffer(MAX_PATH));
	//appPath.ReleaseBuffer();
	//appPath.Format("%s\\MultiAprog", appPath);
	//if (!PathIsDirectory(appPath))
	//{
	//	if (!CreateDirectory(appPath, 0)) {
	//		return FALSE;
	//	}
	//}
	//CString IniFilePath = _T("");
	////文件命名为 ：  TQFP48(7x7)-S02(CS)_570000000EB8A348.csv
	//for (it = m_mapAdpSNInfo.begin(); it != m_mapAdpSNInfo.end(); ++it)
	//{
	//	IniFilePath.Format("%s\\%s_%s.csv", appPath, it->strAdpName, it->strAdpSN);

	//	if (_access(IniFilePath, 0) < 0)
	//	{
	//		std::ofstream outFile;
	//		outFile.open(IniFilePath, std::ios::out | std::ios::app);
	//		if (!outFile)
	//		{
	//			outFile.close();  return FALSE;
	//		}
	//		outFile << "日期" << ','
	//			<< "成功次数" << ','
	//			<< "失败次数" << std::endl;

	//		outFile << it->strWriteTime << ','
	//			<< it->strPass << ','
	//			<< it->strFail << std::endl;
	//		outFile.close();
	//	}
	//	else
	//	{
	//		std::ofstream outFile;
	//		outFile.open(IniFilePath, std::ios::out | std::ios::app);
	//		if (!outFile)
	//		{
	//			outFile.close();  return FALSE;
	//		}

	//		outFile << it->strWriteTime << ','
	//			<< it->strPass << ','
	//			<< it->strFail << std::endl;
	//		outFile.close();
	//	}
	//}

	return true;
}




bool ACReportRecord::ReportDumpSncInfo(ACHtmlLogWriter* m_pLogWriter)
{
	QString strTmp;
	QString strBegin = "\
<table><tr><td style ='font:10pt Consolas;font-weight:bold;'>SNC File Information</td></tr><tr><td><hr style = 'background:#0189B4;height:3px;width:90%;text-align:left;float:left;display:block;'></td></tr></table>\r\n\
<table>\r\n\
";
	m_pLogWriter->WriteLog(strBegin);

	for (auto& projIter : m_mapReport) {
		projIter.second.strCurTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
	}
	if (m_sncReporter.strSncFileName.isEmpty())
	{
		return false;
	}
	else
	{
		strTmp = QString("\
		<tr><td>SNCfile Name:</td><td>%1</td></tr>\r\n\
		<tr><td>StartIndex:</td><td>%2</td></tr>\r\n\
		<tr><td>TotalNumber:</td><td>%3</td></tr>\r\n\
		<tr><td>ChipName:</td><td>%4</td></tr>\r\n\
		<tr><td>strSNGenName:</td><td>%5</td></tr>\r\n\
		<tr><td>ProgramIndex:</td><td>%6</td></tr>\r\n\
		<tr><td>ReuseCnt:</td><td>%7</td></tr>\r\n\
		<tr><td>ReuseIndex:</td><td>%8</td></tr>\r\n\
		").arg(
			m_sncReporter.strSncFileName,
			QString::number(m_sncReporter.StartIndex),
			QString::number(m_sncReporter.TotalNumber),
			m_sncReporter.strChipName,
			m_sncReporter.strSNGenName,
			QString::number(m_sncReporter.ProgramIndex),
			QString::number(m_sncReporter.ReuseCnt),
			m_sncReporter.strReuseIndex
		);
		m_pLogWriter->WriteLog(strTmp);

		QString strEnd = "\
	</table><br/>\r\n\
	";
		m_pLogWriter->WriteLog(strEnd);

		m_sncReporter.strSncFileName = " ";
		m_sncReporter.StartIndex = 0;
		m_sncReporter.TotalNumber = 0;
		m_sncReporter.strChipName = " ";
		m_sncReporter.strSNGenName = " ";
		m_sncReporter.ProgramIndex = 0;
		m_sncReporter.ReuseCnt = 0;
		m_sncReporter.strReuseIndex = " ";
		return true;
	}

}


int ACReportRecord::WriteReportToAloneFile(ACHtmlLogWriter* m_pLogWriter)
{
	if (m_pLogWriter) {
		ReportBeginAlone(m_pLogWriter);
		ReportDumpProjectInfo(m_pLogWriter);
		ReportDumpProjOrgFile(m_pLogWriter);
		//ReportDumpSncInfo(m_pLogWriter);///snc
		ReportDumpOverAllStatistic(m_pLogWriter);
		ReportDumpSiteStatistic(m_pLogWriter);
		//ReportDumpAdpStatics(m_pLogWriter);
		ReportEndAlone(m_pLogWriter);
		return ACERR_OK;
	}
	else {
		return ACERR_FAIL;
	}
}

int ACReportRecord::ReportDumpAdpStatics(ACHtmlLogWriter* pLogWriter)
{
	QString strEnd;
	for (auto& projIter : m_mapReport) {
		std::map<QString, tSiteReport*>::iterator it;
		for (it = projIter.second.mapSiteReport.begin(); it != projIter.second.mapSiteReport.end(); ++it) {
			QString strBegin;
			int AdpIdx;
			QString strBtmBoardID;
			tSiteReport* pSiteReporter = it->second;
			//UINT SktPassCnt[ADAPTER_MAXNUM],SktFailCnt[ADAPTER_MAXNUM];
			if (pSiteReporter == NULL) {
				continue;
			}
			if (!pSiteReporter->BtmBoardInfo.strHardwareUID.empty()) {
				strBtmBoardID = QString::fromStdString(pSiteReporter->BtmBoardInfo.strHardwareUID);
			}
			else {
				strBtmBoardID = "None";
			}

			/////创建站点信息表
			strBegin = QString("\
					<table><tr><td style ='font:10pt Consolas;font-weight:bold;'>SiteSN:%1&nbsp&nbsp&nbsp&nbsp&nbspSitiAlias:%2&nbsp&nbsp&nbsp&nbsp&nbsp\
					   Adapters Information&nbsp&nbsp&nbsp&nbsp&nbspBtmBoard:%3</td></tr><tr><td>\
					<hr style = 'background:#0189B4;height:3px;width:90%%;text-align:left;float:left;display:block;'></td></tr></table>\r\n").arg(pSiteReporter->strSiteSN, pSiteReporter->strSiteAlias, strBtmBoardID);
			pLogWriter->WriteLog(strBegin);

			QString strTmp;
			strTmp = "<table class='subtbl'><tr><th>AdapterSN</th><th>Current Counter</th><th>Total Counter</th><th>Fail Counter</th></tr>";
			pLogWriter->WriteLog(strTmp);

			for (AdpIdx = 0; AdpIdx < pSiteReporter->AdpCnt; AdpIdx++) {
				std::vector<tSktReport>& SktReport = pSiteReporter->vArrSktReport[AdpIdx];
				for (int i = 0; i < SktReport.size(); ++i) {
					if (SktReport[i].SktUID.isEmpty()) {
						continue;
					}
					strTmp = QString("<tr><td bgcolor=#DDDDDD>ADP[%1]</td>\
							  <td bgcolor=#DDDDDD>%2</td><td bgcolor=#DDDDDD>%3</td><td bgcolor=#DDDDDD>%4</td></tr>")
						.arg(SktReport[i].SktUID, QString::number(SktReport[i].AdpCurrentCnt)
							, QString::number(SktReport[i].AdpLimitCnt), QString::number(SktReport[i].AdpFailCnt));
					pLogWriter->WriteLog(strTmp);
				}
			}
			strEnd = "</table><br/>\r\n";
			pLogWriter->WriteLog(strEnd);

		}
	}
	return true;
}

int ACReportRecord::WriteReportToLog(ACHtmlLogWriter* m_pLogWriter)
{
	if (m_pLogWriter) {
		ReportBegin(m_pLogWriter);
		ReportDumpProjectInfo(m_pLogWriter);
		ReportDumpProjOrgFile(m_pLogWriter);
		ReportDumpSncInfo(m_pLogWriter);///snc
		ReportDumpOverAllStatistic(m_pLogWriter);
		ReportDumpSiteStatistic(m_pLogWriter);
		ReportDumpAdpStatics(m_pLogWriter);
		ReportEnd(m_pLogWriter);
		return ACERR_OK;
	}
	else {
		return ACERR_FAIL;
	}
}

//int ACReportRecord::GetRunStatusReport(IReportlog* pReportlog, bool bIsDetail)
//{
//	INT Ret = ACERR_OK;
//	INT AdpIdx;
//	UINT AllSitePass = 0, AllSiteFail = 0;
//
//	std::map<CString, int> failInfoMap;
//	failInfoMap.clear();
//
//	if (bIsDetail) {
//		CTime CurTime = CTime::GetCurrentTime();
//		m_Reporter.strCurTime.Format("%d/%02d/%02d %02d:%02d:%02d", CurTime.GetYear(), CurTime.GetMonth(),
//			CurTime.GetDay(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
//		pReportlog->PrintLog("*********************************************************************");
//		pReportlog->PrintLog("**************     Project Basic Information   **********************");
//		pReportlog->PrintLog("*********************************************************************");
//		pReportlog->PrintLog("Project Name      : %s", m_Reporter.strProjName);
//		pReportlog->PrintLog("Build Version     : %s", m_Reporter.strBulidVer);
//		pReportlog->PrintLog("Build Time        : %s", m_Reporter.strProjCreateTime);
//		pReportlog->PrintLog("Time When Loaded  : %s", m_Reporter.strProjLoadTime);
//		pReportlog->PrintLog("Current Time      : %s", m_Reporter.strCurTime);
//		pReportlog->PrintLog("Build On          : %s", m_Reporter.strDevType);
//		pReportlog->PrintLog("ChipName          : %s", m_Reporter.strICName);
//		pReportlog->PrintLog("Package           : %s", m_Reporter.strICPack);
//		pReportlog->PrintLog("Adapter           : %s", m_Reporter.strAdapterName);
//		pReportlog->PrintLog("CheckSum          : %s", m_Reporter.strChksum);
//		pReportlog->PrintLog("CheckSum Type     : %s", m_Reporter.strChksumType);
//		pReportlog->PrintLog("Description       : %s", m_Reporter.strProjDescriptor);
//		pReportlog->PrintLog("Author            : %s", m_Reporter.strPCLocation);
//		pReportlog->PrintLog("FuncMode          : %s", m_Reporter.strCurFuncmode);
//		pReportlog->PrintLog("ExecPC            : %s", m_Reporter.strExecPC);
//		pReportlog->PrintLog("Command Sequence  : %s", m_Reporter.strCmdSequence);
//	}
//
//	std::map<CString, tSiteReport*>::iterator it;
//	for (it = m_Reporter.mapSiteReport.begin(); it != m_Reporter.mapSiteReport.end(); ++it) {
//		tSiteReport* pSiteReporter = it->second;
//		AllSitePass += pSiteReporter->PassCnt;
//		AllSiteFail += pSiteReporter->FailCnt;
//	}
//
//	if (!bIsDetail) {
//		pReportlog->PrintLog("Total Pass: %-8d", AllSitePass);
//		pReportlog->PrintLog("Total Fail: %-8d", AllSiteFail);
//	}
//
//	for (it = m_Reporter.mapSiteReport.begin(); it != m_Reporter.mapSiteReport.end(); ++it) {
//		tSiteReport* pSiteReporter = it->second;
//		UINT SktPassCnt[ADAPTER_MAXNUM], SktFailCnt[ADAPTER_MAXNUM];
//
//		if (bIsDetail) {
//			pReportlog->PrintLog("*********************************************************************");
//			pReportlog->PrintLog("SiteSN       : %s    SiteAlias : %s", pSiteReporter->strSiteSN, pSiteReporter->strSiteAlias);
//			pReportlog->PrintLog("Gross        : Pass=%-8d  Fail=%-8d  Total=%-8d",
//				pSiteReporter->PassCnt, pSiteReporter->FailCnt, pSiteReporter->PassCnt + pSiteReporter->FailCnt);
//
//			if (pSiteReporter->BtmBoardInfo.BoardUID != (UINT64)-1) {
//				pReportlog->PrintLog("Bottom Board : %s[%I64X]", pSiteReporter->BtmBoardInfo.BoardName,
//					pSiteReporter->BtmBoardInfo.BoardUID);
//			}
//			else {
//				pReportlog->PrintLog("Bottom Board : None");
//			}
//		}
//
//		for (AdpIdx = 0; AdpIdx < pSiteReporter->AdpCnt; AdpIdx++) {
//			std::vector<tSktReport>& SktReport = pSiteReporter->vArrSktReport[AdpIdx];
//			INT i;
//			GetSktCounter(pSiteReporter, AdpIdx, SktPassCnt[AdpIdx], SktFailCnt[AdpIdx]);
//
//			if (bIsDetail) {
//				pReportlog->PrintLog("SKT[%d]   Pass=%-8d  Fail=%-8d  Total=%-8d",
//					AdpIdx + 1, SktPassCnt[AdpIdx], SktFailCnt[AdpIdx], SktFailCnt[AdpIdx] + SktPassCnt[AdpIdx]);
//				for (i = 0; i < (INT)SktReport.size(); ++i) {
//					pReportlog->PrintLog("         ADP[%I64X]   Pass=%-8d  Fail=%-8d  Total=%-8d",
//						SktReport[i].SktUID, SktReport[i].PassCnt, SktReport[i].FailCnt,
//						SktReport[i].PassCnt + SktReport[i].FailCnt);
//				}
//			}
//		}
//
//		if (bIsDetail) {
//			pReportlog->PrintLog("---------------------------------------------------------------------");
//			pReportlog->PrintLog("Fail Status:");
//		}
//
//		for (AdpIdx = 0; AdpIdx < pSiteReporter->AdpCnt; AdpIdx++) {
//			INT j;
//			std::vector<tSktReport>& vSktReport = pSiteReporter->vArrSktReport[AdpIdx];
//			if (SktFailCnt[AdpIdx] == 0) {///没有错误，不需要
//				continue;
//			}
//			if (bIsDetail) {
//				pReportlog->PrintLog("SKT[%d]", AdpIdx + 1);
//			}
//
//			for (j = 0; j < (INT)vSktReport.size(); ++j) {
//				tSktReport& SktReport = vSktReport[j];
//				std::map<UINT, UINT>::iterator itErr;
//				if (bIsDetail) {
//					pReportlog->PrintLog("    ADP[%016I64X]", SktReport.SktUID);
//				}
//				for (itErr = SktReport.mapErrCode.begin(); itErr != SktReport.mapErrCode.end(); itErr++) {
//					CString strErr;
//					strErr.Format("%s", GetErrMsg(itErr->first));
//					int nFailCnt = itErr->second;
//
//					map<CString, int>::iterator iterItem;
//					iterItem = failInfoMap.find(strErr);
//
//					if (iterItem != failInfoMap.end()) {
//						failInfoMap[strErr] += nFailCnt;
//					}
//					else {
//						std::pair<CString, int> kvPair;
//						kvPair.first = strErr;
//						kvPair.second = nFailCnt;
//						failInfoMap.insert(kvPair);
//					}
//
//					if (bIsDetail) {
//						pReportlog->PrintLog("         ErrNo=0x%04X   Counter=%-6d ErrMsg=%s",
//							itErr->first, itErr->second, GetErrMsg(itErr->first));
//					}
//				}
//			}
//		}
//	}
//
//	if (!bIsDetail) {
//		std::map<CString, int>::iterator itFail = failInfoMap.begin();
//		for (; itFail != failInfoMap.end(); itFail++) {
//			pReportlog->PrintLog("%s:%d", itFail->first, itFail->second);
//		}
//	}
//
//	return Ret;
//}
//
//INT CReporter::DumpReport(IReportlog* pReportlog)
//{
//	INT Ret = ACERR_OK;
//	INT AdpIdx;
//	UINT AllSitePass = 0, AllSiteFail = 0;
//	CTime CurTime = CTime::GetCurrentTime();
//	m_Reporter.strCurTime.Format("%d/%02d/%02d %02d:%02d:%02d", CurTime.GetYear(), CurTime.GetMonth(),
//		CurTime.GetDay(), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
//	pReportlog->PrintLog("*********************************************************************");
//	pReportlog->PrintLog("**************     Project Basic Information   **********************");
//	pReportlog->PrintLog("*********************************************************************");
//	pReportlog->PrintLog("Project Name      : %s", m_Reporter.strProjName);
//	pReportlog->PrintLog("Build Version     : %s", m_Reporter.strBulidVer);
//	pReportlog->PrintLog("Build Time        : %s", m_Reporter.strProjCreateTime);
//	pReportlog->PrintLog("Time When Loaded  : %s", m_Reporter.strProjLoadTime);
//	pReportlog->PrintLog("Current Time      : %s", m_Reporter.strCurTime);
//	pReportlog->PrintLog("Build On          : %s", m_Reporter.strDevType);
//	pReportlog->PrintLog("ChipName          : %s", m_Reporter.strICName);
//	pReportlog->PrintLog("Package           : %s", m_Reporter.strICPack);
//	pReportlog->PrintLog("Adapter           : %s", m_Reporter.strAdapterName);
//	pReportlog->PrintLog("CheckSum          : %s", m_Reporter.strChksum);
//	pReportlog->PrintLog("CheckSum Type     : %s", m_Reporter.strChksumType);
//	pReportlog->PrintLog("Description       : %s", m_Reporter.strProjDescriptor);
//	pReportlog->PrintLog("Author            : %s", m_Reporter.strPCLocation);
//	pReportlog->PrintLog("FuncMode          : %s", m_Reporter.strCurFuncmode);
//	pReportlog->PrintLog("ExecPC            : %s", m_Reporter.strExecPC);
//	pReportlog->PrintLog("Command Sequence  : %s", m_Reporter.strCmdSequence);
//	std::map<CString, tSiteReport*>::iterator it;
//	for (it = m_Reporter.mapSiteReport.begin(); it != m_Reporter.mapSiteReport.end(); ++it) {
//		tSiteReport* pSiteReporter = it->second;
//		AllSitePass += pSiteReporter->PassCnt;
//		AllSiteFail += pSiteReporter->FailCnt;
//	}
//	pReportlog->PrintLog("*********************************************************************");
//	pReportlog->PrintLog("AllSite: Pass=%-8d  Fail=%-8d  Total=%-8d", AllSitePass, AllSiteFail, AllSiteFail + AllSitePass);
//	pReportlog->PrintLog("*********************************************************************");
//
//	for (it = m_Reporter.mapSiteReport.begin(); it != m_Reporter.mapSiteReport.end(); ++it) {
//		tSiteReport* pSiteReporter = it->second;
//		UINT SktPassCnt[ADAPTER_MAXNUM], SktFailCnt[ADAPTER_MAXNUM];
//		pReportlog->PrintLog("*********************************************************************");
//		pReportlog->PrintLog("SiteSN       : %s    SiteAlias : %s", pSiteReporter->strSiteSN, pSiteReporter->strSiteAlias);
//		pReportlog->PrintLog("Gross        : Pass=%-8d  Fail=%-8d  Total=%-8d",
//			pSiteReporter->PassCnt, pSiteReporter->FailCnt, pSiteReporter->PassCnt + pSiteReporter->FailCnt);
//
//		if (pSiteReporter->BtmBoardInfo.BoardUID != (UINT64)-1) {
//			pReportlog->PrintLog("Bottom Board : %s[%I64X]", pSiteReporter->BtmBoardInfo.BoardName,
//				pSiteReporter->BtmBoardInfo.BoardUID);
//		}
//		else {
//			pReportlog->PrintLog("Bottom Board : None");
//		}
//		pReportlog->PrintLog("---------------------------------------------------------------------");
//		for (AdpIdx = 0; AdpIdx < pSiteReporter->AdpCnt; AdpIdx++) {
//			std::vector<tSktReport>& SktReport = pSiteReporter->vArrSktReport[AdpIdx];
//			INT i;
//			GetSktCounter(pSiteReporter, AdpIdx, SktPassCnt[AdpIdx], SktFailCnt[AdpIdx]);
//			pReportlog->PrintLog("SKT[%d]   Pass=%-8d  Fail=%-8d  Total=%-8d",
//				AdpIdx + 1, SktPassCnt[AdpIdx], SktFailCnt[AdpIdx], SktFailCnt[AdpIdx] + SktPassCnt[AdpIdx]);
//			for (i = 0; i < (INT)SktReport.size(); ++i) {
//				pReportlog->PrintLog("         ADP[%I64X]   Pass=%-8d  Fail=%-8d  Total=%-8d",
//					SktReport[i].SktUID, SktReport[i].PassCnt, SktReport[i].FailCnt,
//					SktReport[i].PassCnt + SktReport[i].FailCnt);
//			}
//		}
//		pReportlog->PrintLog("---------------------------------------------------------------------");
//		pReportlog->PrintLog("Fail Status:");
//		for (AdpIdx = 0; AdpIdx < pSiteReporter->AdpCnt; AdpIdx++) {
//			INT j;
//			std::vector<tSktReport>& vSktReport = pSiteReporter->vArrSktReport[AdpIdx];
//			if (SktFailCnt[AdpIdx] == 0) {///没有错误，不需要
//				continue;
//			}
//			pReportlog->PrintLog("SKT[%d]", AdpIdx + 1);
//			for (j = 0; j < (INT)vSktReport.size(); ++j) {
//				tSktReport& SktReport = vSktReport[j];
//				std::map<UINT, UINT>::iterator itErr;
//				pReportlog->PrintLog("    ADP[%016I64X]", SktReport.SktUID);
//				for (itErr = SktReport.mapErrCode.begin(); itErr != SktReport.mapErrCode.end(); itErr++) {
//					pReportlog->PrintLog("         ErrNo=0x%04X   Counter=%-6d ErrMsg=%s",
//						itErr->first, itErr->second, GetErrMsg(itErr->first));
//				}
//			}
//
//		}
//		pReportlog->PrintLog("*********************************************************************");
//	}
//
//	return Ret;
//
//}