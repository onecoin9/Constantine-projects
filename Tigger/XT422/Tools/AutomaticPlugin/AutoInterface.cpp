#include "AutoInterface.h"
//#include "../ComStruct/LogMsg.h"
//#include "./ComStruct/WorkThread.h"
//#include "AutoKProtocal.h"
#include "../ComStruct/ComFunc.h"
//#include "../ComStruct/GlbSetting.h"
//#include "../ComStruct/AprogDevMng.h"
//#include "AutoFProtocal.h"
#include "AutoSProtocal.h"



AutomicSpace::MessSingleton* AutomicSpace::MessSingleton::instance = nullptr;

/************************************************************************/
/* \brief 获取自动化设备接口                                            
/* \param[in] Manufacturer : 自动化设备厂商
/* \param[in] MType ：		自动化设备型号
/* \return 自动化设备接口
/************************************************************************/
IAutomatic* GetAutomatic(std::string ProtocalType, std::string& errStr)
{
	int Ret=0;
	IAutomatic*pIAuto=NULL;

		//tAutoSProtocalSetting* pSProtocalSetting=(tAutoSProtocalSetting*)Para;
		//if(pSProtocalSetting==NULL){
		//	goto __end;
		//}
		pIAuto = new CAutoSProtocal;
		tTCPAutoPara AutoPara;

		Ret = pIAuto->InitAutomatic(COMNTYPE_TCP, &AutoPara);
		if (Ret != 0) {
			pIAuto->GetErrMsg(0, errStr);
			Ret = pIAuto->ReleaseAutomatic();
			SAFEDEL(pIAuto);
		}
	if(pIAuto){
		pIAuto->m_ProtocalType=ProtocalType;
	}
	return pIAuto;
}

/************************************************************************/
/* 释放自动化设备的资源 
/* 成功返回0 ，失败返回负数
/************************************************************************/
int PutAutomatic(IAutomatic* pIFace)
{
	int Ret=0;

	if(pIFace){
		Ret=pIFace->ReleaseAutomatic();
		SAFEDEL(pIFace);
	}
	return Ret;
}
/************************************************************************/
/* 成功返回 0 失败返回负值 
/************************************************************************/
int SetAutomaticSiteDone(IAutomatic *pIFace,std::string& AutoAlias, uint8_t* Result,uint32_t Mask)
{
	int Ret=0;
	//CDevMng& GDevMng=GetGlobalDevMng();
	if(pIFace==NULL){
		Ret=-1; goto __end;
	}

   if(pIFace->m_ProtocalType=="SProtocal"){
		int SiteIdx=pIFace->GetSiteIdx(AutoAlias);
		if(SiteIdx<0){
			//GDevMng.PrintLog(LOGLEVEL_ERR,"Get site index error, AutoAlias=%s",AutoAlias);
		}
		Ret=pIFace->SetDoneSite(SiteIdx,Result,Mask);
	}
	else{
		//GDevMng.PrintLog(LOGLEVEL_ERR,"The automatic device manufacture=%s is not support yet",pIFace->m_ProtocalType);
	}
__end:
	return Ret;
}

/************************************************************************/
/* 失败返回-1，将指定的站点的IC Ready状态清掉                   */
/************************************************************************/
int ClearAutomaticSiteICReady( IAutomatic *pIFace,std::string& AutoAlias )
{
	int Ret=0;
	//CDevMng& GDevMng=GetGlobalDevMng();
	if(pIFace==NULL){
		Ret=-1; goto __end;
	}
	if(pIFace->m_ProtocalType=="SProtocal"){
		uint32_t SiteReadyStatus=0;
		int SiteIdx=pIFace->GetSiteIdx(AutoAlias);
		if(SiteIdx<0){
			//GDevMng.PrintLog(LOGLEVEL_ERR,"Get site index error, AutoAlias=%s",AutoAlias);
		}
		Ret=pIFace->ClearICReadySite(SiteIdx);

	}
	else{
		//GDevMng.PrintLog(LOGLEVEL_ERR,"The automatic device manufacture=%s is not support yet",pIFace->m_ProtocalType);
	}
__end:
	return Ret;
}


