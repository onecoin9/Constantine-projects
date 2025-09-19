#pragma once

typedef struct tagSNCustomCfg
{
	CString esp_exe_path;
	CString exe_param;
	CString data_path;
	CString log_path;
	UINT first_num;
	UINT last_num;
	bool cmp_mode;
	bool t02_as_str;
	bool delete_before;
	bool serial_enc;
	CString key_path;
}tSNCustomCfg;

class CSNCustomCfg
{
public:
	CSNCustomCfg(void);
	~CSNCustomCfg(void);
	BOOL SerialInCfgData(CSerial& lSerial);
	BOOL SerialOutCfgData(CSerial& lSerial);

	tSNCustomCfg* GetCustomCfg(){
		return &cfg_; 
	}

private:
	tSNCustomCfg cfg_;
};
