#include "StdAfx.h"
#include "DllHelpApi.h"

#include "../Com/ComFunc.h"

HMODULE g_hGetPmrId = NULL;

typedef struct _TMsg
{
	BSTR str;
} TMsg, *PTMsg;

typedef void* (__stdcall* LPFNDLLInitLocalDB)(PTMsg lpResult);
typedef void* (__stdcall* LPFNDLLGetRmpIdList)(PTMsg lpResult, int iIdCount);


LPFNDLLInitLocalDB g_pfnInitLocalDB = NULL;
LPFNDLLGetRmpIdList g_pfnGetRmpIdList = NULL;

TMsg g_ResultMsg;

CDllHelp::CDllHelp(void)
{
	m_bLoadDll = false;
	m_hLib = NULL;
}

CDllHelp::~CDllHelp(void)
{
	m_bLoadDll = false;
	GetPmrIdUnInit();
}


void CDllHelp::CopyDirectory(CString strSrc, CString strDest)
{
	CFileFind finder;  
	CString path=strSrc+("\\*.*"); //��Ҫ�������ļ���·��

	BOOL bWorking = finder.FindFile(path);  //�Ƿ��ҵ�����Ҫ�������ļ���·��
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

BOOL CDllHelp::GetPmrIdInit()
{

	BOOL Ret = FALSE;
	CoInitialize(NULL);
	CString strDllPath;
	CString strTempPath=CComFunc::GetCurrentPath();

	CFileFind finder;
	CString path=strTempPath+("\\borlndmm.dll"); 
	BOOL bWorking = finder.FindFile(path); 

	//////copy���Ŀ��ļ���
	CString strSrc;
	CString strDest;
	strSrc.Format("%s\\sngen\\midea\\", strTempPath);
	strDest.Format("%s", strTempPath);
	
	if (!bWorking){
		CopyDirectory(strSrc, strDest);
	}

	strDllPath.Format("%s\\getPmrId.dll",strTempPath);
	g_hGetPmrId = LoadLibrary(strDllPath);

	//g_hGetPmrId = LoadLibrary("GetPmrId.dll");
	if (g_hGetPmrId)
	{
		g_pfnInitLocalDB = (LPFNDLLInitLocalDB)GetProcAddress(g_hGetPmrId, "InitLocalDB");
		g_pfnGetRmpIdList = (LPFNDLLGetRmpIdList)GetProcAddress(g_hGetPmrId, "GetRmpIdList");

		if (g_pfnInitLocalDB == NULL || g_pfnGetRmpIdList == NULL)
		{
			FreeLibrary(g_hGetPmrId);
			g_hGetPmrId = NULL;
			g_pfnInitLocalDB = NULL;
			g_pfnGetRmpIdList = NULL;
			goto __end;
		}
		m_bLoadDll = TRUE;
		Ret=TRUE; goto __end;
	}
	else
	{
		goto __end;
	}

__end:
	return Ret;
	
}

void CDllHelp::GetPmrIdUnInit()
{
	if (g_hGetPmrId)
	{
		FreeLibrary(g_hGetPmrId);
		g_hGetPmrId = NULL;
		g_pfnInitLocalDB = NULL;
		g_pfnGetRmpIdList = NULL;
	}
}

//��ʼ��DLL���ߴ���¼���
char * CDllHelp::InitLocalDB()
{
	if (g_pfnInitLocalDB)
	{
		(*g_pfnInitLocalDB)(&g_ResultMsg);

		return (char *)g_ResultMsg.str;
	}
	else
	{
		return NULL;
	}
}

//���ָ�����������ظ�ID
char * CDllHelp::GetRmpIdList(int iIdCount)
{
	if (g_pfnGetRmpIdList)
	{
		(*g_pfnGetRmpIdList)(&g_ResultMsg, iIdCount);
		return (char *)g_ResultMsg.str;
	}
	else
	{
		return NULL;
	}
}



