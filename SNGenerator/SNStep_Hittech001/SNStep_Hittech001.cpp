// SNStep_Hittech001.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "SNStep_Hittech001.h"
#include "DlgSNStep.h"

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


// CSNStep_Hittech001App

BEGIN_MESSAGE_MAP(CSNStep_Hittech001App, CWinApp)
END_MESSAGE_MAP()


// CSNStep_Hittech001App ����

CSNStep_Hittech001App::CSNStep_Hittech001App()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CSNStep_Hittech001App ����

CSNStep_Hittech001App theApp;


// CSNStep_Hittech001App ��ʼ��

BOOL CSNStep_Hittech001App::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}



//�ӿڲ���
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
	CDlgSNStep *pDlgSNStep = new CDlgSNStep(pSnCfgPara);
	pSnCfgPara->DrvPrivData = dynamic_cast<void*>(pDlgSNStep);
	if (pDlgSNStep == NULL) {
		Ret = FALSE;
		goto __end;
	}
	if (pSnCfgPara->pParent != NULL) {
		Ret = pDlgSNStep->Create(CDlgSNStep::IDD,pSnCfgPara->pParent);
		if (Ret) {
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



BOOL SNInitCtrlsValueWithoutWnd(DRVSNCFGPARA *pSnCfgPara)
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
