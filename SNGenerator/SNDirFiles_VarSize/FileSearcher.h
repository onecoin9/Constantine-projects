#pragma once

#include <vector>

class CFileSearcher
{
public:
	CFileSearcher(void);
	~CFileSearcher(void);
	BOOL ScanFiles(CString& dir,UINT Size,CString strExtName,BOOL Var);
	void Sort();
	INT GetListSize();
	CString GetIdx(INT idx);

private:
	std::vector<CString> v_Files;
};
