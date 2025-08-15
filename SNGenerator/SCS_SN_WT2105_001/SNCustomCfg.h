#pragma once


typedef struct tagSNCustomCfg
{
	UINT DeviceIDLen;	///DeviceID����
	UINT ConnpublicKeyLen;///ConnpublicKey�ĳ���

	UINT64 DeviceIDAddr;
	UINT64 KeyAddr;
	
	UINT64 uFirstGroupAddr;

	CString strID;          //��Ʒ�ͺ�
	CString strFactory;  //��������
	CString strYear;			//��
	CString strMonth;	//��
	CString strProductSN;  //��ˮ��

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
