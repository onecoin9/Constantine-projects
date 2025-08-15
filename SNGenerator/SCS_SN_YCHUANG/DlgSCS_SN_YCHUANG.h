#pragma once

// CDlgSCS_SN_YCHUANG 对话框
#include "SCS_SN_YCHUANGCfg.h"
#include "resource.h"
#include "afxwin.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"

class CDlgSCS_SN_YCHUANG : public CDialog
{
	DECLARE_DYNAMIC(CDlgSCS_SN_YCHUANG)

public:
	CDlgSCS_SN_YCHUANG(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSCS_SN_YCHUANG();

// 对话框数据
	enum { IDD = IDD_DLGSCS_SN_YCHUANG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()	
public:
	virtual BOOL OnInitDialog();
	BOOL InitCtrlsValue();
	BOOL InitCtrlsValueWithoutWnd();
	BOOL GetCtrlsValue();
	INT QuerySN(DWORD Idx,BYTE*pData,INT*pSize);

private:
	CSCS_SN_YCHUANGCfg m_SnYCHUANGCfg;
	DRVSNCFGPARA *m_pSnCfgPara;
	BOOL InitCtrlsValue(CSCS_SN_YCHUANGCfg& SNYCHUANGCfg);

protected:
	virtual void OnOK();
};
