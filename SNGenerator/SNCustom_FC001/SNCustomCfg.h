#pragma once


typedef struct tagSNCustomCfg
{
	UINT64 SNStartAddr; ///SN����ʼ��ַ
	UINT SNFixSize;		///���õĹ̶�SN���ֵĴ�С
	CString strSNFix; ///���õĹ̶�SN�Ĳ���
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
