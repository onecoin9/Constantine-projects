#include "GlbSetting.h"
#include "ComFunc.h"
#include <QCoreApplication>
#include <QDir>
#include <QSettings>

#define MES_SECURE	(0)
#define SECRETKEY_SIZE	(16)
#define COM_MAX_PATH    (260)

static uchar SecretKey[SECRETKEY_SIZE]={0xB2,0x66,0x71,0x3C,0xDE,0x11,0x40,0x98,\
						   0x8D,0xA6,0x4E,0xC1,0x75,0x3C,0xC3,0xCD};


int InitExcelMesSetting(GLBSETTING *pGlbSetting)
{
	std::string ExePath=CComFunc::GetCurrentPath();
	std::string strIniFile = ExePath+"/mes/ExcelMes.ini";
	QSettings settings(QString::fromStdString(strIniFile), QSettings::IniFormat);

	QString strSFCodeDB = settings.value("Config/SoftCodeTable").toString();
	if (strSFCodeDB.isEmpty()) {
		// 处理未找到SoftCodeTable的情况
	}
	//pGlbSetting->MESInfo.ExcelMesSetting.strSFCodeDB = strSFCodeDB.toStdString();

	//QString strBarCodeDB = settings.value("Config/BarCodeTable").toString();
	//if (strBarCodeDB.isEmpty()) {
	//	// 处理未找到BarCodeTable的情况
	//}
	//pGlbSetting->MESInfo.ExcelMesSetting.strBarCodeDB = strBarCodeDB.toStdString();

	//QString strProjPath = settings.value("Config/ProjRootPath").toString();
	//if (strProjPath.isEmpty()) {
	//	// 处理未找到ProjRootPath的情况
	//}
	//pGlbSetting->MESInfo.ExcelMesSetting.strProjPath = strProjPath.toStdString();

	//pGlbSetting->MESInfo.ExcelMesSetting.nTryWhenFailed = settings.value("Config/TryWhenFailed", 0).toInt();
	//pGlbSetting->MESInfo.ExcelMesSetting.nDelay = settings.value("Config/Delay", 0).toInt();

	return ACERR_OK;
}

