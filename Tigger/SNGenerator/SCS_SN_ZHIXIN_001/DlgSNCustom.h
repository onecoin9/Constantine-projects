#pragma once

#include "../Com/Serial.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "../Com/ComTool.h"
#include "SNCustomCfg.h"
// CDlgSNCustom 对话框

typedef struct tagSNBurnInfo
{
	UINT64 Addr;///DeviceID地址
	UINT Len;	///DeviceID长度
	std::vector<BYTE> vData;

	void ReInit(){
		Addr = 0;
		Len = 0;
		vData.clear();
	}
	tagSNBurnInfo(){
		ReInit();
	}
}tSNBurnInfo;

class CDlgSNCustom : public CDialog
{
	DECLARE_DYNAMIC(CDlgSNCustom)

public:
	CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSNCustom();

	BOOL InitCtrls(CSerial& lSerial);
	BOOL GetCtrls(CSerial&lSerial);
	INT QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize);
	INT TellResult(DWORD Idx,INT IsPass);
	INT PreLoad();

// 对话框数据
	enum { IDD = IDD_DLGSNCUSTOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	//virtual BOOL OnInitDialog();
	BOOL InitCtrlsValue(CSNCustomCfg& SNCfg);

public:
	// 固定部分的SN字节数
	DWORD m_SNFixSize;
	CString m_strSNFix;
	CSNCustomCfg m_SnCfg;
	DRVSNCFGPARA *m_pSnCfgPara;
public:
	ULONGLONG m_SNStartAddr;

	BOOL GetDataFromShareMem();
	std::vector<tSNBurnInfo> m_vSNBurnInfo;
	CString m_strJsonData;
};
