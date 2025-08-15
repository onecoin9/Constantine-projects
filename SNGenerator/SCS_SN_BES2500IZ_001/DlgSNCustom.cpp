// DlgSNCustom.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SCS_SN_BES2500IZ_001.h"
#include "DlgSNCustom.h"
#include "../Com/ComTool.h"
#include "../Com/ComFunc.h"
//#include <stdlib.h>


// CDlgSNCustom �Ի���

IMPLEMENT_DYNAMIC(CDlgSNCustom, CDialog)

CDlgSNCustom::CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNCustom::IDD, pParent)
	, m_pSnCfgPara(pSnCfgPara)
	//, m_SNStartAddr(0)
	//, m_DeviceAddr(0)
	//, m_DeviceIDLen(0)
	//, m_SecretAddr(0)
	//, m_SecretLen(0)
	//, m_strCSVFile(_T(""))
	, m_strElaphantPath(_T(""))
	, m_strSoundPlusPath(_T(""))
{
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	CString strPath;
	LoadInI();
	
	//pCfg->DeviceIDAddr=0x1FF100;
	//pCfg->DeviceIDLen=17;
	//pCfg->SecretAddr=0x1FF000;
	//pCfg->SecretLen=64;
	//pCfg->strCSVFile="";
}

CDlgSNCustom::~CDlgSNCustom()
{
}

BOOL CDlgSNCustom::InitCtrlsValue(CSNCustomCfg& SNCfg)
{
	tSNCustomCfg* pCfg=SNCfg.GetCustomCfg();
	CString strText;

	strText.Format("%I64X",pCfg->ElephantAddr);
	GetDlgItem(IDC_ELEPHANT_ADDR)->SetWindowText(strText);

	strText.Format("%u",pCfg->ElephantLen);
	GetDlgItem(IDC_ELEPHANT_LEN)->SetWindowText(strText);
	//

	strText.Format("%I64X",pCfg->SoundPlusAddr);
	GetDlgItem(IDC_SOUNDPLUS_ADDR)->SetWindowText(strText);

	strText.Format("%u",pCfg->SoundPlusLen);
	GetDlgItem(IDC_SOUNDPLUS_LEN)->SetWindowText(strText);
	//
	strText.Format("%I64X",pCfg->BLENameAddr);
	GetDlgItem(IDC_BLT_NAME_ADDR)->SetWindowText(strText);

	strText.Format("%u",pCfg->BLENameLen);
	GetDlgItem(IDC_BLT_NAME_LEN)->SetWindowText(strText);
	//
	strText.Format("%I64X",pCfg->BLEMACAddr);
	GetDlgItem(IDC_BLE_MAC_ADDR)->SetWindowText(strText);

	strText.Format("%u",pCfg->BLEMACLen);
	GetDlgItem(IDC_BLE_MAC_LEN)->SetWindowText(strText);
	///////
	strText.Format("%I64X",pCfg->BTNameAddr);
	GetDlgItem(IDC_BT_NAME_ADDR)->SetWindowText(strText);

	strText.Format("%u",pCfg->BTNameLen);
	GetDlgItem(IDC_BT_NAME_LEN)->SetWindowText(strText);
	//
	strText.Format("%I64X",pCfg->BTMACAddr);
	GetDlgItem(IDC_BT_MAC_ADDR)->SetWindowText(strText);

	strText.Format("%u",pCfg->BTMACLen);
	GetDlgItem(IDC_BT_MAC_LEN)->SetWindowText(strText);

	((CButton*)GetDlgItem(IDC_CHECK_ELEPHANT_EN))->SetCheck(pCfg->nElephantEn);
	((CButton*)GetDlgItem(IDC_CHECK_SOUNDPLAUS_EN))->SetCheck(pCfg->nSoundPlusEn);

	m_ComBoxElephantFrom.SetCurSel(pCfg->nElephanSourceFrom);
	m_ComBoxSoundPlusFrom.SetCurSel(pCfg->nSoundPlusSourceFrom);

	strText.Format("%s",pCfg->strProductBatch);
	GetDlgItem(IDC_PRODUCT_BATCH)->SetWindowText(strText);

	GetDlgItem(IDC_BLT_NAME_CONTENT)->SetWindowText(pCfg->strBLEContent);
	GetDlgItem(IDC_BT_NAME_CONTENT)->SetWindowText(pCfg->strBTContent);

	////////////////////////add////////////////////
	GetDlgItem(IDC_ELAPHANT_PATH)->SetWindowText(pCfg->strElaphantPath); 
	GetDlgItem(IDC_SOUNDPLUS_PATH)->SetWindowText(pCfg->strSoundPlusPath); 
	m_strElaphantPath.Format("%s", pCfg->strElaphantPath);
	m_strSoundPlusPath.Format("%s", pCfg->strSoundPlusPath);

	UpdateData(FALSE);
	return TRUE;
}

BOOL CDlgSNCustom::InitCtrls(CSerial& lSerial)
{
	BOOL Ret=TRUE;
	CString strErrMsg;
	if(lSerial.GetLength()!=0){///�ٴδ򿪶Ի����ʱ��ͻᴫ�룬��֮ǰ����Ϊ��Դ���ͷţ�������Ҫ���´��ļ�
		Ret=m_SnCfg.SerialInCfgData(lSerial);
		if(Ret==TRUE){
			tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
			//m_CSVFile.CloseFile();
			//if(m_CSVFile.OpenFile(pCfg->strCSVFile)!=0){
			//	strErrMsg=m_CSVFile.GetErrMsg();
			//	MessageBox(strErrMsg,"",MB_OK);
			//}
		}
	}
	else{///�״μ��ص�ʱ��lSerial��û��ֵ
	}
	Ret=InitCtrlsValue(m_SnCfg);
	return Ret;
}