int AProgSetting::InitGlbSetting( GLBSETTING *pGlbSetting )
{
	int Ret=ACERR_OK;
	
	//Log
	QString ExePath = QDir::currentPath();
	QString strIniFile = ExePath + "/MultiCfg.ini";
	QSettings settings(strIniFile, QSettings::IniFormat);

	QString logPath = settings.value("Log/LogPath", ExePath + "/log").toString();
	if (logPath.isEmpty()) {
		return ACERR_PARA;
	}
	pGlbSetting->strLogPath = logPath.toStdString();

	pGlbSetting->dwLogSize = settings.value("Log/LogSize", 1024).toInt();
	pGlbSetting->dwLogDay = settings.value("Log/LogDay", 100).toInt();
	//pGlbSetting->dwReportAutoSave = settings.value("Log/ReportAutoSave", 1).toInt();
	//pGlbSetting->dwReportSaveAlone = settings.value("Log/ReportSaveAlone", 1).toInt();
	//pGlbSetting->dwCreateNewLogWhenLoadProj = settings.value("Log/CreateNewLogWhenLoadProj", 0).toInt();
	//pGlbSetting->dwAdpInfoAutoSave = settings.value("Log/SaveAdapterInfo", 0).toInt();
	//pGlbSetting->uReportSaveFormat = settings.value("Log/ReportSaveFormat", 0).toInt();

	////Buffer
	//pGlbSetting->strTmpBufPath = settings.value("Buffer/BuffPath").toString().toStdString();

	////DevScan
	//pGlbSetting->bScanUSB = settings.value("DevScan/ScanUSB", 1).toInt();
	//pGlbSetting->bScanNet = settings.value("DevScan/ScanNET", 0).toInt();
	//pGlbSetting->bAutoConnect = settings.value("DevScan/AutoConnect", 0).toInt();

	//QString netBroadcastAddr = settings.value("DevScan/NetBroadcastAddr", "255.255.255.255").toString();
	//if (netBroadcastAddr.isEmpty()) {
	//	pGlbSetting->strNetBroadcastAddr = "255.255.255.255";
	//}
	//else {
	//	pGlbSetting->strNetBroadcastAddr = netBroadcastAddr.toStdString();
	//}

	////SNCfg
	//pGlbSetting->bBreakPoint = settings.value("SNCfg/BreakPoint", 1).toInt();

	QString saveSNCfg = settings.value("SNCfg/SaveSNCfg", "Project").toString();
	if (saveSNCfg.isEmpty()) {
		return ACERR_PARA;
	}
	//pGlbSetting->strSaveSNCfg = saveSNCfg.toStdString();
	//pGlbSetting->dwSNCfgInProjectHandle = settings.value("SNCfg/SNCfgInProjectHandle", 2).toInt();

	/////Sound设置
	//pGlbSetting->Sound.SoundEn = settings.value("Sound/SoundEn", 1).toInt();
	//pGlbSetting->Sound.LoopUntilClear = settings.value("Sound/LoopUntilClear", 0).toInt();

	//QString soundErr = settings.value("Sound/SoundErr", "./sound/error.wav").toString();
	//if (soundErr.isEmpty()) {
	//	soundErr = "./sound/error.wav";
	//}
	//pGlbSetting->Sound.strSoundErr = soundErr.toStdString();

	//QString soundPass = settings.value("Sound/SoundPass", "./sound/pass.wav").toString();
	//if (soundPass.isEmpty()) {
	//	soundPass = "./sound/pass.wav";
	//}
	//pGlbSetting->Sound.strSoundPass = soundPass.toStdString();

	/////自动化设置
	//pGlbSetting->bAutoEn = settings.value("Automatic/AutoEn", 1).toInt();
	//pGlbSetting->bAutoScanAndConnect = settings.value("Automatic/AutoScanAndConnect", 1).toInt();

	QString autoProtocal = settings.value("Automatic/AutoProtocal", "FProtocal").toString();
	if (autoProtocal.isEmpty()) {
		return ACERR_PARA;
	}
	pGlbSetting->strProtocalType = autoProtocal.toStdString();

	//pGlbSetting->nStopWhenErrorSequence = settings.value("Automatic/StopWhenErrorSequence", 0).toInt();
	//pGlbSetting->OverlapCheckEn = settings.value("Automatic/OverlapCheckEn", 0).toInt();

	/////EProtocal设置
	//pGlbSetting->AutoEProtocal.dwBaudRate = settings.value("EProtocal/BaudRate", 9600).toInt();
	//pGlbSetting->AutoEProtocal.dwUartPort = settings.value("EProtocal/ComPort", 4).toInt();
	//pGlbSetting->AutoEProtocal.uActControl = settings.value("EProtocal/ActControl", 8).toInt();
	//pGlbSetting->AutoEProtocal.DumpPackEn = settings.value("EProtocal/DumpPackEn", 0).toInt();
	//pGlbSetting->AutoEProtocal.dwTimeout = settings.value("EProtocal/Timeout", 1000).toInt();



	/////KProtocal设置
	//QString comm = settings.value("KProtocal/Comm", "UART").toString();
	//if (comm.isEmpty()) {
	//	return ACERR_PARA;
	//}
	//pGlbSetting->AutoKProtocal.strComm = comm.toStdString();

	//pGlbSetting->AutoKProtocal.dwUartPort = settings.value("KProtocal/CommUartPort", 1).toInt();
	//pGlbSetting->AutoKProtocal.DumpPackEn = settings.value("KProtocal/DumpPackEn", 1).toInt();
	//pGlbSetting->AutoKProtocal.SendTryCnt = settings.value("KProtocal/SendTryCnt", 4).toInt();
	//pGlbSetting->AutoKProtocal.DelayModule = settings.value("KProtocal/DelayModule", 500).toInt();


	/////FProtocal设备设置
	//QString fComm = settings.value("FProtocal/Comm", "UART").toString();
	//if (fComm.isEmpty()) {
	//	return ACERR_PARA;
	//}
	//pGlbSetting->AutoFProtocal.strComm = fComm.toStdString();

	//pGlbSetting->AutoFProtocal.dwUartPort = settings.value("FProtocal/CommUartPort", 1).toInt();
	//pGlbSetting->AutoFProtocal.dwTimeout = settings.value("FProtocal/CommTimeout", 1).toInt();
	//pGlbSetting->AutoFProtocal.dwRetryCnt = settings.value("FProtocal/CommRetryCnt", 1).toInt();
	//pGlbSetting->AutoFProtocal.dwRetryTimeWait = settings.value("FProtocal/CommRetryTimeWait", 1).toInt();
	//pGlbSetting->AutoFProtocal.DumpPackEn = settings.value("FProtocal/DumpPackEn", 1).toInt();
	//pGlbSetting->AutoFProtocal.StartWaitSendedEn = settings.value("FProtocal/StartWaitSendedEn", 0).toInt();
	//pGlbSetting->AutoFProtocal.dwBaudRate = settings.value("FProtocal/Baudrate", 115200).toInt();
	//pGlbSetting->AutoFProtocal.ProtocalNegotiationEn = settings.value("FProtocal/ProtocalNegotiationEn", 1).toInt();


	///SProtocal
	QString sIPAddr = settings.value("SProtocal/IPAddr", "127.0.0.1").toString();
	if (sIPAddr.isEmpty()) {
		pGlbSetting->AutoSProtocal.strIPAddress = "127.0.0.1";
	}
	else {
		pGlbSetting->AutoSProtocal.strIPAddress = sIPAddr.toStdString();
	}

	QString allMachType = settings.value("SProtocal/AllMachType", "IPS7000,PHA2000,IPS5000").toString();
	QStringList allMachTypeList = allMachType.split(",");
	for (const QString& item : allMachTypeList) {
		pGlbSetting->AutoSProtocal.m_vAutoMachAllMode.push_back(item.toStdString());
	}

	QString needTransTaskDataMachType = settings.value("SProtocal/NeedTransTaskDataMachType", "IPS7000,PHA2000,IPS5000").toString();
	QStringList needTransTaskDataMachTypeList = needTransTaskDataMachType.split(",");
	for (const QString& item : needTransTaskDataMachTypeList) {
		pGlbSetting->AutoSProtocal.m_vNeedSendTaskDataType.push_back(item.toStdString());
	}

	QString machType = settings.value("SProtocal/MachType", "PHA2000").toString();
	if (machType.isEmpty()) {
		pGlbSetting->AutoSProtocal.strMachType = "PHA2000";
	}
	else {
		pGlbSetting->AutoSProtocal.strMachType = machType.toStdString();
	}

	pGlbSetting->AutoSProtocal.Port = settings.value("SProtocal/Port", 1000).toInt();
	pGlbSetting->AutoSProtocal.DumpPackEn = settings.value("SProtocal/DumpPackEn", 1).toInt();


	//ICTNetProtocal
	//pGlbSetting->AutoICTNetProtocal.port = settings.value("ICTNetProtocal/Port", 40866).toInt();


	///读取映射表
	QString siteMap = settings.value("AutoSiteMap/SiteMap", "").toString();
	if (siteMap.isEmpty()) {
		return ACERR_FAIL;
	}

	QString tmpSiteMap = siteMap;
	QString realSiteAlias;
	QString autoSiteAlias;
	bool bSiteMapBegin = false;
	bool bSiteAlaisGet = false;

	while (!tmpSiteMap.isEmpty()) {
		if (tmpSiteMap.at(0) == '<') {
			bSiteMapBegin = true;
			goto __Next;
		}
		else if (tmpSiteMap.at(0) == '>') {
			if (!realSiteAlias.isEmpty() && !autoSiteAlias.isEmpty()) {
				pGlbSetting->AutoSiteMap[realSiteAlias.toStdString()] = autoSiteAlias.toStdString();
			}
			realSiteAlias.clear();
			autoSiteAlias.clear();
			bSiteMapBegin = false;
			bSiteAlaisGet = false;
			goto __Next;
		}

		if (bSiteMapBegin) {
			if (tmpSiteMap.at(0) == ',') {
				bSiteAlaisGet = true;
				goto __Next;
			}
			if (!bSiteAlaisGet) {
				realSiteAlias += tmpSiteMap.at(0);
			}
			else {
				autoSiteAlias += tmpSiteMap.at(0);
			}
		}

	__Next:
		tmpSiteMap.remove(0, 1);
	}

	if (!realSiteAlias.isEmpty() && !autoSiteAlias.isEmpty()) {
		pGlbSetting->AutoSiteMap[realSiteAlias.toStdString()] = autoSiteAlias.toStdString();
	}
	realSiteAlias.clear();
	autoSiteAlias.clear();

	////MES系统
	/*pGlbSetting->MESInfo.MESEn = settings.value("MES/MESEn", 0).toInt();

	QString MESTmpBuf = settings.value("MES/MESManu", "Midea").toString();
	if (MESTmpBuf.isEmpty()) {
		return ACERR_PARA;
	}
	pGlbSetting->MESInfo.MESManu = MESTmpBuf.toStdString();

	MESTmpBuf = settings.value("MES/MESFirstBoot", "Program").toString();
	if (MESTmpBuf.isEmpty()) {
		return ACERR_PARA;
	}
	pGlbSetting->MESInfo.MESFirtBoot = MESTmpBuf.toStdString();

	pGlbSetting->MESInfo.MESBKEn = settings.value("MES/MESBkEn", 1).toInt();
	pGlbSetting->MESInfo.MESExChangeModeEn = settings.value("MES/MESExChangeModeEn", 0).toInt();*/

	////Midea
	//int ucharsUsed;
	//memset(TmpBuf,0,COM_MAX_PATH);
	//GetPrivateProfileString("Midea", "MideaSoftCode","",TmpBuf, COM_MAX_PATH, strIniFile);
	//if(TmpBuf[0]==0){
	//	//return ACERR_PARA;
	//}
	//ucharsUsed=(int)strlen(TmpBuf);
	//DeCode(TmpBuf,ucharsUsed,TmpBuf,ucharsUsed);
	//pGlbSetting->MESInfo.MideaSetting.strSFCodeDB.Format("%s",TmpBuf);

	//memset(TmpBuf,0,COM_MAX_PATH);
	//GetPrivateProfileString("Midea", "MideaBarCode","",TmpBuf, COM_MAX_PATH, strIniFile);
	//if(TmpBuf[0]==0){
	//	//return ACERR_PARA;
	//}
	//ucharsUsed=(int)strlen(TmpBuf);
	//DeCode(TmpBuf,ucharsUsed,TmpBuf,ucharsUsed);
	//pGlbSetting->MESInfo.MideaSetting.strBarCodeDB.Format("%s",TmpBuf);

	//memset(TmpBuf,0,COM_MAX_PATH);
	//GetPrivateProfileString("Midea", "MideaProjPath","",TmpBuf, COM_MAX_PATH, strIniFile);
	//if(TmpBuf[0]==0){
	//	//return ACERR_PARA;
	//}
	//ucharsUsed=(int)strlen(TmpBuf);
	//DeCode(TmpBuf,ucharsUsed,TmpBuf,ucharsUsed);
	//pGlbSetting->MESInfo.MideaSetting.strProjPath.Format("%s",TmpBuf);

	//pGlbSetting->MESInfo.MideaSetting.nTryWhenFailed=GetPrivateProfileint("Midea","MideaTryWhenFailed",0,strIniFile);
	//pGlbSetting->MESInfo.MideaSetting.nDelay=GetPrivateProfileint("Midea","MideaDelay",0,strIniFile);

	///Timer设置
	//pGlbSetting->nSenorinterval = settings.value("Timer/Sensorinterval", 200).toInt();
	//pGlbSetting->nSecondMinus = settings.value("Timer/SecondMinus", 0).toInt();

	/////OEM
	//pGlbSetting->OEMInfo.OEMCompatible = settings.value("OEM/OEMCompatible", 0).toInt();

	//QString OEMTmpBuf = settings.value("OEM/OEMName", "Acroview").toString();
	//if (OEMTmpBuf.isEmpty()) {
	//	pGlbSetting->OEMInfo.OEMName = "Acroview";
	//}
	//else {
	//	pGlbSetting->OEMInfo.OEMName = OEMTmpBuf.toStdString();
	//}

	//OEMTmpBuf = settings.value("OEM/BuildVersion", "").toString();
	//pGlbSetting->BuildVersion = OEMTmpBuf.toStdString();

	/////OCX
	//pGlbSetting->OCXInfo.WindowEn = settings.value("OCX/WindowEn", 0).toInt();
	//pGlbSetting->OCXInfo.OcxCallEn = false;
	//pGlbSetting->OCXInfo.InsetionCheckShowPassIfICOut = settings.value("OCX/InsertionCheckShowPassWhenICOut", 0).toInt();
	/////Adapter Notify
	//pGlbSetting->AdpNotify.bEn = settings.value("ADPNOTIFY/NotifyEn", 1).toInt();
	//pGlbSetting->AdpNotify.bAbsEn = settings.value("ADPNOTIFY/AbsEn", 0).toInt();
	//pGlbSetting->AdpNotify.PercValue = settings.value("ADPNOTIFY/PercentValue", 70).toInt();
	//pGlbSetting->AdpNotify.AbsValue = settings.value("ADPNOTIFY/AbsValue", 40000).toInt();
	////UID
	//pGlbSetting->UIDInfo.bUIDEn = settings.value("UID/UIDEn", 0).toInt();
	//pGlbSetting->UIDInfo.nUIDMODE = settings.value("UID/UIDMODE", 1).toInt();
	//pGlbSetting->UIDInfo.nEndType = settings.value("UID/EndType", 0).toInt();
	//pGlbSetting->UIDInfo.uSwapUnit = settings.value("UID/SwapUnit", 0).toInt();
	////Project
	//pGlbSetting->dwMatchDevTypeEn = settings.value("Project/MatchDevTypeEn", 1).toInt();
	////SNNET
	//QString SNNETTtmpBuf = settings.value("SNNET/SNNetIP", "127.0.0.1").toString();
	//pGlbSetting->SNNetIP = SNNETTtmpBuf.toStdString();

	//pGlbSetting->SNNetPort = settings.value("SNNET/SNNetPort", 1008).toInt();

	////Lic
	//QString LICTmpBuf = settings.value("LIC/LicServerIP", "127.0.0.1").toString();
	//pGlbSetting->strLicServerIP = LICTmpBuf.toStdString();

	//LICTmpBuf = settings.value("LIC/LicFilePath", "C:\\ACROVIEW\\Autolic").toString();
	//pGlbSetting->strLicFilePath = LICTmpBuf.toStdString();

	//pGlbSetting->uLicServerPort = settings.value("LIC/LicServerPort", 2020).toInt();
	//pGlbSetting->IsLicEn = settings.value("LIC/LicEnable", 1).toInt();
	//pGlbSetting->uLicDelayTime = settings.value("LIC/DelayTime", 1000).toInt();

	//pGlbSetting->SNEnableUid = settings.value("SNNET/SNEnableUid", 0).toInt();

	//pGlbSetting->uSNMode = settings.value("SNCfg/SNMode", 0).toInt();

	//pGlbSetting->nSysLogEn = settings.value("SysLog/SysLogEnable", 0).toInt();

	//pGlbSetting->nSaveSiteLog = settings.value("Log/SiteLogEnable", 0).toInt();

	//pGlbSetting->nDumpaSNLogEn = settings.value("Log/DumpaSNLogEn", 1).toInt();

	//pGlbSetting->nCntInfoEn = settings.value("SysLog/CntInfoEn", 1).toInt();

	//InitExcelMesSetting(pGlbSetting);

	//QString LanguageTmpBuf = settings.value("Language/CurLang", "").toString();
	//if (LanguageTmpBuf.isEmpty()) {
	//	return ACERR_FAIL;
	//}
	//pGlbSetting->CurLang = LanguageTmpBuf.toStdString();


__end:
	return ACERR_OK;
}

