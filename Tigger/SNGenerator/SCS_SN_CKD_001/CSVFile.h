#pragma once
#include "afx.h"

class CCSVFile : public CStdioFile
{
public:
	CCSVFile(void);
	~CCSVFile(void);

	INT OpenFile(CString strFilePath);
	INT CloseFile();
	BOOL IsReady(){return m_IsReady;}	
	INT ReadSN(UINT Index,CString&strFstCol,CString&strSecCol);

	CString GetErrMsg(){return m_strErrMsg;}
protected:
	INT SeparatePara(CString&OneLine,CString&strFstCol,CString&strSecCol);

private:
	CString m_strErrMsg;
	UINT m_EachLineSize;
	ULONGLONG m_PosStart;
	BOOL m_IsReady;
};
