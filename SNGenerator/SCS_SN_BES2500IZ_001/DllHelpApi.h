#pragma once
#include "LogFile.h"

typedef int (cdecl  *FnProduct_GetSNorMAC)(char* WifiMAC, char* Product_Batch, char* Product_Type, char* pOutSN_Value);
typedef int (cdecl  *FnATS_CALL_IT_SQL_API)(char* pSQL_API, char* pOutResult_Str, int length_max);

typedef struct tagDllApi{
	FnProduct_GetSNorMAC pFnProduct_GetSNorMAC;
	FnATS_CALL_IT_SQL_API pFnATS_CALL_IT_SQL_API;
};

class CDllHelp
{
public:
	CDllHelp(void);
	~CDllHelp(void);
	
	void CopyDirectory(CString strSrc, CString strDest);
	BOOL AttachDll();
	BOOL DetachDll();
	int Product_GetSNorMAC(char* WifiMAC, char* Product_Batch, char* Product_Type, char* pOutSN_Value);
	int ATS_CALL_IT_SQL_API(char* pSQL_API,char* pOutResult_Str, int length_max);
	void AttachLog(CLogFile* prt){m_FileLog =prt; };
	
public:
	bool m_bLoadDll;
private:
	HINSTANCE m_hLib;
	tagDllApi m_tagDllApi;
	CLogFile* m_FileLog;
};
