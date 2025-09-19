#pragma once
#include "LogFile.h"

class CDllHelp
{
public:
	CDllHelp(void);
	~CDllHelp(void);
	
	void AttachLog(CLogFile* prt){m_FileLog =prt; };
	
public:
	bool m_bLoadDll;
private:
	HINSTANCE m_hLib;
	CLogFile* m_FileLog;

public:
	BOOL GetPmrIdInit();
	void GetPmrIdUnInit();
	char * GetRmpIdList(int iIdCount);
	char * InitLocalDB();
	void CopyDirectory(CString strSrc, CString strDest);


};
