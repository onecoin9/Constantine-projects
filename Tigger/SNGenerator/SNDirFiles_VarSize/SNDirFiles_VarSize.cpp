// SNDirFiles_VarSize.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "SNDirFiles_VarSize.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

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

#include "DlgSNDirFiles_VarSize.h"
// CSNDirFiles_VarSizeApp

BEGIN_MESSAGE_MAP(CSNDirFiles_VarSizeApp, CWinApp)
END_MESSAGE_MAP()


// CSNDirFiles_VarSizeApp 构造

CSNDirFiles_VarSizeApp::CSNDirFiles_VarSizeApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CSNDirFiles_VarSizeApp 对象

CSNDirFiles_VarSizeApp theApp;


// CSNDirFiles_VarSizeApp 初始化

BOOL CSNDirFiles_VarSizeApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}


BOOL SNCreateWindow(DRVSNCFGPARA *pSnCfgPara,RECT Position)
{
	BOOL Ret=TRUE;
	RECT Rect;
	///When Under Debug Need to Mask Next Line
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HINSTANCE save_hInstance = AfxGetResourceHandle();
	AfxSetResourceHandle(theApp.m_hInstance);
	CDlgSNDirFiles_VarSize *pDlgSN=new CDlgSNDirFiles_VarSize(pSnCfgPara);
	pSnCfgPara->DrvPrivData=dynamic_cast<void*>(pDlgSN);
	if(pDlgSN==NULL){
		Ret=FALSE;
		goto __end;
	}
	if(pSnCfgPara->pParent!=NULL){
		Ret=pDlgSN->Create(CDlgSNDirFiles_VarSize::IDD,pSnCfgPara->pParent);
		if(Ret){
			pDlgSN->GetWindowRect(&Rect);
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
	CDlgSNDirFiles_VarSize *pDlgSN=reinterpret_cast<CDlgSNDirFiles_VarSize*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN){
		delete pDlgSN;
		pSnCfgPara->DrvPrivData=NULL;
	}
	return TRUE;
}


BOOL SNShowWindow(DRVSNCFGPARA *pSnCfgPara ,BOOL bShow)
{
	CDlgSNDirFiles_VarSize *pDlgSN=reinterpret_cast<CDlgSNDirFiles_VarSize*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN && pDlgSN->GetSafeHwnd()){
		if(bShow)
			pDlgSN->ShowWindow(SW_SHOW);
		else
			pDlgSN->ShowWindow(SW_HIDE);
		return TRUE;
	}
	else{
		return FALSE;
	}
}


BOOL SNGetCapability(DRVSNCFGPARA *pSnCfgPara)
{
	return TRUE;
}


BOOL SNInitCtrlsValue(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSNDirFiles_VarSize *pDlgSN=reinterpret_cast<CDlgSNDirFiles_VarSize*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN){
		CSerial lSerial;
		if(pSnCfgPara->pSNCFGInitBuf!=NULL&&pSnCfgPara->dwInitBufUse!=0){
			lSerial.SerialInBuff(pSnCfgPara->pSNCFGInitBuf,pSnCfgPara->dwInitBufUse);
		}
		return pDlgSN->InitCtrls(lSerial);
	}
	else{
		return FALSE;
	}
	return TRUE;
}

BOOL SNInitCtrlsValueWithoutWnd(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSNDirFiles_VarSize *pDlgSN=reinterpret_cast<CDlgSNDirFiles_VarSize*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN){
		CSerial lSerial;
		if(pSnCfgPara->pSNCFGInitBuf!=NULL&&pSnCfgPara->dwInitBufUse!=0){
			lSerial.SerialInBuff(pSnCfgPara->pSNCFGInitBuf,pSnCfgPara->dwInitBufUse);
		}
		return pDlgSN->InitCtrls(lSerial);
	}
	else{
		return FALSE;
	}
	return TRUE;
}


INT SNGetCtrlsValue(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSNDirFiles_VarSize *pDlgSN=reinterpret_cast<CDlgSNDirFiles_VarSize*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN){
		CSerial lSerial;
		if(pDlgSN->GetCtrls(lSerial)==FALSE){
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
	CDlgSNDirFiles_VarSize *pDlgSN=reinterpret_cast<CDlgSNDirFiles_VarSize*>(pSnCfgPara->DrvPrivData);
	if(Idx==0){
		goto __end;
	}
	if(pDlgSN){
		return pDlgSN->QuerySN(Idx,pData,pSize);
	}

__end:
	return -1;
}

INT SNPreLoad(DRVSNCFGPARA*pSNCfgPara)
{
	CDlgSNDirFiles_VarSize *pDlgSN=reinterpret_cast<CDlgSNDirFiles_VarSize*>(pSNCfgPara->DrvPrivData);
	if(pDlgSN){
		return pDlgSN->PreLoad();
	}

__end:
	return -1;
}

INT SNTellResult(DRVSNCFGPARA*pSnCfgPara,DWORD Idx,INT IsPass)
{
	CDlgSNDirFiles_VarSize *pDlgSN=reinterpret_cast<CDlgSNDirFiles_VarSize*>(pSnCfgPara->DrvPrivData);
	if(Idx==0){
		goto __end;
	}
	if(pDlgSN){
		return pDlgSN->TellResult(Idx,IsPass);

	}

__end:
	return -1;
}

INT SNGetFeature(DRVSNCFGPARA*pSnCfgPara,const char*strJsonIn, char* strJsonOut, INT SNSize)
{
	CDlgSNDirFiles_VarSize *pDlgSNFile=reinterpret_cast<CDlgSNDirFiles_VarSize*>(pSnCfgPara->DrvPrivData);
	if(strJsonOut == NULL){
		goto __end;
	}
	if(pDlgSNFile){
		return pDlgSNFile->GetSNFeature(strJsonIn,strJsonOut,SNSize);
	}

__end:
	return -1;
}