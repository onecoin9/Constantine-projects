#pragma once

#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "../Com/Serial.h"
#include <vector>

// CDlgSNFile �Ի���

#define DATAMODE_DATA	 (1)
#define DATAMODE_FILE	 (2)
#define DATAMODE_MIX (3)
#define BYTEMODE_BE		(1)
#define BYTEMODE_LE		(2)
typedef struct tagSNInfo
{
	INT SNDataMode;  //DATAMODE_DATA etc
	INT SNByteMode; ///BYTEMODE_BE etc
	UINT64 LineIndex;///��¼��ǰ�ڶ�����
	UINT64 SNInfoStartLine;	///��¼SNInfo�ڵڼ���
	UINT64 SNInfoStartPos;///��¼SNInfo����ʼλ��
	INT RecordSize;	///һ����¼��ռ���ֽڳ��ȣ�Ҳ����һ���ж��ٸ��ֽڣ��������������ܷ���
	CString strCurRecord;

	struct tagSNInfo(){
		ReInit();
	}
	void ReInit(){
		SNDataMode = 0;
		SNByteMode = 0;
		LineIndex =0;
		SNInfoStartLine = 0;
		SNInfoStartPos = 0;
		RecordSize = 0;
		strCurRecord.Empty();
	}	
}tSNInfo;

typedef struct tagSNOneGroup{
	UINT64 SNAddr;
	INT SNSize;
	BYTE *pData;
}tSNOneGroup;



class CDlgSNFile : public CDialog
{
	DECLARE_DYNAMIC(CDlgSNFile)

public:
	CDlgSNFile(DRVSNCFGPARA *pSNCfgPara,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgSNFile();

// �Ի�������
	enum { IDD = IDD_DLGSNFILE };

	BOOL InitCtrls(CSerial& lSerial);
	BOOL InitCtrlsWthoutWnd(CSerial& lSerial);
	BOOL GetCtrls(CSerial&lSerial);
	INT QuerySN(DWORD Idx,BYTE*pData,INT*pSize);
	INT QueryPrint(DWORD Idx,BYTE*pData,INT*pSize);
	INT GetSNFeature(const char*strJsonIn, char* strJsonOut, INT SNSize);//

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

	DRVSNCFGPARA *m_pSNCfgPara;

	INT GetSNSerChipInfo();
	INT PrepareSNInfo();
	INT OpenSNSer();
	INT CloseSNSer();
	INT BuildIdxMap();
	INT GetPrintContent(CString strCurRow, CString& strPrint);
	INT GetCurRowFormat(CString strSrc);
	INT GetDataFromFile(CString strFileName, BYTE** pSN, int nExpectSize);

	INT FreeSNGroups(std::vector<tSNOneGroup> &vSNGroups);
	int GetSNValue(BYTE**pp_snv,int*len,BYTE*buf,int* offset);
	INT GetSNGroups(UINT& SNIdx,std::vector<tSNOneGroup> &vSNGroups,BYTE* buf,INT len, CString& strPrint);

public:
	afx_msg void OnBnClickedBtnsnfilesel();

private:
	CString m_strFile;
	CString m_ChipName;
	CString m_ByteMode;
	CString m_DataMode;
	CStdioFile m_SNFile;
	tSNInfo m_SNInfo; ///��¼SNInfo
	CString m_strErrMsg;
	std::vector<tSNInfo> m_vRecordIdxMap;
};
