#pragma once

#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "../Com/Serial.h"
#include <vector>

#include "SNCustomCfg.h"
#include "CSVFile.h"
// CDlgID206 �Ի���

//#define DATAMODE_DATA	 (1)
//#define DATAMODE_FILE	 (2)
//#define BYTEMODE_BE		(1)
//#define BYTEMODE_LE		(2)
//typedef struct tagSNInfo
//{
//	INT SNDataMode;  //DATAMODE_DATA etc
//	INT SNByteMode; ///BYTEMODE_BE etc
//	UINT64 LineIndex;///��¼��ǰ�ڶ�����
//	UINT64 SNInfoStartLine;	///��¼SNInfo�ڵڼ���
//	UINT64 SNInfoStartPos;///��¼SNInfo����ʼλ��
//	INT RecordSize;	///һ����¼��ռ���ֽڳ��ȣ�Ҳ����һ���ж��ٸ��ֽڣ��������������ܷ���
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
	CDlgID206(DRVSNCFGPARA *pSNCfgPara,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgID206();

// �Ի�������
	enum { IDD = IDD_DLGID206 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
