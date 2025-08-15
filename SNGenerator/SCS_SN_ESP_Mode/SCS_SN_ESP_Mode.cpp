// CSNCustomEspApp.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "SCS_SN_ESP_Mode.h"
#include "DlgSNCustom.h"

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


// CSNCustomEspApp

BEGIN_MESSAGE_MAP(CSNCustomEspApp, CWinApp)
END_MESSAGE_MAP()


// CSNCustomEspApp 构造

CSNCustomEspApp::CSNCustomEspApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CSNCustomEspApp 对象

CSNCustomEspApp theApp;

struct ResourceRaii {
	ResourceRaii() : ins_(AfxGetResourceHandle()) {
		AfxSetResourceHandle(theApp.m_hInstance);
	}

	~ResourceRaii() {
		AfxSetResourceHandle(ins_);
	}

private:
	HINSTANCE ins_;
};


// CSNCustomEspApp 初始化

BOOL CSNCustomEspApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}


BOOL SNCreateWindow(DRVSNCFGPARA *pSnCfgPara,RECT Position)
{
	BOOL Ret=TRUE;
	RECT Rect;
#ifdef _DEBUG
	HINSTANCE save_hInstance = AfxGetResourceHandle();
	AfxSetResourceHandle(theApp.m_hInstance);
#else
	HINSTANCE save_hInstance = AfxGetResourceHandle();
	AfxSetResourceHandle(theApp.m_hInstance);
#endif 
	
	CDlgSNCustom* pDlgSN = new CDlgSNCustom(pSnCfgPara);
	pSnCfgPara->DrvPrivData = reinterpret_cast<void*>(pDlgSN);
	if(pDlgSN == NULL){
		Ret = FALSE;
		goto __end;
	}
	if(pSnCfgPara->pParent!=NULL){
		Ret=pDlgSN->Create(CDlgSNCustom::IDD, pSnCfgPara->pParent);
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
	if(pSnCfgPara->DrvPrivData){
		CDlgSNCustom *pDlgSN = reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
		delete pDlgSN;
		pSnCfgPara->DrvPrivData = NULL;
	}
	return TRUE;
}


BOOL SNShowWindow(DRVSNCFGPARA *pSnCfgPara ,BOOL bShow)
{
	CDlgSNCustom* pDlgSN = reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
	if(!pDlgSN || !pDlgSN->GetSafeHwnd())
		return FALSE;

	if (bShow) {
		ResourceRaii res_raii;
		if (!pDlgSN->ShowWorkSheetDlg()) {
			AfxMessageBox("Enable Avent Special Rule, please fill work sheet", MB_OK | MB_ICONERROR);
			return FALSE;
		}
		pDlgSN->ShowWindow(SW_SHOW);
	}
	else pDlgSN->ShowWindow(SW_HIDE);
	return TRUE;
}


BOOL SNGetCapability(DRVSNCFGPARA *pSnCfgPara)
{
	return TRUE;
}


BOOL SNInitCtrlsValue(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSNCustom *pDlgSN = reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
	if(!pDlgSN)
		return FALSE;

	CSerial lSerial;
	if(pSnCfgPara->pSNCFGInitBuf != NULL && pSnCfgPara->dwInitBufUse != 0){
		lSerial.SerialInBuff(pSnCfgPara->pSNCFGInitBuf, pSnCfgPara->dwInitBufUse);
	}
	return pDlgSN->InitCtrls(lSerial);
}


INT SNGetCtrlsValue(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSNCustom *pDlgSN = reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
	if(!pDlgSN){
		return -1;
	}
	
	CSerial lSerial;
	if(pDlgSN->GetCtrls(lSerial) == FALSE){
		return -1;
	}

	pSnCfgPara->dwInitBufUse = lSerial.GetLength();
	if(pSnCfgPara->pSNCFGInitBuf == NULL){
		return -3;
	}
	if(pSnCfgPara->dwInitBufSize < lSerial.GetLength()){
		return -2;
	}

	lSerial.SerialOutBuff(pSnCfgPara->pSNCFGInitBuf, lSerial.GetLength());
	return 0;
}

INT SNQuery(DRVSNCFGPARA* pSnCfgPara, DWORD Idx, BYTE* pData, INT* pSize)
{
	CDlgSNCustom* pDlgSN = reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
	if(Idx == 0 || !pDlgSN || pSnCfgPara->pSiteSN[0] == 0 || pSnCfgPara->AdapIdx > 7)
		return -1;
	return pDlgSN->QuerySN(Idx, pData, pSize);
}

INT SNPreLoad(DRVSNCFGPARA*pSNCfgPara)
{
	CDlgSNCustom *pDlgSN = reinterpret_cast<CDlgSNCustom*>(pSNCfgPara->DrvPrivData);
	if(pDlgSN)
		return pDlgSN->PreLoad();
	return -1;
}

INT SNTellResult(DRVSNCFGPARA*pSnCfgPara,DWORD Idx,INT IsPass)
{
	CDlgSNCustom *pDlgSN = reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
	if(Idx == 0 || !pDlgSN)
		return -1;
	return pDlgSN->TellResult(Idx,IsPass);
}

/*!
 * @brief: 确认是否在失败的时候能够被放入Reuse队列
 *
 * @param[in]: pSnCfgPara : DRVSNCFGPARA结构体
 * @return  
 *		0表示不要放入Reuse对了，1表示可以放入，-1表示出错
 */
INT SNCheckNeedTobeReuse(DRVSNCFGPARA*pSnCfgPara)
{
	return 0;
}

/*!
 * @brief: 站点与SOCKET SN对应关系
 *
 * @param[in]: pSnCfgPara : DRVSNCFGPARA结构体
 * @param[in]: pSnCfgPara : DRVSNCFGPARA结构体
 * @return: 0成功
 *	
 */
INT SNSetProgramInfo(DRVSNCFGPARA* pSnCfgPara, const char* info_json)
{
	CDlgSNCustom *pDlgSN = reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
	if(!pDlgSN)
		return -1;
	return pDlgSN->SetProgramInfo(info_json);
}

void SNSetMessageDumpCallback(DRVSNCFGPARA* pSnCfgPara, MessageDumpCallback callback) {
	CDlgSNCustom *pDlgSN = reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
	if(!pDlgSN)
		return;
	pDlgSN->SetMessageDumpCallback(callback);
}