BOOL CDlgSNCustom::SaveCtrls()
{
	UpdateData(TRUE);
	CString strText;
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	GetDlgItem(IDC_ELEPHANT_ADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->ElephantAddr)!=1){
		MessageBox("��ȡ�����ַ����,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_ELEPHANT_LEN)->GetWindowText(strText);
	if(sscanf(strText,"%d",&pCfg->ElephantLen)!=1){
		MessageBox("��ȡ�����ֽڳ��ȴ���,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_SOUNDPLUS_ADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->SoundPlusAddr)!=1){
		MessageBox("��ȡ���ӵ�ַ����,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_SOUNDPLUS_LEN)->GetWindowText(strText);
	if(sscanf(strText,"%d",&pCfg->SoundPlusLen)!=1){
		MessageBox("��ȡ�����ֽڳ��ȴ���,��ȷ��");
		return FALSE;
	}
	///////////////
	GetDlgItem(IDC_BLT_NAME_ADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->BLENameAddr)!=1){
		MessageBox("��ȡBLT Name��ַ����,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_BLT_NAME_LEN)->GetWindowText(strText);
	if(sscanf(strText,"%d",&pCfg->BLENameLen)!=1){
		MessageBox("��ȡBLT Name�ֽڳ��ȴ���,��ȷ��");
		return FALSE;
	}
	///////////////////////////
	GetDlgItem(IDC_BLT_NAME_CONTENT)->GetWindowText(pCfg->strBLEContent);

	GetDlgItem(IDC_BT_NAME_CONTENT)->GetWindowText(pCfg->strBTContent);
	/////////////////////

	GetDlgItem(IDC_BLE_MAC_ADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->BLEMACAddr)!=1){
		MessageBox("��ȡBLT MAC��ַ����,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_BLE_MAC_LEN)->GetWindowText(strText);
	if(sscanf(strText,"%d",&pCfg->BLEMACLen)!=1){
		MessageBox("��ȡBLT MAC�ֽڳ��ȴ���,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_BT_NAME_ADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->BTNameAddr)!=1){
		MessageBox("��ȡBT Name��ַ����,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_BT_NAME_LEN)->GetWindowText(strText);
	if(sscanf(strText,"%d",&pCfg->BTNameLen)!=1){
		MessageBox("��ȡBT Name�ֽڳ��ȴ���,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_BT_MAC_ADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->BTMACAddr)!=1){
		MessageBox("��ȡBT Name��ַ����,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_BT_MAC_LEN)->GetWindowText(strText);
	if(sscanf(strText,"%d",&pCfg->BTMACLen)!=1){
		MessageBox("��ȡBT MAC�ֽڳ��ȴ���,��ȷ��");
		return FALSE;
	}
	
	int nTotalFromMes = 0;
	pCfg->nElephanSourceFrom = 0;
	INT CurSel=((CComboBox*)GetDlgItem(IDC_COMBO_ELEPHANT_FRROM))->GetCurSel();
	if (CurSel == 1){  //����ͬʱѡ��������Mes
		++nTotalFromMes;
		pCfg->nElephanSourceFrom = 1;
	}
	pCfg->nSoundPlusSourceFrom = 0;
	if(((CComboBox*)GetDlgItem(IDC_COMBO_SOUNDPLUS_FROM))->GetCurSel() == 1){
		++nTotalFromMes;
		pCfg->nSoundPlusSourceFrom = 1;
	}
	
	if (nTotalFromMes == 2){
		MessageBox("����key�ļ��������ļ�����ͬʱ����MES,����");
		return FALSE;
	}

	GetDlgItem(IDC_PRODUCT_BATCH)->GetWindowText(pCfg->strProductBatch); 

	pCfg->nElephantEn = (((CButton*)GetDlgItem(IDC_CHECK_ELEPHANT_EN))->GetCheck()==TRUE) ? 1 :0;
	pCfg->nSoundPlusEn = (((CButton*)GetDlgItem(IDC_CHECK_SOUNDPLAUS_EN))->GetCheck()==TRUE) ? 1: 0;

	///////////////////////////////////////add/////////////////
	GetDlgItem(IDC_ELAPHANT_PATH)->GetWindowText(pCfg->strElaphantPath); 
	GetDlgItem(IDC_SOUNDPLUS_PATH)->GetWindowText(pCfg->strSoundPlusPath); 
	//////////////////////////////////////////////

	m_tCfgInI = *pCfg;
	//////////////
	CString strIniFile;
	CString strTemp;
	strIniFile.Format("%s\\sngen\\SCS_SN_BES2500IZ_001.ini", CComFunc::GetCurrentPath());

	strTemp.Format("%d", m_tCfgInI.nElephantEn);
	WritePrivateProfileString("Config", "ElephantEn", (LPCTSTR)strTemp, strIniFile);

	strTemp.Format("%d", m_tCfgInI.nSoundPlusEn);
	WritePrivateProfileString("Config", "SoundPlusEn", (LPCTSTR)strTemp, strIniFile);

	WritePrivateProfileString("Config", "ProductBatch", (LPCTSTR)m_tCfgInI.strProductBatch, strIniFile);
	
	strTemp.Format("%I64X", m_tCfgInI.ElephantAddr);
	WritePrivateProfileString("Config", "ElephantAddr", (LPCTSTR)strTemp, strIniFile);
	
	strTemp.Format("%u", m_tCfgInI.ElephantLen);
	WritePrivateProfileString("Config", "ElephantLen", (LPCTSTR)strTemp, strIniFile);
	
	strTemp.Format("%d", m_tCfgInI.nElephanSourceFrom);
	WritePrivateProfileString("Config", "ElephanSourceFrom", (LPCTSTR)strTemp, strIniFile);
	
	strTemp.Format("%I64X", m_tCfgInI.SoundPlusAddr);
	WritePrivateProfileString("Config", "SoundPlusAddr", (LPCTSTR)strTemp, strIniFile);

	strTemp.Format("%u", m_tCfgInI.SoundPlusLen);
	WritePrivateProfileString("Config", "SoundPlusLen", (LPCTSTR)strTemp, strIniFile);
	
	strTemp.Format("%d", m_tCfgInI.nSoundPlusSourceFrom);
	WritePrivateProfileString("Config", "nSoundPlusSourceFrom", (LPCTSTR)strTemp, strIniFile);
	
	strTemp.Format("%I64X", m_tCfgInI.BLENameAddr);
	WritePrivateProfileString("Config", "BLENameAddr", (LPCTSTR)strTemp, strIniFile);

	strTemp.Format("%u", m_tCfgInI.BLENameLen);
	WritePrivateProfileString("Config", "BLENameLen", (LPCTSTR)strTemp, strIniFile);
	
	strTemp.Format("%I64X", m_tCfgInI.BLEMACAddr);
	WritePrivateProfileString("Config", "BLEMACAddr", (LPCTSTR)strTemp, strIniFile);
	
	strTemp.Format("%u", m_tCfgInI.BLEMACLen);
	WritePrivateProfileString("Config", "BLEMACLen", (LPCTSTR)strTemp, strIniFile);
	
	strTemp.Format("%I64X", m_tCfgInI.BTNameAddr);
	WritePrivateProfileString("Config", "BTNameAddr", (LPCTSTR)strTemp, strIniFile);

	strTemp.Format("%u", m_tCfgInI.BTNameLen);
	WritePrivateProfileString("Config", "BTNameLen", (LPCTSTR)strTemp, strIniFile);
	
	strTemp.Format("%I64X", m_tCfgInI.BTMACAddr);
	WritePrivateProfileString("Config", "BTMACAddr", (LPCTSTR)strTemp, strIniFile);

	strTemp.Format("%u", m_tCfgInI.BTMACLen);
	WritePrivateProfileString("Config", "BTMACLen", (LPCTSTR)strTemp, strIniFile);

	WritePrivateProfileString("Config", "BLEContent", (LPCTSTR)m_tCfgInI.strBLEContent, strIniFile);
	WritePrivateProfileString("Config", "BTContent", (LPCTSTR)m_tCfgInI.strBTContent, strIniFile);
	
	///////////////////////add////////////////////
	WritePrivateProfileString("Config", "SoundPlusPath", (LPCTSTR)m_tCfgInI.strSoundPlusPath, strIniFile);
	WritePrivateProfileString("Config", "ElaphantPath", (LPCTSTR)m_tCfgInI.strElaphantPath, strIniFile);
	////////////////////////////////////////////////

	return TRUE;
}

BOOL CDlgSNCustom::GetCtrls(CSerial&lSerial)
{
	BOOL Ret=TRUE;
	Ret=SaveCtrls();
	if(Ret!=TRUE){
		return Ret;
	}
	return m_SnCfg.SerialOutCfgData(lSerial);
}

INT CDlgSNCustom::QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	CString Curtime;
	CTime Time;
	CSerial lSerial;
	CString strFstCol,strSecCol;
	UINT GroupCnt=4;
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	int nDllApiRet = 0;
	char pTemp[65536] = {0};
	int nTempSize = 65536;
	CString strTempSN_BLE_MAC;
	CString strTempSN_BT_MAC;
	CString strTempSN;
	CString strProduct_Type;
	CString strTmpBT_MAC;
	std::vector<BYTE> vHex;
	BYTE* pTempBuff = NULL;
	UINT uLenTempBuff = 0;
	UINT uRealLen = 0;

	UINT nFileSize1 = 0;
	BYTE* pBuff1 = NULL;

	UINT nFileSize2 = 0;
	BYTE* pBuff2 = NULL;

	if (pCfg->nElephantEn == 1){
		GroupCnt++;
	}

	if (pCfg->nSoundPlusEn == 1){
		GroupCnt++;
	}

	lSerial<<GroupCnt;
	//first group for  ble mac
	CString strsWifiMAC;

	//////////////////////////////
	if (pCfg->BLEMACLen <= 0){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "ble mac���õĳ��Ȳ���С��0");
		Ret = -1;
		goto __end;
	}

	if (pCfg->BTMACLen <= 0){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "bt mac���õĳ��Ȳ���С��0");
		Ret = -1;
		goto __end;
	}

	if (pCfg->BTNameLen <= 0){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "bt name���õĳ��Ȳ���С��0");
		Ret = -1;
		goto __end;
	}

	if (pCfg->BLENameLen <= 0){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "ble name���õĳ��Ȳ���С��0");
		Ret = -1;
		goto __end;
	}


	srand(clock());
	long Random= rand() * 65535 + rand() * 32761 + rand() * 256 + rand() * 128;
	strsWifiMAC.Format("%I64d", Random);
	strProduct_Type.Format("BLE_MAC");
	nDllApiRet = m_DllHelpApi.Product_GetSNorMAC( (LPSTR)(LPCSTR)strsWifiMAC, (LPSTR)(LPCSTR)pCfg->strProductBatch, (LPSTR)(LPCSTR)strProduct_Type, pTemp);
	m_FileLog.PrintLog(LOGLEVEL_ERR, "run Product_GetSNorMAC, BLE_MAC function result = %d , WifiMAC=%s, Product_Batch=%s, Product_Type=%s", nDllApiRet,
		strsWifiMAC, pCfg->strProductBatch, strProduct_Type); 
	if (nDllApiRet < 0){
		Ret = -1;
		goto __end;
	}
	strTempSN_BLE_MAC.Format("%s", pTemp);
	
	lSerial<<pCfg->BLEMACAddr;
	lSerial<<pCfg->BLEMACLen;
	//////////////////////////////////////
	uLenTempBuff = pCfg->BLEMACLen;
	pTempBuff = new BYTE[uLenTempBuff];
	if (pTempBuff== NULL){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "�����ڴ�ʧ��");
		Ret = -1;
		goto __end;
	}
	memset(pTempBuff, 0, uLenTempBuff);
	uRealLen = strTempSN_BLE_MAC.GetLength();
	if (uRealLen == 0){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "��ȡ����ʵ��BLE_MAC����Ϊ0");
		Ret = -1;
		goto __end;
	}
	if ( uRealLen > uLenTempBuff){
		uRealLen = uLenTempBuff;	
	}
	memcpy(pTempBuff,(BYTE*)strTempSN_BLE_MAC.GetBuffer(), uRealLen);
	lSerial.SerialInBuff((BYTE*)pTempBuff, uLenTempBuff);
	
	delete [] pTempBuff;
	pTempBuff = NULL;
	////////////////////////////////////////////////////////

	strTempSN_BLE_MAC.ReleaseBuffer();
	m_FileLog.PrintLog(LOGLEVEL_ERR, "ble mac,  Addr=%I64X, Len=%u, real =%d", pCfg->BLEMACAddr,pCfg->BLEMACLen, strTempSN_BLE_MAC.GetLength()); 

	//second BT Mac
	strProduct_Type.Format("KEY");
	memset(pTemp, 0, nTempSize);
	nDllApiRet = m_DllHelpApi.Product_GetSNorMAC( (LPSTR)(LPCSTR)strsWifiMAC, (LPSTR)(LPCSTR)pCfg->strProductBatch, (LPSTR)(LPCSTR)strProduct_Type, pTemp );
	m_FileLog.PrintLog(LOGLEVEL_ERR, "run Product_GetSNorMAC, BT_MAC function result = %d ", nDllApiRet); 
	if (nDllApiRet < 0){
		Ret = -1;
		goto __end;
	}
	strTempSN_BT_MAC.Format("%s", pTemp);
	///�¼�
	strTmpBT_MAC.Format("%s", pTemp);
	//////////

	lSerial<<pCfg->BTMACAddr;
	lSerial<<pCfg->BTMACLen;
	////////////////////////////////////
	uLenTempBuff = pCfg->BTMACLen;
	pTempBuff = new BYTE[uLenTempBuff];
	if (pTempBuff== NULL){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "�����ڴ�ʧ��");
		Ret = -1;
		goto __end;
	}
	memset(pTempBuff, 0, uLenTempBuff);
	uRealLen = strTempSN_BT_MAC.GetLength();
	if (uRealLen == 0){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "��ȡ����ʵ��BT_MAC����Ϊ0");
		Ret = -1;
		goto __end;
	}
	if ( uRealLen > uLenTempBuff){
		uRealLen = uLenTempBuff;	
	}
	memcpy(pTempBuff,(BYTE*)strTempSN_BT_MAC.GetBuffer(), uRealLen);
	lSerial.SerialInBuff((BYTE*)pTempBuff, uLenTempBuff);

	delete [] pTempBuff;
	pTempBuff = NULL;
	/////////////////////////////////////////////
	strTempSN_BT_MAC.ReleaseBuffer();

	m_FileLog.PrintLog(LOGLEVEL_ERR, "BT Mac,  Addr=%I64X, Len=%u, real=%d", pCfg->BTMACAddr, pCfg->BTMACLen, strTempSN_BT_MAC.GetLength()); 

	//Third  ����
	if (pCfg->nElephantEn ==1){

		if (pCfg->ElephantLen <= 0){
			m_FileLog.PrintLog(LOGLEVEL_ERR, "Elephant���õĳ��Ȳ���С��0");
			Ret = -1;
			goto __end;
		}
		
		lSerial<<pCfg->ElephantAddr;
		lSerial<<pCfg->ElephantLen;
		if (pCfg->nElephanSourceFrom == 1){ //��Դ��Mes			
			CString strSQL;
			CString strValue;
			strValue.Format("%s", strTmpBT_MAC);
			memset(pTemp, 0, nTempSize);
			strSQL.Format("select mes_otherdep_call_pkg.f_getBinbyMAC('%s','KEY','%s') from dual", pCfg->strProductBatch,  strValue);
			nDllApiRet = m_DllHelpApi.ATS_CALL_IT_SQL_API( (LPSTR)(LPCSTR)strSQL,pTemp, nTempSize );
			m_FileLog.PrintLog(LOGLEVEL_ERR, "run ATS_CALL_IT_SQL_API result = %d ", nDllApiRet); 
			if (nDllApiRet < 0){
				Ret = -1;
				goto __end;
			}

			strValue.Empty();
			strValue.Format("%s", pTemp);
			///////////////////////////////////////
			m_FileLog.PrintLog(LOGLEVEL_ERR, "ATS_CALL_IT_SQL_API ���ص�ԭʼ�ַ���=%s", strValue);
			strValue.Replace("@#", ""); 

			if (strValue.IsEmpty()){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "run ATS_CALL_IT_SQL_API ���ص�����Ϊ��"); 
				Ret = -1;
				goto __end;
			}

			vHex.clear();
			ComTool::Str2HexNoTruncate(strValue,ComTool::ENDIAN_BIG,vHex);
			if (vHex.size() == 0){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "�ַ���ת��16���Ƹ�ʽ��������"); 
				Ret = -1;
				goto __end;
			}
			BYTE* pHexData = new BYTE[vHex.size()];
			if (pHexData == NULL){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "�����ڴ����"); 
				Ret = -1;
				goto __end;
			}
			memset(pHexData, 0, vHex.size());
			std::copy(vHex.begin(), vHex.end(), pHexData);
			//lSerial.SerialInBuff(pHexData, vHex.size());

			delete [] pHexData;
			pHexData = NULL;

			////////////////////////////////////
			uLenTempBuff = pCfg->ElephantLen;
			pTempBuff = new BYTE[uLenTempBuff];
			if (pTempBuff== NULL){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "�����ڴ�ʧ��");
				Ret = -1;
				goto __end;
			}
			memset(pTempBuff, 0, uLenTempBuff);
			if (vHex.size() > uLenTempBuff){
				std::copy(vHex.begin(), vHex.begin() + uLenTempBuff, pTempBuff);
			}else{
				std::copy(vHex.begin(), vHex.end(), pTempBuff);
			}
			lSerial.SerialInBuff((BYTE*)pTempBuff, uLenTempBuff);

			delete [] pTempBuff;
			pTempBuff = NULL;
			/////////////////////////////////////////////

			m_FileLog.PrintLog(LOGLEVEL_ERR, "Elephant,  Addr=%I64X, Len=%u, real =%d", pCfg->ElephantAddr, pCfg->ElephantLen, vHex.size()); 
			/*lSerial.SerialInBuff((BYTE*)strValue.GetBuffer(),strValue.GetLength());
			strValue.ReleaseBuffer();*/
		}else{ //��Դ���ļ�
			CString strValue;
			CFile file;
			CString strFilePath;
			if(file.Open(m_strElaphantPath, CFile::modeRead|CFile::shareDenyNone,NULL)==FALSE){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "���ļ�ʧ��%s", m_strElaphantPath);
				Ret = -1;
				goto __end;
			}
			nFileSize1 = file.GetLength();
			pBuff1 = new BYTE[nFileSize1 + 1];
			if (pBuff1 == NULL){
				goto __end;
			}
			memset(pBuff1, 0, nFileSize1 +1);
			file.Read(pBuff1, nFileSize1);
			file.Close();

			strValue.Format("%s",pBuff1 );
			/////////////////////////////////////////////////////////////////////////

			uLenTempBuff = pCfg->ElephantLen;
			pTempBuff = new BYTE[uLenTempBuff];
			if (pTempBuff== NULL){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "�����ڴ�ʧ��");
				Ret = -1;
				goto __end;
			}
			memset(pTempBuff, 0, uLenTempBuff);
			uRealLen = strValue.GetLength();
			if (uRealLen == 0){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "��ȡ����ʵ��Elephant����Ϊ0");
				Ret = -1;
				goto __end;
			}
			if ( uRealLen > uLenTempBuff){
				uRealLen = uLenTempBuff;	
			}
			memcpy(pTempBuff,(BYTE*)strValue.GetBuffer(), uRealLen);
			lSerial.SerialInBuff((BYTE*)pTempBuff, uLenTempBuff);

			delete [] pTempBuff;
			pTempBuff = NULL;
			////////////////////////////////////////////////////////

			//lSerial.SerialInBuff((BYTE*)strValue.GetBuffer(),strValue.GetLength());
			strValue.ReleaseBuffer();
			m_FileLog.PrintLog(LOGLEVEL_ERR, "Elephant,  Addr=%I64X, Len=%u, real =%d", pCfg->ElephantAddr, pCfg->ElephantLen, strValue.GetLength()); 
		}
	}
	
	//four  bt name
	lSerial<<pCfg->BTNameAddr;
	lSerial<<pCfg->BTNameLen;
	strTempSN.Format("%s", pCfg->strBTContent);
	//////////////////////////////////////
	uLenTempBuff = pCfg->BTNameLen;
	pTempBuff = new BYTE[uLenTempBuff];
	if (pTempBuff== NULL){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "�����ڴ�ʧ��");
		Ret = -1;
		goto __end;
	}
	memset(pTempBuff, 0, uLenTempBuff);

	uRealLen = strTempSN.GetLength();
	if (uRealLen == 0){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "��ȡ����ʵ��bt name����Ϊ0");
		Ret = -1;
		goto __end;
	}
	if ( uRealLen > uLenTempBuff){
		uRealLen = uLenTempBuff;	
	}
	memcpy(pTempBuff,(BYTE*)strTempSN.GetBuffer(), uRealLen);
	lSerial.SerialInBuff((BYTE*)pTempBuff, uLenTempBuff);

	delete [] pTempBuff;
	pTempBuff = NULL;
	////////////////////////////////////////////////////////
	//lSerial.SerialInBuff((BYTE*)strTempSN.GetBuffer(),strTempSN.GetLength());
	strTempSN.ReleaseBuffer();
	m_FileLog.PrintLog(LOGLEVEL_ERR, "bt name,  Addr=%I64X, Len=%u, real=%d", pCfg->BTNameAddr, pCfg->BTNameLen, strTempSN.GetLength()); 
	
	//five ble name
	lSerial<<pCfg->BLENameAddr;
	lSerial<<pCfg->BLENameLen;
	strTempSN.Format("%s", pCfg->strBLEContent );
	//////////////////////////////////////
	uLenTempBuff = pCfg->BLENameLen;
	pTempBuff = new BYTE[uLenTempBuff];
	if (pTempBuff== NULL){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "�����ڴ�ʧ��");
		Ret = -1;
		goto __end;
	}
	memset(pTempBuff, 0, uLenTempBuff);

	uRealLen = strTempSN.GetLength();
	if (uRealLen == 0){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "��ȡ����ʵ��ble name����Ϊ0");
		Ret = -1;
		goto __end;
	}
	if ( uRealLen > uLenTempBuff){
		uRealLen = uLenTempBuff;	
	}
	memcpy(pTempBuff,(BYTE*)strTempSN.GetBuffer(), uRealLen);
	lSerial.SerialInBuff((BYTE*)pTempBuff, uLenTempBuff);

	delete [] pTempBuff;
	pTempBuff = NULL;
	////////////////////////////////////////////////////////
	//lSerial.SerialInBuff((BYTE*)strTempSN.GetBuffer(),strTempSN.GetLength());
	strTempSN.ReleaseBuffer();
	m_FileLog.PrintLog(LOGLEVEL_ERR, "ble name,  Addr=%I64X, Len=%u,  real=%d", pCfg->BLENameAddr, pCfg->BLENameLen, strTempSN.GetLength()); 

	//Six ����
	if (pCfg->nSoundPlusEn ==1){
		lSerial<<pCfg->SoundPlusAddr;
		lSerial<<pCfg->SoundPlusLen;

		if (pCfg->SoundPlusLen <= 0){
			m_FileLog.PrintLog(LOGLEVEL_ERR, "SoundPlus ���õĳ��Ȳ���С��0");
			Ret = -1;
			goto __end;
		}

		if (pCfg->nSoundPlusSourceFrom == 1){ //��Դ��Mes			
			CString strSQL;
			CString strValue;
			strValue.Format("%s", strTmpBT_MAC); //��Զ�̶����õ�BT_MAC
			memset(pTemp, 0, nTempSize);
			strSQL.Format("select mes_otherdep_call_pkg.f_getBinbyMAC('%s','KEY','%s') from dual", pCfg->strProductBatch,  strValue);
			nDllApiRet = m_DllHelpApi.ATS_CALL_IT_SQL_API( (LPSTR)(LPCSTR)strSQL,pTemp, nTempSize );
			m_FileLog.PrintLog(LOGLEVEL_ERR, "run ATS_CALL_IT_SQL_API,  function result = %d ", nDllApiRet); 
			if (nDllApiRet < 0){
				Ret = -1;
				goto __end;
			}

			strValue.Empty();
			strValue.Format("%s", pTemp);
			/////////////
			m_FileLog.PrintLog(LOGLEVEL_ERR, "ATS_CALL_IT_SQL_API ���ص�ԭʼ�ַ���=%s", strValue);
			strValue.Replace("@#", ""); 
			if (strValue.IsEmpty()){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "run ATS_CALL_IT_SQL_API ���ص�����Ϊ��"); 
				Ret = -1;
				goto __end;
			}

			vHex.clear();
			ComTool::Str2HexNoTruncate(strValue,ComTool::ENDIAN_BIG,vHex);
			if (vHex.size() == 0){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "�ַ���ת��16���Ƹ�ʽ��������"); 
				Ret = -1;
				goto __end;
			}
			BYTE* pHexData = new BYTE[vHex.size()];
			if (pHexData == NULL){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "�����ڴ����"); 
				Ret = -1;
				goto __end;
			}
			memset(pHexData, 0, vHex.size());
			std::copy(vHex.begin(), vHex.end(), pHexData);
			//lSerial.SerialInBuff(pHexData, vHex.size());
			delete [] pHexData;
			pHexData = NULL;

			////////////////////////////////////
			uLenTempBuff = pCfg->SoundPlusLen;
			pTempBuff = new BYTE[uLenTempBuff];
			if (pTempBuff== NULL){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "�����ڴ�ʧ��");
				Ret = -1;
				goto __end;
			}
			memset(pTempBuff, 0, uLenTempBuff);
			if (vHex.size() > uLenTempBuff){
				std::copy(vHex.begin(), vHex.begin() + uLenTempBuff, pTempBuff);
			}else{
				std::copy(vHex.begin(), vHex.end(), pTempBuff);
			}
			lSerial.SerialInBuff((BYTE*)pTempBuff, uLenTempBuff);

			delete [] pTempBuff;
			pTempBuff = NULL;
			/////////////////////////////////////////////

			m_FileLog.PrintLog(LOGLEVEL_ERR, "SoundPlus,  Addr=%I64X, Len=%u,  real=%d", pCfg->SoundPlusAddr, pCfg->SoundPlusLen, vHex.size());
			/*lSerial.SerialInBuff((BYTE*)strValue.GetBuffer(),strValue.GetLength());
			strValue.ReleaseBuffer();*/
		}else{ //��Դ���ļ�
			CString strValue;
			CFile file;
			CString strFilePath;
			if(file.Open(m_strSoundPlusPath, CFile::modeRead|CFile::shareDenyNone,NULL)==FALSE){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "���ļ�ʧ��%s", m_strSoundPlusPath);
				Ret = -1;
				goto __end;
			}
			nFileSize2 = file.GetLength();
			pBuff2 = new BYTE[nFileSize2 + 1];
			if (pBuff2 == NULL){
				goto __end;
			}
			memset(pBuff2, 0, nFileSize2 +1);
			file.Read(pBuff2, nFileSize2);
			file.Close();

			strValue.Format("%s",pBuff2 );
			/////////////////////////////////////////////////////////////////////////
			uLenTempBuff = pCfg->SoundPlusLen;
			pTempBuff = new BYTE[uLenTempBuff];
			if (pTempBuff== NULL){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "�����ڴ�ʧ��");
				Ret = -1;
				goto __end;
			}
			memset(pTempBuff, 0, uLenTempBuff);

			uRealLen = strValue.GetLength();
			if (uRealLen == 0){
				m_FileLog.PrintLog(LOGLEVEL_ERR, "��ȡ����ʵ��SoundPlus����Ϊ0");
				Ret = -1;
				goto __end;
			}
			if ( uRealLen > uLenTempBuff){
				uRealLen = uLenTempBuff;	
			}
			
			memcpy(pTempBuff,(BYTE*)strValue.GetBuffer(), uRealLen);
			lSerial.SerialInBuff((BYTE*)pTempBuff, uLenTempBuff);

			delete [] pTempBuff;
			pTempBuff = NULL;
			////////////////////////////////////////////////////////
			//lSerial.SerialInBuff((BYTE*)strValue.GetBuffer(),strValue.GetLength());
			strValue.ReleaseBuffer();

			m_FileLog.PrintLog(LOGLEVEL_ERR, "SoundPlus,  Addr=%I64X, Len=%u,  real=%d", pCfg->SoundPlusAddr, pCfg->SoundPlusLen, strValue.GetLength());
		}
	}

	if(*pSize<lSerial.GetLength()){///����ط�Ҫע�ⷵ��ʵ��ʹ�õ��ֽ�����ʵ�ʿ������Ĵ�С
		*pSize=lSerial.GetLength();
		Ret=-2; goto __end;
	}
	Ret=lSerial.GetLength();///����ֵ��Ҫ�����������ֽ���
	memcpy(pData,lSerial.GetBuffer(),lSerial.GetLength());

