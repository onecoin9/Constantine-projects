#pragma once

#include "stdint.h"
#include "../Com/Serial.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "../Com/ComTool.h"
#include "SNCustomCfg.h"
#include "DllHelpApi.h"
#include "CSVFile.h"
#include "LogFile.h"
#include "afxwin.h"
// CDlgSNCustom �Ի���

typedef struct
{
	uint8_t SNBlock;   // SN ��Ҫ��¼����һ��λ��, 1��ʾmain block, 2��ʾ info block, ohter ����
	uint32_t SNAddr;   // SN ��Ҫ��¼���ĵ�ַ 32bits, С�˴��
	uint32_t SNLen;    // SN ��Ҫ��¼���ĳ��� 32bits, С�˴��
	//uint8_t *SNData;   // SN ���ݣ����� = SNLen
	CString  SNData;
} SN_InfoTpye;

//typedef struct
//{
//	uint8_t SNBlock;   // SN ��Ҫ��¼����һ��λ��, 1��ʾmain block, 2��ʾ info block, ohter ����
//	uint32_t SNAddr;   // SN ��Ҫ��¼���ĵ�ַ 32bits, С�˴��
//	uint32_t SNLen;    // SN ��Ҫ��¼���ĳ��� 32bits, С�˴��
//	uint8_t SNData[0];   // SN ���ݣ����� = SNLen
//} SN_InfoTpye;

typedef struct
{
	char MagicNum[8];     // �̶��ַ�������SNALV�� �����油��
	uint16_t Version;	  // �汾�� ��ʼΪ 0x0001��С�˴��
	uint8_t SNInfoCnt;    // ��������Ҫ��¼����SN
	SN_InfoTpye *SnInfo; // SN��Ϣ��һ��SNInfoCnt��
} SN_ALV_Type;


class CDlgSNCustom : public CDialog
{
	DECLARE_DYNAMIC(CDlgSNCustom)

public:
	CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgSNCustom();

	BOOL InitCtrls(CSerial& lSerial);
	BOOL GetCtrls(CSerial&lSerial);
	INT QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize);
	//INT QuerySN(DRVSNCFGPARA *pSnCfgPara,DWORD Idx,BYTE*pData,INT*pSize);
	INT TellResult(DWORD Idx,INT IsPass);
	INT PreLoad();
	void InitOnce();
	INT CreatLog();
	void LoadInI();
	INT SetTagInfo(char*pData,char* pChipName,char* pManuName);
	BOOL ParseTagInfo();
	INT ChangeData(CString& strData);

// �Ի�������
	enum { IDD = IDD_DLGSNCUSTOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	BOOL InitCtrlsValue(CSNCustomCfg& SNCfg);
	BOOL SaveCtrls();
	BOOL GetComboValue(BOOL IsInit);
	afx_msg void OnCbnSelchangeComboProducts();
	afx_msg void OnEnKillfocusEditSnAddr();
	afx_msg void OnBnClickedCheckMode();
public:
	// �̶����ֵ�SN�ֽ���
	CSNCustomCfg m_SnCfg;
	DRVSNCFGPARA *m_pSnCfgPara;
	CCSVFile m_CSVFile;

	CDllHelp m_DllHelpApi;
	CLogFile m_FileLog;
	CString m_strProducts;
	CString m_strSNAddr;
	CString m_strSNLen;

	
	CString ProductsName;
	CDllHelp DllHelp;

	tagSNCustomCfg m_tCfgInI;

	std::vector<tSNCustomCfg> items;

	SN_InfoTpye m_SNInfo;
	SN_ALV_Type m_SNType;

	CString testID;

	CString m_strBufAddr;
	CString m_strChipName;///tag�е�оƬ��
	INT ChipNameLen;
	INT m_BufferType;
	CString m_strTagInfo;
	CString m_strAprChipName;//���ع���ʱ����������ʾ��оƬ��
	CString m_strManuName;  ///������
	BOOL ISMessage;
	BOOL AddOption;
	BOOL Changeover;
};
