#pragma once

#include "../Com/Serial.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "../Com/ComTool.h"
#include "SNCustomCfg.h"
#include "CSVFile.h"
// CDlgSNCustom �Ի���

class CDlgSNCustom : public CDialog
{
	DECLARE_DYNAMIC(CDlgSNCustom)

public:
	CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgSNCustom();

	BOOL InitCtrls(CSerial& lSerial);
	BOOL GetCtrls(CSerial&lSerial);
	INT QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize);
	INT TellResult(DWORD Idx,INT IsPass);
	INT PreLoad();
	CString GetCurrentPath( void );

// �Ի�������
	enum { IDD = IDD_DLGSNCUSTOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	BOOL InitCtrlsValue(CSNCustomCfg& SNCfg);
	BOOL SaveCtrls();
public:
	// �̶����ֵ�SN�ֽ���
	CSNCustomCfg m_SnCfg;
	DRVSNCFGPARA *m_pSnCfgPara;
	CCSVFile m_CSVFile;
public:
	ULONGLONG m_SNStartAddr;
	LONGLONG m_DeviceAddr;
	DWORD m_DeviceIDLen;
	LONGLONG m_SecretAddr;
	DWORD m_SecretLen;
	CString m_strCSVFile;
public:
	afx_msg void OnBnClickedBtnselcsv();
public:
	
public:
	afx_msg void OnEnKillfocusEdit();
};
