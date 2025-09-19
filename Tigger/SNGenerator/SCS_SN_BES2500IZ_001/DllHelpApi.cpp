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
	CString path=strSrc+("\\*.*"); //��Ҫ�������ļ���·��
	
	bool bWorking = finder.FindFile(path);  //�Ƿ��ҵ�����Ҫ�������ļ���·��
	while(bWorking){  
		bWorking = finder.FindNextFile();  
	
		if(finder.IsDirectory() && !finder.IsDots()){ //�ǲ�����Ч���ļ���
			CopyDirectory(finder.GetFilePath(), strDest+("\\")+finder.GetFileName()); //�ݹ�����ļ���
		}  
		else{ //���ļ���ֱ�Ӹ��� 
			CopyFile(finder.GetFilePath(),strDest+("\\")+finder.GetFileName(),FALSE);  //�����ļ����µ������ļ�
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
		//m_FileLog->PrintLog(0, "LoadLibrary%s����������:%d\r\n", strDest, ErrNo);
		//m_pILog->PrintLog(LOGLEVEL_ERR, "��%s����������:%d\r\n", strDllPath, ErrNo);
		goto __end;
	}
	
	m_tagDllApi.pFnProduct_GetSNorMAC =(FnProduct_GetSNorMAC)GetProcAddress(m_hLib, "Product_GetSNorMAC");
	if(m_tagDllApi.pFnProduct_GetSNorMAC ==NULL){
		//m_FileLog->PrintLog(0, "Product_GetSNorMAC ����ʧ��\r\n");
		goto __end;
	}

	m_tagDllApi.pFnATS_CALL_IT_SQL_API=(FnATS_CALL_IT_SQL_API)GetProcAddress(m_hLib, "ATS_CALL_IT_SQL_API");
	if(m_tagDllApi.pFnATS_CALL_IT_SQL_API==NULL){
		//m_FileLog->PrintLog(0, "ATS_CALL_IT_SQL_API ����ʧ��\r\n");
		goto __end;
	}
	
	m_bLoadDll = true;
	Ret = TRUE;

__end:

	if (Ret == FALSE){
		CString strTip;
		strTip.Format("%s", "ATS_Station_Managment.dll��ʼ��ʧ�ܣ�����ϵ��Ӧ�̣�");
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

