#include "StdAfx.h"
#include "../Com/Serial.h"
#include "SNCustomCfg.h"


CSNCustomCfg::CSNCustomCfg(void)
{
	/*m_Cfg.SNStartAddr=0xFFF0;
	m_Cfg.SNFixSize=8;
	m_Cfg.strSNFix="";*/
}

CSNCustomCfg::~CSNCustomCfg(void)
{
}

BOOL CSNCustomCfg::SerialInCfgData(CSerial& lSerial)
{
	try{
		lSerial>>/*m_Cfg.strAccount>>m_Cfg.strPassword>>*/m_Cfg.strMoLotNo>>m_Cfg.uSNAddr;
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}

BOOL CSNCustomCfg::SerialOutCfgData(CSerial& lSerial)
{
	try{
		lSerial<</*m_Cfg.strAccount<<m_Cfg.strPassword<<*/m_Cfg.strMoLotNo<<m_Cfg.uSNAddr;
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}