__end:

	if (pBuff1){
		delete [] pBuff1;
		pBuff1 = NULL;
	}

	if (pBuff2){
		delete [] pBuff2;
		pBuff2 = NULL;
	}

	if (pTempBuff){
		delete [] pTempBuff;
		pTempBuff = NULL;
	}

	return Ret;
}

INT CDlgSNCustom::TellResult(DWORD Idx,INT IsPass)
{
	INT Ret=0;
	return Ret;
}

INT CDlgSNCustom::PreLoad()
{
	INT Ret=0;
	InitOnce();
	return Ret;

}

INT CDlgSNCustom::CreatLog()
{
	CString strLogPath;
	CTime Time;
	CString strCurPath= CComFunc::GetCurrentPath();
	Time=CTime::GetCurrentTime();
	strLogPath.Format("%s\\sngen\\%d%02d%02d_%02d%02d%02d.txt", strCurPath,Time.GetYear(),Time.GetMonth(),Time.GetDay(),
		Time.GetHour(),Time.GetMinute(),Time.GetSecond());

	m_FileLog.SetLogFile(strLogPath);
	return 0;
}

void CDlgSNCustom::LoadInI()
{
	CHAR TmpBuf[512];
	CString strTemp;
	CString strIniFile;
	memset(TmpBuf,0,512);
	strIniFile.Format("%s\\sngen\\SCS_SN_BES2500IZ_001.ini", CComFunc::GetCurrentPath());
	if (!PathFileExists(strIniFile)){
		AfxMessageBox("�Ҳ���SCS_SN_BES2500IZ_001.ini�����ļ�������!");
		return ;
	}

	if (m_DllHelpApi.AttachDll() == FALSE){
		m_FileLog.PrintLog(LOGLEVEL_ERR, "ATS_Station_Managment.dll�������ӿ�ʧ�� "); ////
		return ;
	}
	
	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "BLEContent", "",  TmpBuf/*(LPSTR)(LPCTSTR)m_tCfgInI.strBLEContent*/, MAX_PATH, strIniFile);
	m_tCfgInI.strBLEContent.Format("%s", TmpBuf);
	
	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "BTContent", "", TmpBuf /*(LPSTR)(LPCTSTR)m_tCfgInI.strBTContent*/, MAX_PATH, strIniFile);
	m_tCfgInI.strBTContent.Format("%s", TmpBuf);

	/////////////////////////////add///////////////////
	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "SoundPlusPath", "",  TmpBuf, MAX_PATH, strIniFile);
	m_tCfgInI.strSoundPlusPath.Format("%s", TmpBuf);

	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "ElaphantPath", "",  TmpBuf, MAX_PATH, strIniFile);
	m_tCfgInI.strElaphantPath.Format("%s", TmpBuf);
	/////////////////////////////////////////////////////////
	

	m_tCfgInI.nElephantEn = GetPrivateProfileInt("Config", "ElephantEn ",0, strIniFile);
	m_tCfgInI.nSoundPlusEn = GetPrivateProfileInt("Config", "SoundPlusEn ",0, strIniFile);

	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "ProductBatch", "",TmpBuf, MAX_PATH, strIniFile);
	m_tCfgInI.strProductBatch.Format("%s", TmpBuf);

	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "ElephantAddr", "",TmpBuf, MAX_PATH, strIniFile);
	strTemp.Format("%s", TmpBuf);
	sscanf(strTemp, "%I64X", &m_tCfgInI.ElephantAddr);

	m_tCfgInI.ElephantLen = GetPrivateProfileInt("Config", "ElephantLen ", 0, strIniFile);
	m_tCfgInI.nElephanSourceFrom = GetPrivateProfileInt("Config", "ElephanSourceFrom ", 0, strIniFile);

	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "SoundPlusAddr", "",TmpBuf, MAX_PATH, strIniFile);
	strTemp.Format("%s", TmpBuf);
	sscanf(strTemp, "%I64X", &m_tCfgInI.SoundPlusAddr);

	m_tCfgInI.SoundPlusLen = GetPrivateProfileInt("Config", "SoundPlusLen ", 0, strIniFile);
	m_tCfgInI.nSoundPlusSourceFrom = GetPrivateProfileInt("Config", "SoundPlusSourceFrom ", 0, strIniFile);
	//
	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "BLENameAddr", "",TmpBuf, MAX_PATH, strIniFile);
	strTemp.Format("%s", TmpBuf);
	sscanf(strTemp, "%I64X", &m_tCfgInI.BLENameAddr);

	m_tCfgInI.BLENameLen = GetPrivateProfileInt("Config", "BLENameLen ", 0, strIniFile);
	//
	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "BLEMACAddr", "",TmpBuf, MAX_PATH, strIniFile);
	strTemp.Format("%s", TmpBuf);
	sscanf(strTemp, "%I64X", &m_tCfgInI.BLEMACAddr);

	m_tCfgInI.BLEMACLen = GetPrivateProfileInt("Config", "BLEMACLen ", 0, strIniFile);
	////
	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "BTNameAddr", "",TmpBuf, MAX_PATH, strIniFile);
	strTemp.Format("%s", TmpBuf);
	sscanf(strTemp, "%I64X", &m_tCfgInI.BTNameAddr);

	m_tCfgInI.BTNameLen = GetPrivateProfileInt("Config", "BTNameLen ", 0, strIniFile);
	///
	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "BTMACAddr", "",TmpBuf, MAX_PATH, strIniFile);
	strTemp.Format("%s", TmpBuf);
	sscanf(strTemp, "%I64X", &m_tCfgInI.BTMACAddr);

	m_tCfgInI.BTMACLen = GetPrivateProfileInt("Config", "BTMACLen ", 0, strIniFile);

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	*pCfg  = m_tCfgInI;

}