bool SaveExcelMesSetting(GLBSETTING*pGlbSetting)
{
	QString exePath = QCoreApplication::applicationDirPath();
	QString strIniFile = exePath + "/mes/ExcelMes.ini";

	QSettings settings(strIniFile, QSettings::IniFormat);

	//settings.setValue("Config/SoftCodeTable", QString::fromStdString(pGlbSetting->MESInfo.ExcelMesSetting.strSFCodeDB));
	//settings.setValue("Config/BarCodeTable", QString::fromStdString(pGlbSetting->MESInfo.ExcelMesSetting.strBarCodeDB));
	//settings.setValue("Config/ProjRootPath", QString::fromStdString(pGlbSetting->MESInfo.ExcelMesSetting.strProjPath));
	//settings.setValue("Config/TryWhenFailed", pGlbSetting->MESInfo.ExcelMesSetting.nTryWhenFailed);
	//settings.setValue("Config/Delay", pGlbSetting->MESInfo.ExcelMesSetting.nDelay);

	// 检查写入是否成功
	if (settings.status() != QSettings::NoError) {
		// 处理写入失败的情况
		return false;
	}

	return true;
}


int AProgSetting::SaveGlbSetting( GLBSETTING *pGlbSetting )
{
	QString exePath = QCoreApplication::applicationDirPath();
	QString strIniFile = exePath + "/MultiCfg.ini";

	QSettings settings(strIniFile, QSettings::IniFormat);

	//Log
	settings.setValue("Log/LogPath", QString::fromStdString(pGlbSetting->strLogPath));
	settings.setValue("Log/LogSize", QString::number(pGlbSetting->dwLogSize));
	settings.setValue("Log/LogDay", QString::number(pGlbSetting->dwLogDay));
	//settings.setValue("Log/ReportAutoSave", QString::number(pGlbSetting->dwReportAutoSave));
	//settings.setValue("Log/CreateNewLogWhenLoadProj", QString::number(pGlbSetting->dwCreateNewLogWhenLoadProj));
	//settings.setValue("Log/ReportSaveFormat", QString::number(pGlbSetting->uReportSaveFormat));
	//settings.setValue("Log/ReportSaveAlone", QString::number(pGlbSetting->dwReportSaveAlone));
	//settings.setValue("Log/SaveAdapterInfo", QString::number(pGlbSetting->dwAdpInfoAutoSave));

	////Buffer
	//settings.setValue("Buffer/BuffPath", QString::fromStdString(pGlbSetting->strTmpBufPath));

	////DevScan
	//settings.setValue("DevScan/ScanUSB", pGlbSetting->bScanUSB);
	//settings.setValue("DevScan/ScanNET", pGlbSetting->bScanNet);
	//settings.setValue("DevScan/AutoConnect", pGlbSetting->bAutoConnect);
	//settings.setValue("DevScan/NetBroadcastAddr", QString::fromStdString(pGlbSetting->strNetBroadcastAddr));

	////SNCfg
	//settings.setValue("SNCfg/BreakPoint", pGlbSetting->bBreakPoint);
	//settings.setValue("SNCfg/SNCfgInProjectHandle", QString::number(pGlbSetting->dwSNCfgInProjectHandle));
	//settings.setValue("SNCfg/SaveSNCfg", QString::fromStdString(pGlbSetting->strSaveSNCfg));

	/////Sound
	//settings.setValue("Sound/SoundEn", pGlbSetting->Sound.SoundEn);
	//settings.setValue("Sound/LoopUntilClear", pGlbSetting->Sound.LoopUntilClear);
	//settings.setValue("Sound/SoundErr", QString::fromStdString(pGlbSetting->Sound.strSoundErr));
	//settings.setValue("Sound/SoundPass", QString::fromStdString(pGlbSetting->Sound.strSoundPass));

	/////自动化设置
	//settings.setValue("Automatic/AutoEn", pGlbSetting->bAutoEn);
	//settings.setValue("Automatic/AutoScanAndConnect", pGlbSetting->bAutoScanAndConnect);
	settings.setValue("Automatic/AutoProtocal", QString::fromStdString(pGlbSetting->strProtocalType));
	//settings.setValue("Automatic/StopWhenErrorSequence", QString::number(pGlbSetting->nStopWhenErrorSequence));
	//settings.setValue("Automatic/OverlapCheckEn", QString::number(pGlbSetting->OverlapCheckEn));

	/////KProtocal设置
	//settings.setValue("KProtocal/Comm", QString::fromStdString(pGlbSetting->AutoKProtocal.strComm));
	//settings.setValue("KProtocal/CommUartPort", QString::number(pGlbSetting->AutoKProtocal.dwUartPort));
	//settings.setValue("KProtocal/DumpPackEn", pGlbSetting->AutoKProtocal.DumpPackEn);
	//settings.setValue("KProtocal/SendTryCnt", QString::number(pGlbSetting->AutoKProtocal.SendTryCnt));
	//settings.setValue("KProtocal/DelayModule", QString::number(pGlbSetting->AutoKProtocal.DelayModule));

	/////EProtocal设置
	//settings.setValue("EProtocal/ComPort", QString::number(pGlbSetting->AutoEProtocal.dwUartPort));
	//settings.setValue("EProtocal/ActControl", QString::number(pGlbSetting->AutoEProtocal.uActControl));
	//settings.setValue("EProtocal/BaudRate", QString::number(pGlbSetting->AutoEProtocal.dwBaudRate));
	//settings.setValue("EProtocal/DumpPackEn", pGlbSetting->AutoEProtocal.DumpPackEn);
	//settings.setValue("EProtocal/Timeout", QString::number(pGlbSetting->AutoEProtocal.dwTimeout));


	///// FProtocal自动化设置
	//settings.setValue("EProtocal/Comm", QString::fromStdString(pGlbSetting->AutoFProtocal.strComm));
	//settings.setValue("EProtocal/CommUartPort", QString::number(pGlbSetting->AutoFProtocal.dwUartPort));
	//settings.setValue("EProtocal/CommTimeout", QString::number(pGlbSetting->AutoFProtocal.dwTimeout));
	//settings.setValue("EProtocal/CommRetryCnt", QString::number(pGlbSetting->AutoFProtocal.dwRetryCnt));
	//settings.setValue("EProtocal/CommRetryTimeWait", QString::number(pGlbSetting->AutoFProtocal.dwRetryTimeWait));
	//settings.setValue("EProtocal/DumpPackEn", pGlbSetting->AutoFProtocal.DumpPackEn);
	//settings.setValue("EProtocal/StartWaitSendedEn", pGlbSetting->AutoFProtocal.StartWaitSendedEn);
	//settings.setValue("EProtocal/ProtocalNegotiationEn", pGlbSetting->AutoFProtocal.ProtocalNegotiationEn);

	///SProtocal设置
	settings.setValue("SProtocal/IPAddr", QString::fromStdString(pGlbSetting->AutoSProtocal.strIPAddress));
	settings.setValue("SProtocal/DumpPackEn", pGlbSetting->AutoSProtocal.DumpPackEn);
	settings.setValue("SProtocal/Port", QString::number(pGlbSetting->AutoSProtocal.Port));
	settings.setValue("SProtocal/MachType", QString::fromStdString(pGlbSetting->AutoSProtocal.strMachType));

	///站点映射关系表
	int SiteMapSize = pGlbSetting->AutoSiteMap.size();
	if (SiteMapSize > 0) {
		QString TmpBuf;
		for (auto it = pGlbSetting->AutoSiteMap.begin(); it != pGlbSetting->AutoSiteMap.end(); ++it) {
			QString Tmp = QString("<%1,%2>").arg(QString::fromStdString(it->first), QString::fromStdString(it->second));
			TmpBuf.append(Tmp);
		}

		settings.setValue("AutoSiteMap/SiteMap", TmpBuf);
		if (settings.status() != QSettings::NoError) {
			return ACERR_FAIL;
		}
	}

	////MES系统
	//settings.setValue("MES/MESEn", pGlbSetting->MESInfo.MESEn);
	//settings.setValue("MES/MESManu", QString::fromStdString(pGlbSetting->MESInfo.MESManu));
	//settings.setValue("MES/MESFirstBoot", QString::fromStdString(pGlbSetting->MESInfo.MESFirtBoot));
	//settings.setValue("MES/MESBkEn", pGlbSetting->MESInfo.MESBKEn);
	//settings.setValue("MES/MESExChangeModeEn", pGlbSetting->MESInfo.MESExChangeModeEn);
	
	///Midea
	/*settings.setValue("Midea/MideaSoftCode", QString::fromStdString(pGlbSetting->MESInfo.MideaSetting.strSFCodeDB));
	int ucharused;

	memset(TmpBuf,0,COM_MAX_PATH);
	sprintf(TmpBuf,"%s",pGlbSetting->MESInfo.MideaSetting.strSFCodeDB);
	ucharused=(int)strlen(TmpBuf);
	EnCode(TmpBuf,ucharused,TmpBuf,ucharused);
	RetWrite=WritePrivateProfileStringA("Midea", "MideaSoftCode",TmpBuf, strIniFile);
	if(RetWrite==false){
		goto __end;
	}

	memset(TmpBuf,0,COM_MAX_PATH);
	sprintf(TmpBuf,"%s",pGlbSetting->MESInfo.MideaSetting.strBarCodeDB);
	ucharused=(int)strlen(TmpBuf);
	EnCode(TmpBuf,ucharused,TmpBuf,ucharused);
	RetWrite=WritePrivateProfileStringA("Midea", "MideaBarCode",TmpBuf, strIniFile);
	if(RetWrite==false){
		goto __end;
	}

	memset(TmpBuf,0,COM_MAX_PATH);
	sprintf(TmpBuf,"%s",pGlbSetting->MESInfo.MideaSetting.strProjPath);
	ucharused=(int)strlen(TmpBuf);
	EnCode(TmpBuf,ucharused,TmpBuf,ucharused);
	RetWrite=WritePrivateProfileStringA("Midea", "MideaProjPath",TmpBuf, strIniFile);
	if(RetWrite==false){
		goto __end;
	}

	memset(TmpBuf,0,COM_MAX_PATH);
	sprintf(TmpBuf,"%d",pGlbSetting->MESInfo.MideaSetting.nTryWhenFailed);
	RetWrite=WritePrivateProfileStringA("Midea", "MideaTryWhenFailed",TmpBuf, strIniFile);
	if(RetWrite==false){
		goto __end;
	}

	memset(TmpBuf,0,COM_MAX_PATH);
	sprintf(TmpBuf,"%d",pGlbSetting->MESInfo.MideaSetting.nDelay);
	RetWrite=WritePrivateProfileStringA("Midea", "MideaDelay",TmpBuf, strIniFile);
	if(RetWrite==false){
		goto __end;
	}*/

	///Timer设置
//	settings.setValue("Timer/Sensorinterval", QString::number(pGlbSetting->nSenorinterval));
//	settings.setValue("Timer/SecondMinus", QString::number(pGlbSetting->nSecondMinus));
//
//	///OCX
//	settings.setValue("OCX/WindowEn", pGlbSetting->OCXInfo.WindowEn);
//	
//	//UID
//	settings.setValue("UID/UIDEn", QString::number(pGlbSetting->UIDInfo.bUIDEn));
//	settings.setValue("UID/UIDMODE", QString::number(pGlbSetting->UIDInfo.nUIDMODE));
//	settings.setValue("UID/ENDTYPE", QString::number(pGlbSetting->UIDInfo.nEndType));
//	settings.setValue("UID/SwapUnit", QString::number(pGlbSetting->UIDInfo.uSwapUnit));
//	
//	//SNCfg
//	settings.setValue("SNCfg/SNMode", QString::number(pGlbSetting->uSNMode));
//
//	//SNNet
//	settings.setValue("SNNet/SNEnableUid", pGlbSetting->SNEnableUid);
//	settings.setValue("SNNet/SNEnableUid", QString::number(pGlbSetting->SNNetPort));
//	settings.setValue("SNNet/SNEnableUid", QString::fromStdString(pGlbSetting->SNNetIP));
//
//	//LIC
//	settings.setValue("LIC/LicServerPort", QString::number(pGlbSetting->uLicServerPort));
//	settings.setValue("LIC/LicServerIP", QString::fromStdString(pGlbSetting->strLicServerIP));
//	settings.setValue("LIC/LicFilePath", QString::fromStdString(pGlbSetting->strLicFilePath));
//	
//	///Adapter Notify
//	settings.setValue("ADPNOTIFY/NotifyEn", pGlbSetting->AdpNotify.bEn);
//	settings.setValue("ADPNOTIFY/AbsEn", pGlbSetting->AdpNotify.bAbsEn);
//	settings.setValue("ADPNOTIFY/AbsValue", pGlbSetting->AdpNotify.AbsValue);
//	settings.setValue("ADPNOTIFY/PercentValue", QString::number(pGlbSetting->AdpNotify.PercValue));
//
//////////
//	bool RetWrite = SaveExcelMesSetting(pGlbSetting);
//	if(RetWrite==false){
//		return ACERR_FAIL;
//	}
//
//
//	settings.setValue("Language/CurLang", QString::fromStdString(pGlbSetting->CurLang));
//	if(RetWrite==false){
//		return ACERR_FAIL;
//	}

__end:
	return true;//RetWrite==true?ACERR_OK:ACERR_FAIL;
}

