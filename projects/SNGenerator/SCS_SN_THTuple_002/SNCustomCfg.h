#pragma once



typedef struct tagSNCustomCfg
{
	CString URLPara;
	CString DownloadIPAddr;
	UINT DownloadIPPort;
	CString GDHPara;
	CString UploadIPAddr;
	UINT UploadIPPort;
	UINT64 MACSNAddr;
	UINT MACSNLen;
	CString WorkOrderID;
	CString CachePath;
	CString _MATNR;
	CString _TYPE1;
	CString _DOTYPE;
	CString _USERID;
	CString _FILEVERSION;
	CString _RemoteUpdate;
	CString Datecode;
	CString KehuName;
	CString ModelID;
	CString Version;
	CString FwId;
	CString DeviceInfo;
	CString Token;
	CString KeyType;
	CString TestLightcount;
	CString _MultiLightIndex;
	CString _TestLightIndex;
	CString PC_MAC;
	CString strOrderID; //Î¯Íâµ¥ºÅ
	CString strDomain; //url
	CString strCode;
	CString strSecretId;
	CString strSecretKey;
	CString strAlgorithm;
	UINT nMode;
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
