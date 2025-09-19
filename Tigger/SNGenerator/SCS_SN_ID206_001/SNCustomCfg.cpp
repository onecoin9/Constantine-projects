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
		lSerial>>m_Cfg.UUIDAddr>>m_Cfg.uLenUUID>>m_Cfg.KeyAddr>>m_Cfg.uLenKEY>>m_Cfg.MacAddr>>m_Cfg.uLenMAC>>m_Cfg.LinkAddr>>m_Cfg.uLenLINK>>m_Cfg.strCSVFile;
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}

BOOL CSNCustomCfg::SerialOutCfgData(CSerial& lSerial)
{

	try{
		lSerial<<m_Cfg.UUIDAddr<<m_Cfg.uLenUUID<<m_Cfg.KeyAddr<<m_Cfg.uLenKEY<<m_Cfg.MacAddr<<m_Cfg.uLenMAC<<m_Cfg.LinkAddr<<m_Cfg.uLenLINK<<m_Cfg.strCSVFile;
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}
