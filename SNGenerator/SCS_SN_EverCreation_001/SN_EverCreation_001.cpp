// SN_EverCreation_001.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "SN_EverCreation_001.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif


#include "DlgSN_EverCreation_001.h"

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


// CSN_EverCreation_001App

BEGIN_MESSAGE_MAP(CSN_EverCreation_001App, CWinApp)
END_MESSAGE_MAP()

// CSN_EverCreation_001App ����

CSN_EverCreation_001App::CSN_EverCreation_001App()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CSN_EverCreation_001App ����

CSN_EverCreation_001App theApp;


// CSN_EverCreation_001App ��ʼ��

BOOL CSN_EverCreation_001App::InitInstance()
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
	CDlgSN_EverCreation_001 *pDlgSN_EverCreation_001=new CDlgSN_EverCreation_001(pSnCfgPara);
	pSnCfgPara->DrvPrivData=dynamic_cast<void*>(pDlgSN_EverCreation_001);
	if(pDlgSN_EverCreation_001==NULL){
		Ret=FALSE;
		goto __end;
	}
	if(pSnCfgPara->pParent!=NULL){
		Ret=pDlgSN_EverCreation_001->Create(CDlgSN_EverCreation_001::IDD,pSnCfgPara->pParent);
		if(Ret){
			pDlgSN_EverCreation_001->GetWindowRect(&Rect);
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
	CDlgSN_EverCreation_001 *pDlgSN_EverCreation_001=reinterpret_cast<CDlgSN_EverCreation_001*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN_EverCreation_001){
		delete pDlgSN_EverCreation_001;
		pSnCfgPara->DrvPrivData=NULL;
	}
	return TRUE;
}

BOOL SNShowWindow(DRVSNCFGPARA *pSnCfgPara ,BOOL bShow)
{
	CDlgSN_EverCreation_001 *pDlgSN_EverCreation_001=reinterpret_cast<CDlgSN_EverCreation_001*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN_EverCreation_001 && pDlgSN_EverCreation_001->GetSafeHwnd()){
		if(bShow)
			pDlgSN_EverCreation_001->ShowWindow(SW_SHOW);
		else
			pDlgSN_EverCreation_001->ShowWindow(SW_HIDE);
		return TRUE;
	}
	else{
		return FALSE;
	}
}

BOOL SNInitCtrlsValue(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSN_EverCreation_001 *pDlgSN_EverCreation_001=reinterpret_cast<CDlgSN_EverCreation_001*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN_EverCreation_001){
		CSerial lSerial;
		if(pSnCfgPara->pSNCFGInitBuf!=NULL&&pSnCfgPara->dwInitBufUse!=0){
			lSerial.SerialInBuff(pSnCfgPara->pSNCFGInitBuf,pSnCfgPara->dwInitBufUse);
		}
		return pDlgSN_EverCreation_001->InitCtrls(lSerial);
	}
	else{
		return FALSE;
	}
	return TRUE;
}

BOOL SNInitCtrlsValueWithoutWnd(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSN_EverCreation_001 *pDlgSN_EverCreation_001=reinterpret_cast<CDlgSN_EverCreation_001*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN_EverCreation_001){
		CSerial lSerial;
		if(pSnCfgPara->pSNCFGInitBuf!=NULL&&pSnCfgPara->dwInitBufUse!=0){
			lSerial.SerialInBuff(pSnCfgPara->pSNCFGInitBuf,pSnCfgPara->dwInitBufUse);
		}
		return pDlgSN_EverCreation_001->InitCtrls(lSerial);
	}
	else{
		return FALSE;
	}
	return TRUE;
}

INT SNGetCtrlsValue(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSN_EverCreation_001 *pDlgSN_EverCreation_001=reinterpret_cast<CDlgSN_EverCreation_001*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN_EverCreation_001){
		CSerial lSerial;
		if(pDlgSN_EverCreation_001->GetCtrls(lSerial)==FALSE){
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
	CDlgSN_EverCreation_001 *pDlgSN_EverCreation_001=reinterpret_cast<CDlgSN_EverCreation_001*>(pSnCfgPara->DrvPrivData);
	if(Idx==0){
		goto __end;
	}
	if(pDlgSN_EverCreation_001){
		return pDlgSN_EverCreation_001->QuerySN(Idx,pData,pSize);
	}

__end:
	return -1;
}