#pragma once


typedef struct tagSNCustomCfg
{
	UINT64 DeviceIDAddr;///DeviceID地址
	UINT DeviceIDLen;	///DeviceID长度
	UINT64 SecretAddr;///Secret的地址
	UINT SecretLen;///Secret的长度
	CString strCSVFile;
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