bool AProgSetting::IsAutomaticEn( GLBSETTING* pGlbSetting )
{
	return true;// pGlbSetting->bAutoEn;
}

std::string AProgSetting::GetAutoManu( GLBSETTING*pGlbSetting )
{
	return pGlbSetting->strProtocalType;
}

bool AProgSetting::IsAutoScanAndConnect( GLBSETTING*pGlbSetting )
{
	return true;// pGlbSetting->bAutoScanAndConnect;
}

bool AProgSetting::IsMESEn(GLBSETTING*pGlbSetting)
{
	return true;//pGlbSetting->MESInfo.MESEn;
}

bool AProgSetting::IsMESExChangeModeEn(GLBSETTING*pGlbSetting){
	return true;//pGlbSetting->MESInfo.MESExChangeModeEn;
}

std::string AProgSetting::GetMESManu( GLBSETTING*pGlbSetting)
{
	std::string strManu="";
	//if(pGlbSetting->MESInfo.MESEn==1){
	//	strManu=pGlbSetting->MESInfo.MESManu;
	//}
	return strManu;
}

std::string AProgSetting::GetMESFirstBoot(GLBSETTING*pGlbSetting)
{
	std::string strFirstBoot="";
	//if(pGlbSetting->MESInfo.MESEn==1){
	//	strFirstBoot=pGlbSetting->MESInfo.MESFirtBoot;
	//}
	return strFirstBoot;
}

