#pragma once

// CDlgSN_EverCreation_001 对话框
#include "SN_EverCreation_001List.h"
#include "resource.h"
#include "afxwin.h"
#include "SN_EverCreation_001Cfg.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"

class CDlgSN_EverCreation_001 : public CDialog
{
	DECLARE_DYNAMIC(CDlgSN_EverCreation_001)

public:
	CDlgSN_EverCreation_001(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSN_EverCreation_001();

// 对话框数据
	enum { IDD = IDD_DLGSN_EverCreation_001 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()	
public:
	virtual BOOL OnInitDialog();
	BOOL InitCtrls(CSerial& lSerial);
	BOOL GetCtrls(CSerial&lSerial);
	INT QuerySN(DWORD Idx,BYTE*pData,INT*pSize);
	INT RunExcuCmdLine(DWORD nIdx, std::vector<BYTE>& vOut);

	LRESULT SetText(INT nRow,INT nColumn,CString&strText);///设置结果值
	CString GetTipText(INT nRow,INT nColumn);
	UINT GetTextInfo(INT subCmd,INT nRow,INT nColumn);
	
private:
	CRITICAL_SECTION m_csQuery;
	CSN_EverCreation_001List m_ListCtrlST;
	CSN_EverCreation_001Cfg m_SN_EverCreation_001Cfg;
	DRVSNCFGPARA *m_pSnCfgPara;
	BOOL InitCtrlList();
	BOOL InitCtrlsValue(CSN_EverCreation_001Cfg& SN_EverCreation_001Cfg);
	BOOL ModifySNValue(SNGROUP* pSnGroup,INT nRow,INT nColumn);
	BOOL StrDec2StrHex(CString&str);
	BOOL StrHex2StrDec(CString&str);
	std::vector<CString>vHeader;
protected:
	virtual void OnOK();
	CComboBox m_cmbSNGroup;
public:
	afx_msg void OnCbnSelchangeCombo1();
};