void CDlgSNCustom::InitOnce()
{
	
	CreatLog();
	CString strMsg;
	strMsg.Format("����DLLʧ��");

	if (m_DllHelpApi.m_bLoadDll){
		strMsg.Format("����DLL�ɹ�");
	}
	m_FileLog.PrintLog(LOGLEVEL_ERR, strMsg);
	m_DllHelpApi.AttachLog(&m_FileLog);

	//UpdateData(FALSE);
	//m_FileLog.PrintLog(LOGLEVEL_ERR, );
}
void CDlgSNCustom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	/*DDX_Text(pDX, IDC_DEVICEID_ADDR, m_DeviceAddr);
	DDX_Text(pDX, IDC_DEVICEID_LEN, m_DeviceIDLen);
	DDX_Text(pDX, IDC_SECRET_ADDR, m_SecretAddr);
	DDX_Text(pDX, IDC_SECRET_LEN, m_SecretLen);*/
	//DDX_Text(pDX, IDC_CSVFILE, m_strCSVFile);
	DDX_Control(pDX, IDC_COMBO_ELEPHANT_FRROM, m_ComBoxElephantFrom);
	DDX_Control(pDX, IDC_COMBO_SOUNDPLUS_FROM, m_ComBoxSoundPlusFrom);
	DDX_Text(pDX, IDC_ELAPHANT_PATH, m_strElaphantPath);
	DDX_Text(pDX, IDC_SOUNDPLUS_PATH, m_strSoundPlusPath);
}