std::string AProgSetting::GetSoundErrPath(GLBSETTING*pGlbSetting)
{
	//if(pGlbSetting->Sound.strSoundErr.at(0) == '.'){///相对路径
	//	string strExePath;
	//	strExePath=CComFunc::GetCurrentPath();
	//	strExePath +="//"+pGlbSetting->Sound.strSoundErr;
	//	return strExePath;
	//}
	//else{
	//	return pGlbSetting->Sound.strSoundErr;
	//}
	return "";
}

std::string AProgSetting::GetSoundPassPath(GLBSETTING*pGlbSetting)
{
	//if(pGlbSetting->Sound.strSoundPass.at(0) == '.'){///相对路径
	//	string strExePath;
	//	strExePath=CComFunc::GetCurrentPath();
	//	strExePath +="//"+pGlbSetting->Sound.strSoundPass;
	//	return strExePath;
	//}
	//else{
	//	return pGlbSetting->Sound.strSoundPass;
	//}
	return "";
}

bool AProgSetting::EnCode( char*pSrcData,int SrcSize,char*pDestData,int DestSize )
{
#if MES_SECURE==1
	int i;
	for(i=0;i<SrcSize;i++){
		pDestData[i]=pSrcData[i]^SecretKey[i%SECRETKEY_SIZE];
	}
#endif
	return true;
}

