// SNFile.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "SNFile.h"
#include "DlgSNFile.h"

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


// CSNFileApp

BEGIN_MESSAGE_MAP(CSNFileApp, CWinApp)
END_MESSAGE_MAP()


// CSNFileApp ����

CSNFileApp::CSNFileApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CSNFileApp ����

CSNFileApp theApp;


// CSNFileApp ��ʼ��

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
