// SNCustom_FC001.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "SCS_SN_THTuple_002.h"
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


// CSNCustom_FC001App

BEGIN_MESSAGE_MAP(CSNCustom_FC001App, CWinApp)
END_MESSAGE_MAP()


// CSNCustom_FC001App ����

CSNCustom_FC001App::CSNCustom_FC001App()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CSNCustom_FC001App ����

CSNCustom_FC001App theApp;


// CSNCustom_FC001App ��ʼ��

BOOL CSNCustom_FC001App::InitInstance()
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
	
	CDlgSNCustom *pDlgSN=new CDlgSNCustom(pSnCfgPara);
	pSnCfgPara->DrvPrivData=dynamic_cast<void*>(pDlgSN);
	if(pDlgSN==NULL){
		Ret=FALSE;
		goto __end;
	}
	if(pSnCfgPara->pParent!=NULL){
		Ret=pDlgSN->Create(CDlgSNCustom::IDD,pSnCfgPara->pParent);
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
	CDlgSNCustom *pDlgSN=reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN){
		delete pDlgSN;
		pSnCfgPara->DrvPrivData=NULL;
	}
	return TRUE;
}


BOOL SNShowWindow(DRVSNCFGPARA *pSnCfgPara ,BOOL bShow)
{
	CDlgSNCustom *pDlgSN=reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
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
	CDlgSNCustom *pDlgSN=reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
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
	if (true){
		return 0;
	}
	CDlgSNCustom *pDlgSN=reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
	if(pDlgSN){
		CSerial lSerial;
		if(pDlgSN->GetCtrls(lSerial)==FALSE){
			return -1;
		}
		else{
			pSnCfgPara->dwInitBufUse=lSerial.GetLength();
			if (lSerial.GetLength() == 0){
				return 0;
			}

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
	CDlgSNCustom *pDlgSN=reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
	if(Idx==0){
		goto __end;
	}
	if(pDlgSN){
		return pDlgSN->QuerySN(pSnCfgPara->AdapIdx,Idx,pData,pSize);
	}

__end:
	return -1;
}

INT SNPreLoad(DRVSNCFGPARA*pSNCfgPara)
{
	CDlgSNCustom *pDlgSN=reinterpret_cast<CDlgSNCustom*>(pSNCfgPara->DrvPrivData);
	if(pDlgSN){
		return pDlgSN->PreLoad();
	}

__end:
	return -1;
}

INT SNTellResult(DRVSNCFGPARA*pSnCfgPara,DWORD Idx,INT IsPass)
{
	CDlgSNCustom *pDlgSN=reinterpret_cast<CDlgSNCustom*>(pSnCfgPara->DrvPrivData);
	if(Idx==0){
		goto __end;
	}
	if(pDlgSN){
		return pDlgSN->TellResult(Idx,IsPass);
	}

__end:
	return -1;
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

