#pragma once

#include "SNDirFilesList.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "../Com/ComTool.h"
#include "../Com/Serial.h"
#include "SNDirFilesCfg.h"
#include "FileSearcher.h"
// CDlgSNDirFiles �Ի���

#define GROUPCNT_MAX (20)

class CDlgSNDirFiles : public CDialog
{
	DECLARE_DYNAMIC(CDlgSNDirFiles)

public:
	CDlgSNDirFiles(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgSNDirFiles();
	BOOL InitCtrls(CSerial& lSerial);
	BOOL GetCtrls(CSerial&lSerial);
	INT QuerySN(DWORD Idx,BYTE*pData,INT*pSize);
	INT TellResult(DWORD Idx,INT IsPass);
	INT PreLoad();


	LRESULT SetText(INT nRow,INT nColumn,CString&strText);
	BOOL InitCtrlsValue(CSNDirFilesCfg& SNCfg);

// �Ի�������
	enum { IDD = IDD_DLGSNDIRFILES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
	CSNDirFilesList m_ListCtrlST;
	DRVSNCFGPARA *m_pSnCfgPara;
	CComboBox m_cmbSNGroup;
	std::vector<CString>vHeader;
	BOOL InitCtrlList();

	CSNDirFilesCfg m_SnCfg;

	BOOL GetFileLength(CString strFile,UINT& Length);
	virtual BOOL OnInitDialog();
	
	CStdioFile m_logFile;

	INT OpenLog();
	void CloseLog();
	void WriteLog(CString str);
	
	INT CreatePassDir( const CString& DirName);
	BOOL  MoveFileToPass(const CString&File,CString& DestDir);


	CFileSearcher m_vFileSearcher[GROUPCNT_MAX];
	INT m_SNTotalSize;///���кŸ��ⲿ��SN�ܹ���Ҫ���ٸ��ֽ�
public:
	afx_msg void OnCbnSelchangeCombo1();
};
