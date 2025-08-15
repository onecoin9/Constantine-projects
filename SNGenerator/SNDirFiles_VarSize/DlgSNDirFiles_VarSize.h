#pragma once

#include "SNDirFiles_VarSizeList.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "../Com/ComTool.h"
#include "../Com/Serial.h"
#include "SNDirFiles_VarSizeCfg.h"
#include "FileSearcher.h"
// CDlgSNDirFiles_VarSize �Ի���

#define GROUPCNT_MAX (20)

class CDlgSNDirFiles_VarSize : public CDialog
{
	DECLARE_DYNAMIC(CDlgSNDirFiles_VarSize)

public:
	CDlgSNDirFiles_VarSize(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgSNDirFiles_VarSize();
	BOOL InitCtrls(CSerial& lSerial);
	BOOL GetCtrls(CSerial&lSerial);
	INT QuerySN(DWORD Idx,BYTE*pData,INT*pSize);
	INT TellResult(DWORD Idx,INT IsPass);
	INT PreLoad();
	INT GetSNFeature(const char*strJsonIn, char* strJsonOut, INT SNSize);


	LRESULT SetText(INT nRow,INT nColumn,CString&strText);
	BOOL InitCtrlsValue(CSNDirFiles_VarSizeCfg& SNCfg);

// �Ի�������
	enum { IDD = IDD_DLGSNDirFiles_VarSize };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
	CSNDirFiles_VarSizeList m_ListCtrlST;
	DRVSNCFGPARA *m_pSnCfgPara;
	CComboBox m_cmbSNGroup;
	CString m_strSNMaxSize;
	CString m_strFillData;
	std::vector<CString>vHeader;
	BOOL InitCtrlList();

	CSNDirFiles_VarSizeCfg m_SnCfg;

	BOOL GetFileLength(CString strFile,UINT& Length);
	virtual BOOL OnInitDialog();
	
	CStdioFile m_logFile;
	CStdioFile m_FilePassResult;
	UINT m_PassFileID;

	INT OpenLog();
	INT OpenResultLog(CString strPassDir);
	void CloseLog();
	void CloseResult();
	void WriteLog(CString str);
	void WriteResult(CString str);
	INT RecordPassResult(CString FilePath);
	
	INT CreatePassDir( const CString& DirName);
	BOOL  MoveFileToPass(const CString&File,CString& DestDir);


	CFileSearcher m_vFileSearcher[GROUPCNT_MAX];
	INT m_SNTotalSize;///���кŸ��ⲿ��SN�ܹ���Ҫ���ٸ��ֽ�
public:
	afx_msg void OnCbnSelchangeCombo1();
	//afx_msg void OnBnClickedCheck1();
public:
	afx_msg void OnBnClickedButton1();
public:
	afx_msg void OnEnKillfocusEditData();
};
