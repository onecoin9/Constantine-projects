#include "StdAfx.h"
#include "DllHelp.h"

#include "../Com/ComFunc.h"

CDllHelp::CDllHelp(void)
{
	m_bLoadDll = false;
}

CDllHelp::~CDllHelp(void)
{
	m_bLoadDll = false;
}

BOOL CDllHelp::AttachDll()
{
	char path[512] ={0};
	char msg[512] ={0};
	int len = 502;
	CString strPath;
	CString strReg;
	int tempSize = 1024;
	UCHAR* reg = new UCHAR[tempSize];
	memset(reg, 0, tempSize);
	BOOL Ret = FALSE;
	CString strCurPath=CComFunc::GetCurrentPath();
	strPath.Format("%s", strCurPath);
	strPath +="\\lib2\\Device_TH.ini";
	memcpy(path, strPath, strPath.GetLength());
	/*path = (LPTSTR)(LPCTSTR)strPath;*/
	strCurPath += "\\lib2\\Tuple_TuoHeng.dll";
	m_hLib = LoadLibrary(strCurPath); 
	if (m_hLib == NULL) {
		DWORD ErrNo = GetLastError();
		//m_pILog->PrintLog(LOGLEVEL_ERR, "打开%s出错，错误码:%d\r\n", strDllPath, ErrNo);
		goto __end;
	}
	
	m_tagDll.pFnRegisterID=(FnRegisterID)GetProcAddress(m_hLib, "RegisterID");
	if(m_tagDll.pFnRegisterID ==NULL){
		goto __end;
	}

	m_tagDll.pFnGet_Firmware=(FnGet_Firmware)GetProcAddress(m_hLib, "Get_Firmware");
	if(m_tagDll.pFnGet_Firmware==NULL){
		goto __end;
	}

	m_tagDll.pFnGetTuple_Zigbee =(FnGetTuple_Zigbee)GetProcAddress(m_hLib, "GetTuple_Zigbee");
	if (m_tagDll.pFnGetTuple_Zigbee == NULL){
		goto __end;
	}

	m_tagDll.pFnGetEncrypTuple =(FnGetEncrypTuple)GetProcAddress(m_hLib, "GetEncrypTuple");
	if (m_tagDll.pFnGetEncrypTuple == NULL){
		goto __end;
	}

	m_tagDll.pGetTuple_BLE =(FnGetTuple_BLE)GetProcAddress(m_hLib, "GetTuple_BLE");
	if (m_tagDll.pGetTuple_BLE == NULL){
		goto __end;
	}

	m_tagDll.pGetPlatformGetEncrypTuple_BLE = (FnGetPlatformGetEncrypTuple_BLE)GetProcAddress(m_hLib, "PlatformGetEncrypTuple_BLE");
	if (m_tagDll.pGetPlatformGetEncrypTuple_BLE == NULL){
		goto __end;
	}

	RegisterID(path, reg, msg, 512);

	if (*reg == 0x01){
		m_bLoadDll = true;
	}

	Ret = TRUE;

__end:

	if (reg){
		delete [] reg;
		reg = NULL;
	}

	if (Ret == FALSE){
		CString strTip;
		strTip.Format("%s", "Tuple_TuoHeng.dll初始化失败");
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


void CDllHelp::RegisterID(char path[], unsigned char* reg, char msg[], int len){
	if (m_tagDll.pFnRegisterID){
		m_tagDll.pFnRegisterID(path, reg, msg, len);
	}
}

void CDllHelp::Get_Firmware(char out_sourcing[], char path[], char msg[], char url[], unsigned char* reg, char filename[], int len, int len2, int len3)
{
	if (m_tagDll.pFnGet_Firmware){
		m_tagDll.pFnGet_Firmware(out_sourcing, path, msg, url, reg, filename, len, len2, len3);
	}
}

void CDllHelp::GetTuple_Zigbee(char path[], unsigned char* reg, char TupleOut[], int len)
{
	if (m_tagDll.pFnGetTuple_Zigbee){
		m_tagDll.pFnGetTuple_Zigbee(path, reg, TupleOut, len);
	}
}

void CDllHelp::GetEncrypTuple(char product_model[], char path[], unsigned char *reg, char Message[], char TupleOut[], int len, int len2)
{
	if (m_tagDll.pFnGetEncrypTuple){
		m_tagDll.pFnGetEncrypTuple(product_model, path, reg, Message, TupleOut, len, len2);
	}
}

void CDllHelp::GetTuple_BLE(char path[], unsigned char* reg, char bleData[], int len)
{
	if (m_tagDll.pGetTuple_BLE){
		m_tagDll.pGetTuple_BLE(path, reg, bleData, len);
	}
}

void CDllHelp::GetPlatformGetEncrypTuple_BLE(char product_model[], char path[], unsigned char *reg, char Message[], char TupleOut[], int len, int len2)
{
	if (m_tagDll.pGetPlatformGetEncrypTuple_BLE){
		m_tagDll.pGetPlatformGetEncrypTuple_BLE(product_model, path, reg, Message, TupleOut, len, len2);
	}
}

