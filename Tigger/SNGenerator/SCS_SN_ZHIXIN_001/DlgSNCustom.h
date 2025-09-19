#pragma once

#include "../Com/Serial.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "../Com/ComTool.h"
#include "SNCustomCfg.h"
// CDlgSNCustom �Ի���

typedef struct tagSNBurnInfo
{
	UINT64 Addr;///DeviceID��ַ
	UINT Len;	///DeviceID����
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
	CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgSNCustom();

	BOOL InitCtrls(CSerial& lSerial);
	BOOL GetCtrls(CSerial&lSerial);
	INT QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize);
	INT TellResult(DWORD Idx,INT IsPass);
	INT PreLoad();

// �Ի�������
	enum { IDD = IDD_DLGSNCUSTOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	//virtual BOOL OnInitDialog();
	BOOL InitCtrlsValue(CSNCustomCfg& SNCfg);

public:
	// �̶����ֵ�SN�ֽ���
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
