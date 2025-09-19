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
		lSerial>>m_Cfg.ProductsName>>m_Cfg.SNAddr>>m_Cfg.SNLen>>m_Cfg.BufAddr>>m_Cfg.IsRemap;
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}

BOOL CSNCustomCfg::SerialOutCfgData(CSerial& lSerial)
{
	try{
		lSerial<<m_Cfg.ProductsName<<m_Cfg.SNAddr<<m_Cfg.SNLen<<m_Cfg.BufAddr<<m_Cfg.IsRemap;
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}