/************************************************************************/
/* 失败返回-1，检查到IC放下返回1，未检查到IC返回0                       */
/************************************************************************/
int CheckAutomaticSiteICReady(IAutomatic *pIFace,std::string& AutoAlias,uint8_t*pAutoChangeAdp,uint8_t*pICStatus)
{
	int Ret=0;
	//CDevMng& GDevMng=GetGlobalDevMng();
	*pAutoChangeAdp=false;
	*pICStatus=0xFF;///默认全部放置
	if(pIFace==NULL){
		Ret=-1; goto __end;
	}

	if(pIFace->m_ProtocalType=="SProtocal"){
		uint32_t SiteReadyStatus=0;
		uint8_t IsReady=0;
		int SiteIdx=pIFace->GetSiteIdx(AutoAlias);
		if(SiteIdx<0){
			Ret=-1;
			//GDevMng.PrintLog(LOGLEVEL_ERR,"Get site index error, AutoAlias=%s",AutoAlias);
			goto __end;
		}
		if(0){
			Ret=pIFace->GetICReadySite(SiteIdx,&IsReady);
			if(Ret!=0){
				//GDevMng.PrintLog(LOGLEVEL_ERR,"GetICReadySite failed Ret=%d",Ret);
				goto __end;
			}
			if(IsReady==1){
				Ret=1; ///IC 已经放好
			}
			else{
				Ret=0;///IC 还未放好
			}
		}
		else{
			uint32_t ICStatus;
			Ret=pIFace->GetICReadySite(SiteIdx,&IsReady,&ICStatus);//需要获取IC的放置状态，并将其中的没放置位置设置为Disable
			if(Ret!=0){
				//GDevMng.PrintLog(LOGLEVEL_ERR,"GetICReadySiteWithICStatus failed Ret=%d",Ret);
				goto __end;
			}
			if(IsReady==1){
				Ret=1; ///IC 已经放好
				*pAutoChangeAdp=true;
				*pICStatus=ICStatus;///更新芯片放置状态
			}
			else{
				Ret=0;///IC 还未放好
			}
		}
	}
	else{
		//GDevMng.PrintLog(LOGLEVEL_ERR,"The automatic protocal:%s is not support yet",pIFace->m_ProtocalType);
	}

__end:
	return Ret;
}

	
//int TestUart(void)
//{
//	int Ret=0;
//	uint8_t ICStatus=0,AutoChangeAdp=false;
//	tCOMPara ComPara;
//	std::string strAutoAlias="A";
//	IAutomatic*pIAuto=new CAutoKProtocal;
//	CDevMng& GDevMng=GetGlobalDevMng();
//	ComPara.dwComPort=4;
//	ComPara.dwBaudRate=CBR_9600;
//	ComPara.wDataWidth=8;
//	ComPara.wParityMode=ODDPARITY;
//	ComPara.wStopWidth=ONESTOPBIT;
//	ComPara.wFlowCtrl=FLOWCTRL_NONE;
//	Ret=pIAuto->InitAutomatic(COMNTYPE_UART,(void*)&ComPara);
//	if(Ret!=0){
//		goto __end;
//	}
//	pIAuto->m_ProtocalType="KProtocal";
//	while(1){
//		Sleep(1000);
//		if(CheckAutomaticSiteICReady(pIAuto,strAutoAlias,&AutoChangeAdp,&ICStatus)==1){
//			SetAutomaticSiteDone(pIAuto,strAutoAlias,0xF0,0xFF);
//		}
//	}
//
//__end:
//	return Ret;
//}

bool CheckAutomaticConnect( IAutomatic*pIFace )
{
	bool Ret=false;
	if(!pIFace){
		goto __end;
	}
	Ret=pIFace->CheckConnect();
__end:
	return Ret;
}

bool IAutomatic::IsCurProtocalVersionLargerThan(uint8_t ProtocalVer[])
{
	if(m_ProtocalVer[1]>ProtocalVer[1]){
		return  true;
	}
	else if(m_ProtocalVer[1]==ProtocalVer[1]){
		if(m_ProtocalVer[0]>ProtocalVer[0]){
			return true;
		}
		else{
			return false;
		}
	}
	else{
		return false;
	}
}
