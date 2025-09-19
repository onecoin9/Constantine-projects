#pragma once
#include "afx.h"

#define MAC_LEN_BYTE_SIZE		(6)
#define CSV_COL_NUM				(6)
typedef struct tagParamCol
{
	CString strDevNameCol;
	CString strDevSecretCol;
	CString strProdKeyCol;
	CString strProdSecretCol;
	CString strPidCol;
	UCHAR MacCol[MAC_LEN_BYTE_SIZE];

}tParamCol;

class CCSVFile : public CStdioFile
{
public:
	CCSVFile(void);
	~CCSVFile(void);

	INT OpenFile(CString strFilePath);
	INT CloseFile();
	BOOL IsReady(){return m_IsReady;}	
	INT ReadSN(UINT Index, tParamCol &ParamCol, INT &nTotalLen);

	CString GetErrMsg(){return m_strErrMsg;}
protected:
	INT SeparatePara(CString&OneLine, CStringArray &strArrayCol);

private:
	CString m_strErrMsg;
	UINT m_EachLineSize;
	ULONGLONG m_PosStart;
	BOOL m_IsReady;
};
