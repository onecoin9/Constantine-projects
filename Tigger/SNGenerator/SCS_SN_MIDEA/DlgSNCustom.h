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
// CDlgSNCustom 对话框

typedef struct
{
	uint8_t SNBlock;   // SN 需要烧录到哪一个位置, 1表示main block, 2表示 info block, ohter 保留
	uint32_t SNAddr;   // SN 需要烧录到的地址 32bits, 小端存放
	uint32_t SNLen;    // SN 需要烧录到的长度 32bits, 小端存放
	//uint8_t *SNData;   // SN 数据，长度 = SNLen
	CString  SNData;
} SN_InfoTpye;

//typedef struct
//{
//	uint8_t SNBlock;   // SN 需要烧录到哪一个位置, 1表示main block, 2表示 info block, ohter 保留
//	uint32_t SNAddr;   // SN 需要烧录到的地址 32bits, 小端存放
//	uint32_t SNLen;    // SN 需要烧录到的长度 32bits, 小端存放
//	uint8_t SNData[0];   // SN 数据，长度 = SNLen
//} SN_InfoTpye;

typedef struct
{
	char MagicNum[8];     // 固定字符串，“SNALV” ，后面补零
	uint16_t Version;	  // 版本号 初始为 0x0001，小端存放
	uint8_t SNInfoCnt;    // 该座子需要烧录几组SN
	SN_InfoTpye *SnInfo; // SN信息，一共SNInfoCnt组
} SN_ALV_Type;


class CDlgSNCustom : public CDialog
{
	DECLARE_DYNAMIC(CDlgSNCustom)

public:
	CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent = NULL);   // 标准构造函数
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

// 对话框数据
	enum { IDD = IDD_DLGSNCUSTOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

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
	// 固定部分的SN字节数
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
	CString m_strChipName;///tag中的芯片名
	INT ChipNameLen;
	INT m_BufferType;
	CString m_strTagInfo;
	CString m_strAprChipName;//加载工程时，界面上显示的芯片名
	CString m_strManuName;  ///厂商名
	BOOL ISMessage;
	BOOL AddOption;
	BOOL Changeover;
};
