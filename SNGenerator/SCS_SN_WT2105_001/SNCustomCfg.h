#pragma once


typedef struct tagSNCustomCfg
{
	UINT DeviceIDLen;	///DeviceID长度
	UINT ConnpublicKeyLen;///ConnpublicKey的长度

	UINT64 DeviceIDAddr;
	UINT64 KeyAddr;
	
	UINT64 uFirstGroupAddr;

	CString strID;          //产品型号
	CString strFactory;  //生产工厂
	CString strYear;			//年
	CString strMonth;	//月
	CString strProductSN;  //流水号

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