bool AProgSetting::DeCode(char*pSrcData,int SrcSize,char*pDestData,int DestSize )
{
#if MES_SECURE==1
	int i;
	for(i=0;i<SrcSize;i++){
		pDestData[i]=pSrcData[i]^SecretKey[i%SECRETKEY_SIZE];
	}
#endif
	return true;
}


int GetLang(std::string& strLang)
{
	QString exePath = QCoreApplication::applicationDirPath();
	QString strIniFile = exePath + "/MultiCfg.ini";

	QSettings settings(strIniFile, QSettings::IniFormat);

	// 从配置文件中读取当前语言
	QString curLang = settings.value("Language/CurLang", "English").toString();

	// 将读取的语言存储到 strLang 中
	strLang = curLang.toStdString();
	return 0;
}

int SetLang(std::string strLang)
{
	int Ret = 0;
	QString exePath = QCoreApplication::applicationDirPath();
	QString strIniFile = exePath + "/MultiCfg.ini";

	QSettings settings(strIniFile, QSettings::IniFormat);

	// 将语言写入配置文件
	settings.setValue("Language/CurLang", QString::fromStdString(strLang));

	// 检查写入是否成功
	if (!settings.status() == QSettings::NoError) {
		Ret = -1; // 如果写入失败，返回错误代码
	}

	return Ret;
}