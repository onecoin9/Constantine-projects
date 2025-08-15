// SCS_SN_YCHUANG.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "SCS_SN_YCHUANG.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif


#include "DlgSCS_SN_YCHUANG.h"

//
//TODO: 如果此 DLL 动态链接到 MFC DLL，并且从此 DLL 导出的任何
//		函数调用 MFC，请在此函数开始处添加 AFX_MANAGE_STATE 宏。
//		例如:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中。这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//


// CSCS_SN_YCHUANGApp

BEGIN_MESSAGE_MAP(CSCS_SN_YCHUANGApp, CWinApp)
END_MESSAGE_MAP()

// CSCS_SN_YCHUANGApp 构造

CSCS_SN_YCHUANGApp::CSCS_SN_YCHUANGApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CSCS_SN_YCHUANGApp 对象

CSCS_SN_YCHUANGApp theApp;


// CSCS_SN_YCHUANGApp 初始化

BOOL CSCS_SN_YCHUANGApp::InitInstance()
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
	CDlgSCS_SN_YCHUANG *pDlgSCS_SN_YCHUANG=new CDlgSCS_SN_YCHUANG(pSnCfgPara);
	pSnCfgPara->DrvPrivData=dynamic_cast<void*>(pDlgSCS_SN_YCHUANG);
	if(pDlgSCS_SN_YCHUANG==NULL){
		Ret=FALSE;
		goto __end;
	}
	if(pSnCfgPara->pParent!=NULL){
		Ret=pDlgSCS_SN_YCHUANG->Create(CDlgSCS_SN_YCHUANG::IDD,pSnCfgPara->pParent);
		if(Ret){
			pDlgSCS_SN_YCHUANG->GetWindowRect(&Rect);
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

BOOL SNDestroyWindow(DRVSNCFGPARA *pSnCfgPara)
{
	if(pSnCfgPara->DrvPrivData!=NULL){
		CDlgSCS_SN_YCHUANG *pDlgSCS_SN_YCHUANG=dynamic_cast<CDlgSCS_SN_YCHUANG*>(pSnCfgPara->DrvPrivData);
		if(pDlgSCS_SN_YCHUANG!=NULL){
			pDlgSCS_SN_YCHUANG->DestroyWindow();
			delete pDlgSCS_SN_YCHUANG;
			pSnCfgPara->DrvPrivData=NULL;
		}
	}
	return TRUE;
}

BOOL SNShowWindow(DRVSNCFGPARA *pSnCfgPara,BOOL bShow)
{
	if(pSnCfgPara->DrvPrivData!=NULL){
		CDlgSCS_SN_YCHUANG *pDlgSCS_SN_YCHUANG=dynamic_cast<CDlgSCS_SN_YCHUANG*>(pSnCfgPara->DrvPrivData);
		if(pDlgSCS_SN_YCHUANG!=NULL){
			pDlgSCS_SN_YCHUANG->ShowWindow(bShow?SW_SHOW:SW_HIDE);
		}
	}
	return TRUE;
}

BOOL SNInitCtrlsValue(DRVSNCFGPARA *pSnCfgPara)
{
	if(pSnCfgPara->DrvPrivData!=NULL){
		CDlgSCS_SN_YCHUANG *pDlgSCS_SN_YCHUANG=dynamic_cast<CDlgSCS_SN_YCHUANG*>(pSnCfgPara->DrvPrivData);
		if(pDlgSCS_SN_YCHUANG!=NULL){
			return pDlgSCS_SN_YCHUANG->InitCtrlsValue();
		}
	}
	return FALSE;
}

BOOL SNInitCtrlsValueWithoutWnd(DRVSNCFGPARA *pSnCfgPara)
{
	if(pSnCfgPara->DrvPrivData!=NULL){
		CDlgSCS_SN_YCHUANG *pDlgSCS_SN_YCHUANG=dynamic_cast<CDlgSCS_SN_YCHUANG*>(pSnCfgPara->DrvPrivData);
		if(pDlgSCS_SN_YCHUANG!=NULL){
			return pDlgSCS_SN_YCHUANG->InitCtrlsValueWithoutWnd();
		}
	}
	return FALSE;
}

BOOL SNGetCtrlsValue(DRVSNCFGPARA *pSnCfgPara)
{
	if(pSnCfgPara->DrvPrivData!=NULL){
		CDlgSCS_SN_YCHUANG *pDlgSCS_SN_YCHUANG=dynamic_cast<CDlgSCS_SN_YCHUANG*>(pSnCfgPara->DrvPrivData);
		if(pDlgSCS_SN_YCHUANG!=NULL){
			return pDlgSCS_SN_YCHUANG->GetCtrlsValue();
		}
	}
	return FALSE;
}

BOOL SNQuery(DRVSNCFGPARA *pSnCfgPara,DWORD Idx,BYTE*pData,INT*pSize)
{
	if(pSnCfgPara->DrvPrivData!=NULL){
		CDlgSCS_SN_YCHUANG *pDlgSCS_SN_YCHUANG=dynamic_cast<CDlgSCS_SN_YCHUANG*>(pSnCfgPara->DrvPrivData);
		if(pDlgSCS_SN_YCHUANG!=NULL){
			return pDlgSCS_SN_YCHUANG->QuerySN(Idx,pData,pSize);
		}
	}
	return FALSE;
}
