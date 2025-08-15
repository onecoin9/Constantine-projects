#pragma once


typedef struct tagSNCustomCfg
{
	UINT64 SNAddr;
	UINT SNLen;
	CString ProductsName;
	UINT64 BufAddr;
	INT IsRemap;
	INT TestMode;
	UINT64 OptionAddr;

}tSNCustomCfg;

class CSNCustomCfg
{
public:
	CSNCustomCfg(void);
	~CSNCustomCfg(void);
	BOOL SerialInCfgData(CSerial& lSerial);
	BOOL SerialOutCfgData(CSerial& lSerial);
	tSNCustomCfg* GetCustomCfg(){return &m_Cfg;};
private:
	tSNCustomCfg m_Cfg;
};
