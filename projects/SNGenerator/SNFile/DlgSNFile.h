#pragma once

#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "../Com/Serial.h"
#include <vector>

// CDlgSNFile 对话框

#define DATAMODE_DATA	 (1)
#define DATAMODE_FILE	 (2)
#define DATAMODE_MIX (3)
#define BYTEMODE_BE		(1)
#define BYTEMODE_LE		(2)
typedef struct tagSNInfo
{
	INT SNDataMode;  //DATAMODE_DATA etc
	INT SNByteMode; ///BYTEMODE_BE etc
	UINT64 LineIndex;///记录当前在多少行
	UINT64 SNInfoStartLine;	///记录SNInfo在第几行
	UINT64 SNInfoStartPos;///记录SNInfo的起始位置
	INT RecordSize;	///一条记录的占用字节长度，也就是一行有多少个字节，后面用于索引很方便
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
	CDlgSNFile(DRVSNCFGPARA *pSNCfgPara,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSNFile();

// 对话框数据
	enum { IDD = IDD_DLGSNFILE };

	BOOL InitCtrls(CSerial& lSerial);
	BOOL InitCtrlsWthoutWnd(CSerial& lSerial);
	BOOL GetCtrls(CSerial&lSerial);
	INT QuerySN(DWORD Idx,BYTE*pData,INT*pSize);
	INT QueryPrint(DWORD Idx,BYTE*pData,INT*pSize);
	INT GetSNFeature(const char*strJsonIn, char* strJsonOut, INT SNSize);//

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

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
	tSNInfo m_SNInfo; ///记录SNInfo
	CString m_strErrMsg;
	std::vector<tSNInfo> m_vRecordIdxMap;
};