BEGIN_MESSAGE_MAP(CDlgSNCustom, CDialog)
	ON_EN_KILLFOCUS(IDC_CHECK_ELEPHANT_EN, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_CHECK_SOUNDPLAUS_EN, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_ELEPHANT_ADDR, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_ELEPHANT_LEN, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_SOUNDPLUS_ADDR, &CDlgSNCustom::OnEnKillfocusEdit)
	
	/////////////////////
	ON_EN_KILLFOCUS(IDC_COMBO_ELEPHANT_FRROM, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_PRODUCT_BATCH, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_ELAPHANT_PATH, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_COMBO_SOUNDPLUS_FROM, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_SOUNDPLUS_PATH, &CDlgSNCustom::OnEnKillfocusEdit)
	

	//ON_EN_KILLFOCUS(IDC_SECRET_LEN, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_BN_CLICKED(IDC_BTNSEL_ELAPHANT_PATH, &CDlgSNCustom::OnBnClickedBtnselElaphantPath)
	ON_BN_CLICKED(IDC_BTNSEL_SOUNDPLUS_PATH, &CDlgSNCustom::OnBnClickedBtnselSoundplusPath)
	ON_CBN_SELCHANGE(IDC_COMBO_ELEPHANT_FRROM, &CDlgSNCustom::OnCbnSelchangeComboElephantFrrom)
	ON_CBN_SELCHANGE(IDC_COMBO_SOUNDPLUS_FROM, &CDlgSNCustom::OnCbnSelchangeComboSoundplusFrom)
