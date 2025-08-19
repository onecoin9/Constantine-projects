#pragma once


typedef struct tagSNCustomCfg
{
	UINT64 SNStartAddr; ///SN的起始地址
	UINT SNFixSize;		///配置的固定SN部分的大小
	CString strSNFix; ///配置的固定SN的部分
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
