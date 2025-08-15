#pragma once

// CDlgSNStep 对话框
#include "SNStepList.h"
#include "resource.h"
#include "afxwin.h"
#include "SNStepCfg.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"

class CDlgSNStep : public CDialog
{
	DECLARE_DYNAMIC(CDlgSNStep)

public:
	CDlgSNStep(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSNStep();

// 对话框数据
	enum { IDD = IDD_DLGSNSTEP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()	
public:
	virtual BOOL OnInitDialog();
	BOOL InitCtrls(CSerial& lSerial);
	BOOL GetCtrls(CSerial&lSerial);
	INT QuerySN(DWORD Idx,BYTE*pData,INT*pSize);


	LRESULT SetText(INT nRow,INT nColumn,CString&strText);///设置结果值
	CString GetTipText(INT nRow,INT nColumn);
	UINT GetTextInfo(INT subCmd,INT nRow,INT nColumn);

	void SkipStepAdd(DWORD Idx,BYTE*pData,INT*pSize, std::vector<BYTE>&vDecDataOut);
	BOOL SkipStepIncrease(UINT EndianType,std::vector<BYTE>&vStartBase,std::vector<BYTE>&vIncrease,std::vector<BYTE>&vDecDataOut);

	
private:
	CSNStepList m_ListCtrlST;
	CSNStepCfg m_SnStepCfg;
	DRVSNCFGPARA *m_pSnCfgPara;
	BOOL InitCtrlList();
	BOOL InitCtrlsValue(CSNStepCfg& SNStepCfg);
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
