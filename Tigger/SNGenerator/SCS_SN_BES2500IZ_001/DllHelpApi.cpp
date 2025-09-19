#include "StdAfx.h"
#include "DllHelpApi.h"

#include "../Com/ComFunc.h"

CDllHelp::CDllHelp(void)
{
	m_bLoadDll = false;
	m_hLib = NULL;
}

CDllHelp::~CDllHelp(void)
{
	m_bLoadDll = false;
	DetachDll();
}

void CDllHelp::CopyDirectory(CString strSrc, CString strDest)
{
	CFileFind finder;  
	CString path=strSrc+("\\*.*"); //需要拷贝的文件的路径
	
	bool bWorking = finder.FindFile(path);  //是否找到了需要拷贝的文件的路径
	while(bWorking){  
		bWorking = finder.FindNextFile();  
	
		if(finder.IsDirectory() && !finder.IsDots()){ //是不是有效的文件夹
			CopyDirectory(finder.GetFilePath(), strDest+("\\")+finder.GetFileName()); //递归查找文件夹
		}  
		else{ //是文件则直接复制 
			CopyFile(finder.GetFilePath(),strDest+("\\")+finder.GetFileName(),FALSE);  //拷贝文件夹下的所有文件
		}  
	}  

}

BOOL CDllHelp::AttachDll()
{
	BOOL Ret = FALSE;
	CString strCurPath=CComFunc::GetCurrentPath();
	strCurPath += "\\lib\\BES\\ATS_Station_Managment.dll";
	//strCurPath.Format("D:\\ACCode\\MultiAprog\\trunk\\ACStdMes\\ACStdMes\\Build\\ATS_Station_Managment.dll");
	
	
	CString strSrc;
	CString strDest;

	CString strTempPath=CComFunc::GetCurrentPath();
	strSrc.Format("%s\\lib\\BES\\", strTempPath);
	strDest.Format("%s", strTempPath);
	CopyDirectory(strSrc, strDest);
	strDest += "\\ATS_Station_Managment.dll";

	//m_FileLog->PrintLog(0, "attach dll=%s\r\n", strDest);

	m_hLib = LoadLibrary(strDest); 
	if (m_hLib == NULL) {
		DWORD ErrNo = GetLastError();
		//m_FileLog->PrintLog(0, "LoadLibrary%s出错，错误码:%d\r\n", strDest, ErrNo);
		//m_pILog->PrintLog(LOGLEVEL_ERR, "打开%s出错，错误码:%d\r\n", strDllPath, ErrNo);
		goto __end;
	}
	
	m_tagDllApi.pFnProduct_GetSNorMAC =(FnProduct_GetSNorMAC)GetProcAddress(m_hLib, "Product_GetSNorMAC");
	if(m_tagDllApi.pFnProduct_GetSNorMAC ==NULL){
		//m_FileLog->PrintLog(0, "Product_GetSNorMAC 导出失败\r\n");
		goto __end;
	}

	m_tagDllApi.pFnATS_CALL_IT_SQL_API=(FnATS_CALL_IT_SQL_API)GetProcAddress(m_hLib, "ATS_CALL_IT_SQL_API");
	if(m_tagDllApi.pFnATS_CALL_IT_SQL_API==NULL){
		//m_FileLog->PrintLog(0, "ATS_CALL_IT_SQL_API 导出失败\r\n");
		goto __end;
	}
	
	m_bLoadDll = true;
	Ret = TRUE;

__end:

	if (Ret == FALSE){
		CString strTip;
		strTip.Format("%s", "ATS_Station_Managment.dll初始化失败，请联系供应商！");
		AfxMessageBox(strTip);
	}
	return Ret;
}

BOOL CDllHelp::DetachDll()
{
	if (m_hLib != NULL) {
		FreeLibrary(m_hLib);
		m_hLib = NULL;
	}
	return TRUE;
}


int CDllHelp::Product_GetSNorMAC(char* WifiMAC, char* Product_Batch, char* Product_Type, char* pOutSN_Value)
{
	int nRet = -1;
	if (m_tagDllApi.pFnProduct_GetSNorMAC){
		nRet = m_tagDllApi.pFnProduct_GetSNorMAC(WifiMAC, Product_Batch, Product_Type, pOutSN_Value);
		m_FileLog->PrintLog(0, "Product_GetSNorMAC ret =%d\r\n", nRet);
	}
	return nRet;
}
int CDllHelp::ATS_CALL_IT_SQL_API(char* pSQL_API, char* pOutResult_Str, int length_max)
{
	int nRet = 0;
	if (m_tagDllApi.pFnATS_CALL_IT_SQL_API){
		nRet = m_tagDllApi.pFnATS_CALL_IT_SQL_API(pSQL_API, pOutResult_Str, length_max);
		m_FileLog->PrintLog(0, "ATS_CALL_IT_SQL_API ret =%d\r\n", nRet);
	}
	return nRet;
}

