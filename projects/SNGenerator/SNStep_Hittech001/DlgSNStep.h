#pragma once

// CDlgSNStep 对话框
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

	BOOL InitCtrls(CSerial& lSerial);
	BOOL GetCtrls(CSerial& lSerial);
	INT QuerySN(DWORD Idx, BYTE* pData, INT* pSize);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	afx_msg void OnEnKillfocusItemEdit();

	DECLARE_MESSAGE_MAP()	

private:
	CString Integer2Str(UINT16 value, INT base = 10);

private:
	DRVSNCFGPARA* m_pSnCfgPara;
	CSNStepCfg m_SnStepCfg;
};
