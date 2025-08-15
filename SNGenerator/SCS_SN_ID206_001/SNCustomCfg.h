#pragma once


typedef struct tagSNCustomCfg
{
	UINT64 uStartAddr;
	UINT uTotalLen;
	UINT uLenUUID;
	UINT uLenKEY;
	UINT uLenMAC;
	UINT uLenLINK;

	UINT64 UUIDAddr;
	UINT64 KeyAddr;
	UINT64 MacAddr;
	UINT64 LinkAddr;

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
