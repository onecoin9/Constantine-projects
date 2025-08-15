#pragma once


// CDlgSNScan 对话框

#include "SNScanCfg.h"
#include "SNScanList.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "afxwin.h"



class CDlgSNScan : public CDialog
{
	DECLARE_DYNAMIC(CDlgSNScan)

public:
	CDlgSNScan(DRVSNCFGPARA *pSNCfgPara,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSNScan();

// 对话框数据
	enum { IDD = IDD_DLGSNSCAN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	DRVSNCFGPARA *m_pSNCfgPara;
	CSNScanCfg m_SNScanCfg;
	CSNScanList m_ListCtrlST;
	std::vector<CString>vHeader;

	
	BOOL InitCtrlsValue(CSNScanCfg& SNScanCfg);
	BOOL InitCtrlList();

public:
	BOOL InitCtrls(CSerial& lSerial);
	BOOL GetCtrls(CSerial&lSerial);
	INT QuerySN(DWORD Idx,BYTE*pData,INT*pSize);
	INT FetchSN( DWORD Idx,BYTE*pData,INT*pSize );
	INT FetchSNForDB( DWORD Idx,BYTE*pData,INT*pSize);
	INT SetUID(DWORD Idx,BYTE*pUID,INT Size);

	LRESULT SetText(INT nRow,INT nColumn,CString&strText);
	CString GetTipText(INT nRow,INT nColumn);
	UINT GetTextInfo(INT subCmd,INT nRow,INT nColumn);
	INT SNUUIDResult(DWORD Idx,INT IsPass);/////////////////
	CString GetCurrentPath( void );
private:
	CComboBox m_GroupCnt;
	CString m_strRuleIni;
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedBtninisel();
	BOOL m_bScannerEn;
	BOOL m_bUIDEn;
	BOOL m_bRuleDllTest;
	CComboBox m_cmbSNRules;
	afx_msg void OnCbnSelchangeCmbsnrule();
	afx_msg void OnBnClickedChkscanneren();
	afx_msg void OnBnClickedUiden();
protected:
	virtual void OnOK();
public:
	afx_msg void OnBnClickedChktestruledll();
};
