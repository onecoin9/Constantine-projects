#include "StdAfx.h"
#include "../Com/Serial.h"
#include "SNCustomCfg.h"


CSNCustomCfg::CSNCustomCfg(void)
{
}

CSNCustomCfg::~CSNCustomCfg(void)
{
}

BOOL CSNCustomCfg::SerialInCfgData(CSerial& lSerial)
{
	try{
		lSerial>>m_Cfg.DeviceIDLen>>m_Cfg.ConnpublicKeyLen>>m_Cfg.DeviceIDAddr>>m_Cfg.KeyAddr>>m_Cfg.strID>>m_Cfg.strFactory>>m_Cfg.strYear>> \
			m_Cfg.strMonth>>m_Cfg.strProductSN>>m_Cfg.strCSVFile;
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}

BOOL CSNCustomCfg::SerialOutCfgData(CSerial& lSerial)
{
	CString strID;
	CString strFactory;
	CString strYear;
	CString strMonth;
	CString strProductSN;
	
	strID.Format("%s", m_Cfg.strID);
	strFactory.Format("%s", m_Cfg.strFactory);
	strYear.Format("%s", m_Cfg.strYear);
	strMonth.Format("%s", m_Cfg.strMonth);
	strProductSN.Format("%s", m_Cfg.strProductSN);

	try{
		lSerial<<m_Cfg.DeviceIDLen<<m_Cfg.ConnpublicKeyLen<<m_Cfg.DeviceIDAddr<<m_Cfg.KeyAddr<<strID<<strFactory<<strYear<<strMonth\
			<<strProductSN<<m_Cfg.strCSVFile;
		/*lSerial.SerialInBuff((BYTE*)m_Cfg.strID.GetBuffer(), m_Cfg.strID.GetLength()); 
		lSerial.SerialInBuff((BYTE*)m_Cfg.strFactory.GetBuffer(), m_Cfg.strFactory.GetLength()); 
		lSerial.SerialInBuff((BYTE*)m_Cfg.strYear.GetBuffer(), m_Cfg.strYear.GetLength()); 
		lSerial.SerialInBuff((BYTE*)m_Cfg.strMonth.GetBuffer(), m_Cfg.strMonth.GetLength()); 
		lSerial.SerialInBuff((BYTE*)m_Cfg.strProductSN.GetBuffer(), m_Cfg.strProductSN.GetLength()); */
		
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}
