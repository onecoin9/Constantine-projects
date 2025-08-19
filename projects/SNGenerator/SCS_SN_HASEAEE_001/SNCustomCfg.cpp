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
		lSerial>>m_Cfg.DeviceIDAddr>>m_Cfg.DeviceIDLen;
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}

BOOL CSNCustomCfg::SerialOutCfgData(CSerial& lSerial)
{
	try{
		lSerial<<m_Cfg.DeviceIDAddr<<m_Cfg.DeviceIDLen;
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}