END_MESSAGE_MAP()


// CDlgSNCustom ��Ϣ�������

BOOL CDlgSNCustom::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	
	return FALSE;
	//return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void CDlgSNCustom::OnBnClickedBtnselcsv()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strFilePath;
	//CString strErrMsg;
	//CFileDialog dlgFile(TRUE, NULL, NULL, OFN_OVERWRITEPROMPT, "CSV File(*.csv)|*.csv||");
	//if (dlgFile.DoModal() != IDOK){
	//	return;
	//}
	//m_strCSVFile=dlgFile.GetPathName();
	//
	//m_CSVFile.CloseFile();
	//if(m_CSVFile.OpenFile(m_strCSVFile)!=0){
	//	strErrMsg=m_CSVFile.GetErrMsg();
	//	MessageBox(strErrMsg,"",MB_OK);
	//}
	UpdateData(FALSE);
}

void CDlgSNCustom::OnEnKillfocusEdit()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	SaveCtrls();
}

void CDlgSNCustom::OnBnClickedBtnselElaphantPath()
{
	CString strExts;
	strExts.Format(" Files(*.*)|*.*||");
	CFileDialog Dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST, strExts, this);
	if (Dlg.DoModal() == IDOK) {
		m_strElaphantPath = Dlg.GetPathName();
		//strFileOpen = Dlg.GetPathName();
	}
	else {
		m_strElaphantPath.Empty();
		//strFileOpen = "";
	}
	
	UpdateData(FALSE);
}

void CDlgSNCustom::OnBnClickedBtnselSoundplusPath()
{
	CString strExts;
	strExts.Format(" Files(*.*)|*.*||");
	CFileDialog Dlg(TRUE, NULL, NULL, OFN_PATHMUSTEXIST, strExts, this);
	if (Dlg.DoModal() == IDOK) {
		m_strSoundPlusPath = Dlg.GetPathName();
	}
	else {
		m_strSoundPlusPath  = "";
	}
	UpdateData(FALSE);
}

void CDlgSNCustom::OnCbnSelchangeComboElephantFrrom()
{
	OnEnKillfocusEdit();
}

void CDlgSNCustom::OnCbnSelchangeComboSoundplusFrom()
{
	OnEnKillfocusEdit();
}
