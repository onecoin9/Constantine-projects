#pragma once

#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "../Com/Serial.h"
#include <vector>

#include "SNCustomCfg.h"
#include "CSVFile.h"
// CDlgID206 对话框

//#define DATAMODE_DATA	 (1)
//#define DATAMODE_FILE	 (2)
//#define BYTEMODE_BE		(1)
//#define BYTEMODE_LE		(2)
//typedef struct tagSNInfo
//{
//	INT SNDataMode;  //DATAMODE_DATA etc
//	INT SNByteMode; ///BYTEMODE_BE etc
//	UINT64 LineIndex;///记录当前在多少行
//	UINT64 SNInfoStartLine;	///记录SNInfo在第几行
//	UINT64 SNInfoStartPos;///记录SNInfo的起始位置
//	INT RecordSize;	///一条记录的占用字节长度，也就是一行有多少个字节，后面用于索引很方便
//}tSNInfo;
//
//typedef struct tagSNOneGroup{
//	UINT64 SNAddr;
//	INT SNSize;
//	BYTE *pData;
//}tSNOneGroup;



class CDlgID206 : public CDialog
{
	DECLARE_DYNAMIC(CDlgID206)

public:
	CDlgID206(DRVSNCFGPARA *pSNCfgPara,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgID206();

// 对话框数据
	enum { IDD = IDD_DLGID206 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	DRVSNCFGPARA *m_pSNCfgPara;




public:
	afx_msg void OnBnClickedBtnselcsv();

	CSNCustomCfg m_SnCfg;
	DRVSNCFGPARA *m_pSnCfgPara;
	CCSVFile m_CSVFile;

	int GetFirstGroupData(CString& strData);
	INT DoGetFirstGroupData(CString& strData, DWORD Idx);

	INT PreLoad();
	INT TellResult(DWORD Idx,INT IsPass);
	BOOL GetCtrls(CSerial&lSerial);
	BOOL SaveCtrls();
	BOOL InitCtrls(CSerial& lSerial);
	BOOL InitCtrlsValue(CSNCustomCfg& SNCfg);
	INT QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize);
	afx_msg void OnEnKillfocusEdit();

private:
	CString m_strFile;
	CString m_strID;
	CString m_strFactory;
	CString m_strYear;
	CStdioFile m_ID206;
	CString m_strErrMsg;
	
	CString m_strCSVFile;
};
