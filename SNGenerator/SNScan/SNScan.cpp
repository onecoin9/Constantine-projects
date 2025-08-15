// SNScan.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "SNScan.h"
#include "DlgSNScan.h"

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


// CSNScanApp

BEGIN_MESSAGE_MAP(CSNScanApp, CWinApp)
END_MESSAGE_MAP()


// CSNScanApp ����

CSNScanApp::CSNScanApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CSNScanApp ����

CSNScanApp theApp;

// CSNScanApp ��ʼ��
BOOL CSNScanApp::InitInstance()
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
	CDlgSNScan *pDlgSN=new CDlgSNScan(pSnCfgPara);
	pSnCfgPara->DrvPrivData=dynamic_cast<void*>(pDlgSN);
	if(pDlgSN==NULL){
		Ret=FALSE;
		goto __end;
	}
	if(pSnCfgPara->pParent!=NULL){
		Ret=pDlgSN->Create(CDlgSNScan::IDD,pSnCfgPara->pParent);
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
	CDlgSNScan *pDlgSN=reinterpret_cast<CDlgSNScan*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN){
		delete pDlgSN;
		pSnCfgPara->DrvPrivData=NULL;
	}
	return TRUE;
}

BOOL SNShowWindow(DRVSNCFGPARA *pSnCfgPara ,BOOL bShow)
{
	CDlgSNScan *pDlgSN=reinterpret_cast<CDlgSNScan*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN && pDlgSN->GetSafeHwnd()){
		if(bShow){
			pDlgSN->ShowWindow(SW_SHOW);
			pDlgSN->UpdateData(FALSE);
		}
		else
			pDlgSN->ShowWindow(SW_HIDE);
		return TRUE;
	}
	else{
		return FALSE;
	}
}

BOOL SNInitCtrlsValue(DRVSNCFGPARA *pSnCfgPara)
{
	CDlgSNScan *pDlgSN=reinterpret_cast<CDlgSNScan*>(pSnCfgPara->DrvPrivData);
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
	CDlgSNScan *pDlgSN=reinterpret_cast<CDlgSNScan*>(pSnCfgPara->DrvPrivData);
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
	INT Ret=0;
	CDlgSNScan *pDlgSN=reinterpret_cast<CDlgSNScan*>(pSnCfgPara->DrvPrivData);
	HINSTANCE save_hInstance = AfxGetResourceHandle();
	AfxSetResourceHandle(theApp.m_hInstance);
	if(Idx==0){
		Ret=-1;
		goto __end;
	}
	if(pDlgSN){
		Ret=pDlgSN->QuerySN(Idx,pData,pSize);
		if(Ret==0){
			Ret=*pSize;///��Ҫ����ʵ��ʹ�õ��ֽ���
		}
	}
__end:
	AfxSetResourceHandle(save_hInstance);
	return Ret;
}

INT SNSetUID(DRVSNCFGPARA*pSnCfgPara,DWORD Idx,BYTE*pUID,INT Size)
{
	INT Ret=0;
	CDlgSNScan *pDlgSN=reinterpret_cast<CDlgSNScan*>(pSnCfgPara->DrvPrivData);
	HINSTANCE save_hInstance = AfxGetResourceHandle();
	AfxSetResourceHandle(theApp.m_hInstance);

	if(pDlgSN){
		Ret=pDlgSN->SetUID(Idx,pUID,Size);
	}

__end:
	AfxSetResourceHandle(save_hInstance);
	return Ret;
}

INT SNFetchScanned(DRVSNCFGPARA*pSnCfgPara,DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	CDlgSNScan *pDlgSN=reinterpret_cast<CDlgSNScan*>(pSnCfgPara->DrvPrivData);
	HINSTANCE save_hInstance = AfxGetResourceHandle();
	AfxSetResourceHandle(theApp.m_hInstance);

	if(pDlgSN){
		Ret=pDlgSN->FetchSN(Idx,pData,pSize);
	}

__end:
	AfxSetResourceHandle(save_hInstance);
	return Ret;
}

INT SNFetchScannedForDB(DRVSNCFGPARA*pSnCfgPara,DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	CDlgSNScan *pDlgSN=reinterpret_cast<CDlgSNScan*>(pSnCfgPara->DrvPrivData);
	HINSTANCE save_hInstance = AfxGetResourceHandle();
	AfxSetResourceHandle(theApp.m_hInstance);

	if(pDlgSN){
		Ret=pDlgSN->FetchSNForDB(Idx,pData,pSize);
	}

__end:
	AfxSetResourceHandle(save_hInstance);
	return Ret;
}

INT SNTellResult(DRVSNCFGPARA*pSnCfgPara,DWORD Idx,INT IsPass)
{
	INT Ret=0;
	CDlgSNScan *pDlgSN=reinterpret_cast<CDlgSNScan*>(pSnCfgPara->DrvPrivData);
	HINSTANCE save_hInstance = AfxGetResourceHandle();
	AfxSetResourceHandle(theApp.m_hInstance);
	
	if(pDlgSN){
		Ret=pDlgSN->SNUUIDResult(Idx,IsPass);
	}

__end:
	AfxSetResourceHandle(save_hInstance);
	return Ret;
}
