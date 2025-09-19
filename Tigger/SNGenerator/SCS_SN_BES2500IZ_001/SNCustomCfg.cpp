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
		lSerial>>m_Cfg.nElephantEn>>m_Cfg.nSoundPlusEn>>m_Cfg.strProductBatch>>m_Cfg.ElephantAddr>>m_Cfg.ElephantLen \
			>>m_Cfg.nElephanSourceFrom>>m_Cfg.SoundPlusAddr >>m_Cfg.SoundPlusLen >>m_Cfg.nSoundPlusSourceFrom >> \
			m_Cfg.BLENameAddr >> m_Cfg.BLENameLen >>m_Cfg.BLEMACAddr >>m_Cfg.BLEMACLen >>m_Cfg.BTNameAddr >>\
			m_Cfg.BTNameLen >> m_Cfg.strBLEContent >>m_Cfg.strBTContent >>m_Cfg.BTMACAddr >>m_Cfg.BTMACLen>>\
			m_Cfg.strElaphantPath>>m_Cfg.strSoundPlusPath ;
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}

BOOL CSNCustomCfg::SerialOutCfgData(CSerial& lSerial)
{
	try{
		lSerial<<m_Cfg.nElephantEn<<m_Cfg.nSoundPlusEn<<m_Cfg.strProductBatch<<m_Cfg.ElephantAddr<<m_Cfg.ElephantLen \
			<<m_Cfg.nElephanSourceFrom<<m_Cfg.SoundPlusAddr <<m_Cfg.SoundPlusLen<<m_Cfg.nSoundPlusSourceFrom<< \
			m_Cfg.BLENameAddr<< m_Cfg.BLENameLen<<m_Cfg.BLEMACAddr<<m_Cfg.BLEMACLen<<m_Cfg.BTNameAddr<<\
			m_Cfg.BTNameLen<< m_Cfg.strBLEContent<<m_Cfg.strBTContent<<m_Cfg.BTMACAddr<<m_Cfg.BTMACLen<<\
			m_Cfg.strElaphantPath<<m_Cfg.strSoundPlusPath;
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}
