#pragma once


typedef struct tagSNCustomCfg
{
	int nElephantEn;
	int nSoundPlusEn;
	CString strProductBatch;

	UINT64 ElephantAddr;
	UINT ElephantLen;
	int nElephanSourceFrom;

	UINT64 SoundPlusAddr;
	UINT SoundPlusLen;
	int nSoundPlusSourceFrom;

	UINT64 BLENameAddr;
	UINT	BLENameLen;

	UINT64 BLEMACAddr;
	UINT	BLEMACLen;

	UINT64 BTNameAddr;
	UINT	BTNameLen;

	//界面上输入
	CString strBLEContent;
	CString strBTContent;

	UINT64 BTMACAddr;
	UINT	BTMACLen;
	//////////////增加///////////////
	CString strElaphantPath;
	CString strSoundPlusPath;
	/////////////////////////////
	void ReInit(){
		nElephantEn = 0;
		nSoundPlusEn = 0;
		strProductBatch.Empty();
		ElephantAddr = 0;
		ElephantLen = 0;
		nElephanSourceFrom = 0;
		SoundPlusAddr = 0;
		SoundPlusLen = 0;
		nSoundPlusSourceFrom =0;
		BLENameAddr =0;
		BLENameLen = 0;
		BLEMACAddr = 0;
		BLEMACLen = 0;
		BTNameAddr = 0;
		BTNameLen = 0;
		strBLEContent.Empty();
		strBTContent.Empty();
		BTMACAddr = 0;
		BTMACLen = 0;
		strElaphantPath.Empty();
		strSoundPlusPath.Empty();
	}
	tagSNCustomCfg(){
		ReInit();
	}
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
