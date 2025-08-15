// CSNCustomEspApp.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "SCS_SN_ESP_Mode.h"
#include "DlgSNCustom.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: ����� DLL ����� MFC DLL �Ƕ�̬���ӵģ�
//		��Ӵ� DLL �������κε���
//		MFC �ĺ������뽫 AFX_MANAGE_STATE ����ӵ�
//		�ú�������ǰ�档
//
//		����:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// �˴�Ϊ��ͨ������
//		}
//
//		�˺������κ� MFC ����
//		������ÿ��������ʮ����Ҫ������ζ��
//		��������Ϊ�����еĵ�һ�����
//		���֣������������ж������������
//		������Ϊ���ǵĹ��캯���������� MFC
//		DLL ���á�
//
//		�й�������ϸ��Ϣ��
//		����� MFC ����˵�� 33 �� 58��
//


// CSNCustomEspApp

BEGIN_MESSAGE_MAP(CSNCustomEspApp, CWinApp)
END_MESSAGE_MAP()


// CSNCustomEspApp ����

CSNCustomEspApp::CSNCustomEspApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CSNCustomEspApp ����

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


// CSNCustomEspApp ��ʼ��

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
 * @brief: ȷ���Ƿ���ʧ�ܵ�ʱ���ܹ�������Reuse����
 *
 * @param[in]: pSnCfgPara : DRVSNCFGPARA�ṹ��
 * @return  
 *		0��ʾ��Ҫ����Reuse���ˣ�1��ʾ���Է��룬-1��ʾ����
 */
INT SNCheckNeedTobeReuse(DRVSNCFGPARA*pSnCfgPara)
{
	return 0;
}

/*!
 * @brief: վ����SOCKET SN��Ӧ��ϵ
 *
 * @param[in]: pSnCfgPara : DRVSNCFGPARA�ṹ��
 * @param[in]: pSnCfgPara : DRVSNCFGPARA�ṹ��
 * @return: 0�ɹ�
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

