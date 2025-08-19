// SNStep.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "SNStep.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif


#include "DlgSNStep.h"

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


// CSNStepApp

BEGIN_MESSAGE_MAP(CSNStepApp, CWinApp)
END_MESSAGE_MAP()

// CSNStepApp 构造

CSNStepApp::CSNStepApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CSNStepApp 对象

CSNStepApp theApp;


// CSNStepApp 初始化

BOOL CSNStepApp::InitInstance()
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
	CDlgSNStep *pDlgSNStep=new CDlgSNStep(pSnCfgPara);
	pSnCfgPara->DrvPrivData=dynamic_cast<void*>(pDlgSNStep);
	if(pDlgSNStep==NULL){
		Ret=FALSE;
		goto __end;
	}
	if(pSnCfgPara->pParent!=NULL){
		Ret=pDlgSNStep->Create(CDlgSNStep::IDD,pSnCfgPara->pParent);
		if(Ret){
			pDlgSNStep->GetWindowRect(&Rect);
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
	CDlgSNStep *pDlgSNStep=reinterpret_cast<CDlgSNStep*>(pSnCfgPara->DrvPrivData);
	if(pDlgSNStep){
		delete pDlgSNStep;
		pSnCfgPara->DrvPrivData=NULL;
	}
	return TRUE;
}

BOOL SNShowWindow(DRVSNCFGPARA *pSnCfgPara ,BOOL bShow)
{
	CDlgSNStep *pDlgSNStep=reinterpret_cast<CDlgSNStep*>(pSnCfgPara->DrvPrivData);
	if(pDlgSNStep && pDlgSNStep->GetSafeHwnd()){
		if(bShow)
			pDlgSNStep->ShowWindow(SW_SHOW);
		else
			pDlgSNStep->ShowWindow(SW_HIDE);
		return TRUE;
	}
	else{
		return FALSE;
	}
}

BOOL SNInitCtrlsValue(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSNStep *pDlgSNStep=reinterpret_cast<CDlgSNStep*>(pSnCfgPara->DrvPrivData);
	if(pDlgSNStep){
		CSerial lSerial;
		if(pSnCfgPara->pSNCFGInitBuf!=NULL&&pSnCfgPara->dwInitBufUse!=0){
			lSerial.SerialInBuff(pSnCfgPara->pSNCFGInitBuf,pSnCfgPara->dwInitBufUse);
		}
		return pDlgSNStep->InitCtrls(lSerial);
	}
	else{
		return FALSE;
	}
	return TRUE;
}


INT SNGetCtrlsValue(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSNStep *pDlgSNStep=reinterpret_cast<CDlgSNStep*>(pSnCfgPara->DrvPrivData);
	if(pDlgSNStep){
		CSerial lSerial;
		if(pDlgSNStep->GetCtrls(lSerial)==FALSE){
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
	CDlgSNStep *pDlgSNStep=reinterpret_cast<CDlgSNStep*>(pSnCfgPara->DrvPrivData);
	if(Idx==0){
		goto __end;
	}
	if(pDlgSNStep){
		return pDlgSNStep->QuerySN(Idx,pData,pSize);
	}

__end:
	return -1;
}