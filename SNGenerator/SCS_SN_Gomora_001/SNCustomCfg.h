#pragma once


typedef struct tagSNCustomCfg
{
	UINT64 DeviceIDAddr;///DeviceID��ַ
	UINT DeviceIDLen;	///DeviceID����
	UINT64 SecretAddr;///Secret�ĵ�ַ
	UINT SecretLen;///Secret�ĳ���
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
