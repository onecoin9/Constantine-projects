// SNFile.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "SNFile.h"
#include "DlgSNFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: 如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//


// CSNFileApp

BEGIN_MESSAGE_MAP(CSNFileApp, CWinApp)
END_MESSAGE_MAP()


// CSNFileApp 构造

CSNFileApp::CSNFileApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CSNFileApp 对象

CSNFileApp theApp;


// CSNFileApp 初始化

BOOL CSNFileApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}


BOOL SNGetCapability(DRVSNCFGPARA *pSnCfgPara)
{
	return TRUE;
}

BOOL SNCreateWindow(DRVSNCFGPARA *pSnCfgPara,RECT Position)
{
	BOOL Ret=TRUE;
	RECT Rect;
	HINSTANCE save_hInstance = AfxGetResourceHandle();
	AfxSetResourceHandle(theApp.m_hInstance);
	CDlgSNFile *pDlgSNFile=new CDlgSNFile(pSnCfgPara);
	pSnCfgPara->DrvPrivData=dynamic_cast<void*>(pDlgSNFile);
	if(pDlgSNFile==NULL){
		Ret=FALSE;
		goto __end;
	}
	if(pSnCfgPara->pParent!=NULL){
		Ret=pDlgSNFile->Create(CDlgSNFile::IDD,pSnCfgPara->pParent);
		if(Ret){
			pDlgSNFile->GetWindowRect(&Rect);
			pSnCfgPara->SNGenRect.left=0;
			pSnCfgPara->SNGenRect.top=0;
			pSnCfgPara->SNGenRect.right=Rect.right-Rect.left;
			pSnCfgPara->SNGenRect.bottom=Rect.bottom-Rect.top;
		}
	}
__end:
	AfxSetResourceHandle(save_hInstance);
	return Ret;
}

BOOL SNDestroyWindow(DRVSNCFGPARA*pSnCfgPara)
{
	CDlgSNFile *pDlgSNFile=reinterpret_cast<CDlgSNFile*>(pSnCfgPara->DrvPrivData);
	if(pDlgSNFile){
		delete pDlgSNFile;
		pSnCfgPara->DrvPrivData=NULL;
	}
	return TRUE;
}

BOOL SNShowWindow(DRVSNCFGPARA *pSnCfgPara ,BOOL bShow)
{
	CDlgSNFile *pDlgSNFile=reinterpret_cast<CDlgSNFile*>(pSnCfgPara->DrvPrivData);
	if(pDlgSNFile && pDlgSNFile->GetSafeHwnd()){
		if(bShow)
			pDlgSNFile->ShowWindow(SW_SHOW);
		else
			pDlgSNFile->ShowWindow(SW_HIDE);
		return TRUE;
	}
	else{
		return FALSE;
	}
}

BOOL SNInitCtrlsValue(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSNFile *pDlgSNFile=reinterpret_cast<CDlgSNFile*>(pSnCfgPara->DrvPrivData);
	if(pDlgSNFile){
		CSerial lSerial;
		if(pSnCfgPara->pSNCFGInitBuf!=NULL&&pSnCfgPara->dwInitBufUse!=0){
			lSerial.SerialInBuff(pSnCfgPara->pSNCFGInitBuf,pSnCfgPara->dwInitBufUse);
		}
		return pDlgSNFile->InitCtrls(lSerial);
	}
	else{
		return FALSE;
	}
	return TRUE;
}


BOOL SNInitCtrlsValueWithoutWnd(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSNFile *pDlgSNFile=reinterpret_cast<CDlgSNFile*>(pSnCfgPara->DrvPrivData);
	if(pDlgSNFile){
		CSerial lSerial;
		if(pSnCfgPara->pSNCFGInitBuf!=NULL&&pSnCfgPara->dwInitBufUse!=0){
			lSerial.SerialInBuff(pSnCfgPara->pSNCFGInitBuf,pSnCfgPara->dwInitBufUse);
		}
		return pDlgSNFile->InitCtrlsWthoutWnd(lSerial);
	}
	else{
		return FALSE;
	}
	return TRUE;
}



INT SNGetCtrlsValue(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSNFile *pDlgSNFile=reinterpret_cast<CDlgSNFile*>(pSnCfgPara->DrvPrivData);
	if(pDlgSNFile){
		CSerial lSerial;
		if(pDlgSNFile->GetCtrls(lSerial)==FALSE){
			return -1;
		}
		else{
			pSnCfgPara->dwInitBufUse=lSerial.GetLength();
			if(pSnCfgPara->pSNCFGInitBuf==NULL){
				return -3;
			}
			if(pSnCfgPara->dwInitBufSize<lSerial.GetLength()){
				return -2;
			}
			else{
				lSerial.SerialOutBuff(pSnCfgPara->pSNCFGInitBuf,lSerial.GetLength());
			}
		}
	}
	else{
		return -1;
	}
	return 0;
}

INT SNQuery(DRVSNCFGPARA*pSnCfgPara,DWORD Idx,BYTE*pData,INT*pSize)
{
	CDlgSNFile *pDlgSNFile=reinterpret_cast<CDlgSNFile*>(pSnCfgPara->DrvPrivData);
	if(Idx==0){
		goto __end;
	}
	if(pDlgSNFile){
		return pDlgSNFile->QuerySN(Idx,pData,pSize);
	}

__end:
	return -1;
}

INT SNGetObjData(DRVSNCFGPARA*pSnCfgPara,DWORD Idx,BYTE*pData,INT*pSize, int nType)
{
	CDlgSNFile *pDlgSNFile=reinterpret_cast<CDlgSNFile*>(pSnCfgPara->DrvPrivData);
	if(Idx==0){
		goto __end;
	}
	if(pDlgSNFile){
		if (nType == 0){
			return pDlgSNFile->QueryPrint(Idx,pData,pSize);
		}
	}

__end:
	return -1;
}

INT SNGetFeature(DRVSNCFGPARA*pSnCfgPara,const char*strJsonIn, char* strJsonOut, INT SNSize)
{
	CDlgSNFile *pDlgSNFile=reinterpret_cast<CDlgSNFile*>(pSnCfgPara->DrvPrivData);
	if(strJsonOut == NULL){
		goto __end;
	}
	if(pDlgSNFile){
		return pDlgSNFile->GetSNFeature(strJsonIn,strJsonOut,SNSize);
	}

__end:
	return -1;
}
