#include "StdAfx.h"
#include "../Com/Serial.h"
#include "SNCustomCfg.h"

unsigned char wrap_key1[] = { 0x87, 0xD4, 0xAF, 0x1A, 0xF1, 0xEB, 0x50, 0xCB };

CSNCustomCfg::CSNCustomCfg(void)
{
}

CSNCustomCfg::~CSNCustomCfg(void)
{
}

BOOL CSNCustomCfg::SerialInCfgData(CSerial& lSerial)
{
	try{
		UINT temp1 = cfg_.cmp_mode;
		UINT temp2 = cfg_.t02_as_str;
		UINT temp3 = cfg_.delete_before;
		UINT temp4 = cfg_.serial_enc;
		lSerial << cfg_.esp_exe_path << cfg_.exe_param << cfg_.data_path << cfg_.log_path << cfg_.key_path << cfg_.first_num << cfg_.last_num <<
			temp1 << temp2 << temp3 << temp4;
		return TRUE;
	}
	catch(...){
		return FALSE;
	}
}

BOOL CSNCustomCfg::SerialOutCfgData(CSerial& lSerial)
{
	try{
		UINT temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
		lSerial >> cfg_.esp_exe_path >> cfg_.exe_param >> cfg_.data_path >> cfg_.log_path >> cfg_.key_path >> cfg_.first_num >> cfg_.last_num >>
			temp1 >> temp2 >> temp3 >> temp4;
		cfg_.cmp_mode = !!temp1;
		cfg_.t02_as_str = !!temp2;
		cfg_.delete_before = !!temp3;
		cfg_.serial_enc = !!temp4;
		return TRUE;
	}
	catch(...){
		return FALSE;
	}
}
