#include "stdafx.h"
#include "SNScan.h"
#include "SNScanCfg.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"

CSNScanCfg::CSNScanCfg()
{
	m_strSNRule="Disable";
	m_SNRullhLib=NULL;
	memset(&m_SNRuleOpt,0,sizeof(tSNRuleOpt));
	ReInit();
}

CSNScanCfg::~CSNScanCfg()
{
	CloseSNRuleDll();
}

void CSNScanCfg::ReInit()
{
	tSNScanGroup SnGroup;
	m_strIniFile="";
	m_bSNGroupNum=1;
	m_bUIDEn=FALSE;
	m_bScannerEn=TRUE;
	m_vSNGroup.ReInit();
	m_vSNGroup.vSNCached.push_back(SnGroup);
}

BOOL CSNScanCfg::SerialInCfgData( CSerial& lSerial )
{
	tSNScanGroup SNGroup;
	m_vSNGroup.vSNCached.clear();
	try{
		INT i;
		lSerial>>m_strIniFile;
		lSerial>>m_bSNGroupNum;
		lSerial>>m_strSNRule;
		lSerial>>m_bScannerEn>>m_bUIDEn;
		for(i=0;i<m_bSNGroupNum;++i){
			SNGroup.ReInit();
			lSerial>>SNGroup.GroupIdx>>SNGroup.SNAddr>>SNGroup.SNSize>>SNGroup.SNName;
			m_vSNGroup.vSNCached.push_back(SNGroup);
		}
	}
	catch (...){
		return FALSE;
	}
	return TRUE;
}

BOOL CSNScanCfg::SerialOutCfgData( CSerial& lSerial )
{
	INT i;
	if(m_bScannerEn==FALSE && m_bUIDEn==FALSE){
		AfxMessageBox("\"Scanner Enable\" and \"Chip Unique ID Enable\" must choose at least one");
		return FALSE;
	}
	try{
		DRVSNCFGPARA *pSNCfgPara=(DRVSNCFGPARA*)m_pDrvSNCfgPara;
		lSerial<<m_strIniFile;
		lSerial<<m_bSNGroupNum;
		lSerial<<m_strSNRule;
		lSerial<<m_bScannerEn<<m_bUIDEn;
		for(i=0;i<m_bSNGroupNum;++i){
			tSNScanGroup& SNGroup=m_vSNGroup.vSNCached[i];
			lSerial<<SNGroup.GroupIdx<<SNGroup.SNAddr<<SNGroup.SNSize<<SNGroup.SNName;
		}
		if(m_bUIDEn){
			pSNCfgPara->OptFlag |=OPTFLAG_UIDEN;
		}
		else{
			pSNCfgPara->OptFlag &=~OPTFLAG_UIDEN;
		}
	}
	catch (...){
		return FALSE;
	}
	return TRUE;
}

BOOL CSNScanCfg::AppendGroup( INT nNum )
{
	tSNScanGroup SNGroup;
	INT GroupCntCur=(INT)m_vSNGroup.vSNCached.size();
	for(INT i=0;i<nNum;++i){
		SNGroup.ReInit();
		SNGroup.GroupIdx=GroupCntCur+1+i;
		m_vSNGroup.vSNCached.push_back(SNGroup);
	}
	m_bSNGroupNum=(BYTE)m_vSNGroup.vSNCached.size();
	return TRUE;
}

BOOL CSNScanCfg::RemoveGroup( INT nNum )
{
	if(nNum>m_bSNGroupNum){
		nNum=m_bSNGroupNum;
	}
	for(INT i=0;i<nNum;++i){
		m_vSNGroup.vSNCached.pop_back();
	}
	m_bSNGroupNum=(INT)m_vSNGroup.vSNCached.size();
	return TRUE;
}

tSNScanGroup* CSNScanCfg::GetGroup( INT idx )
{
	if(idx>=m_bSNGroupNum){
		return NULL;
	}
	else{
		return &m_vSNGroup.vSNCached[idx];
	}
}

extern CSNScanApp theApp;

CString DllGetCurrentPath( void )
{
	TCHAR szFilePath[MAX_PATH + 1]; 
	TCHAR *pPos=NULL;
	CString str_url;
	GetModuleFileName(theApp.m_hInstance, szFilePath, MAX_PATH); 
	pPos=_tcsrchr(szFilePath, _T('\\'));
	if(pPos!=NULL){
		pPos[0] = 0;//删除文件名，只获得路径
		str_url=CString(szFilePath);
	}
	else{
		pPos=_tcsrchr(szFilePath, _T('/'));
		if(pPos==NULL){
			str_url="";
		}
		else{
			str_url=CString(szFilePath);
		}
	}	
	return str_url;
}

INT CSNScanCfg::SearchSNRule( )
{
	WIN32_FIND_DATA FindFileData;
	CString			strFind;
	strFind.Format("%s\\snrule\\*.dll",DllGetCurrentPath());
	HANDLE hFind=::FindFirstFile(strFind,&FindFileData);
	m_vSNRules.clear();
	if(INVALID_HANDLE_VALUE == hFind) {
		return -1;
	}

	while(TRUE) {
		CString strFileName;
		strFileName.Format(FindFileData.cFileName);
		strFileName.Replace(".dll", "");
		if (strFileName.CompareNoCase("UssKeyGen_0002") == 0){
			if(!FindNextFile(hFind,&FindFileData)){
				break;
			} else{
				continue;
			}
		}
		m_vSNRules.push_back(strFileName);
		if(!FindNextFile(hFind,&FindFileData)) break;
	}

	strFind.Format("%s\\snrule\\*.exe",DllGetCurrentPath());
	hFind=::FindFirstFile(strFind,&FindFileData);

	if(INVALID_HANDLE_VALUE == hFind) {
		return -1;
	}

	while(TRUE) {
		CString strFileName;
		strFileName.Format("SNRuleExe");
		m_vSNRules.push_back(strFileName);
		if(!FindNextFile(hFind,&FindFileData)) break;
	}

	FindClose(hFind);
	return 0;
}

BOOL CSNScanCfg::CloseSNRuleDll()
{
	if(m_SNRullhLib!=NULL){
		if(m_SNRuleOpt.pFreeSNRule){
			m_SNRuleOpt.pFreeSNRule(m_SNRuleOpt.hRule);
		}
		FreeLibrary(m_SNRullhLib);
		m_SNRullhLib=NULL;
		memset(&m_SNRuleOpt,0,sizeof(tSNRuleOpt));
	}
	return TRUE;
}

BOOL CSNScanCfg::InitSNRuleDll(CString& strErrmsg)
{
	INT Ret=TRUE;
	if(m_SNRuleOpt.pSetSNCfg){
		if(m_SNRuleOpt.pSetSNCfg(m_SNRuleOpt.hRule,(LPSTR)(LPCSTR)m_strIniFile)!=0){
			BYTE pMsg[256];
			memset(pMsg,0,256);
			m_SNRuleOpt.pGetErrMsg(m_SNRuleOpt.hRule,pMsg,256);
			strErrmsg.Format("Function \"SetSNCfg\" return failed, ErrMsg=%s",pMsg);
			Ret=FALSE;
		}
	}
	return Ret;
}

INT CSNScanCfg::SetUID( DWORD Idx,BYTE*pUID,INT Size )
{
	INT Cnt,i,bFound=0;
	DRVSNCFGPARA*pCfgPara=(DRVSNCFGPARA*)m_pDrvSNCfgPara;
	tSNCache tmpSNCache;
	tmpSNCache.AdpIdx=pCfgPara->AdapIdx;
	tmpSNCache.strSiteSN.Format("%s",pCfgPara->pSiteSN);
	for(i=0;i<Size;i++){
		CString strTmp;
		strTmp.Format("%02X",pUID[i]);
		tmpSNCache.ChipID +=strTmp;
	}
	tmpSNCache.vSNCached=m_vSNGroup.vSNCached;
	Cnt=(INT)m_SNCached.size();
	for(i=Cnt-1;i>=0;i--){
		///现在原来的Cache队列中查找是否有之前已经Cache过的SN别名和Adp Idx，如果有，则替换值就行，如果没有在进行Push
		if(m_SNCached[i].AdpIdx==tmpSNCache.AdpIdx && m_SNCached[i].strSiteSN==tmpSNCache.strSiteSN){
			m_SNCached[i].ChipID.Format("%s",tmpSNCache.ChipID);
			m_SNCached[i].vSNCached=tmpSNCache.vSNCached;
			bFound=1;
			break;
		}
	}
	if(bFound==0){
		m_SNCached.push_back(tmpSNCache);
	}
	return 0;
}

INT CSNScanCfg::ClearCache()
{
	m_SNCached.clear();
	return 0;
}

INT CSNScanCfg::QuerySN( DWORD Idx,BYTE*pData,INT*pSize )
{
	INT Ret=0;
	CString strErMsg;
	BYTE *pSNValue=NULL;
	try{
		if(m_SNRuleOpt.pGetSNValue){
			CString IDTag,SNTag;
			CSerial lSerial;
			INT i,Cnt;
			Cnt=(INT)m_vSNGroup.vSNCached.size();
			lSerial<<Cnt;
			if(Cnt!=0){		
				for(i=0;i<Cnt;i++){	
					CSerial lSerialSN;
					pSNValue=new BYTE[m_vSNGroup.vSNCached[i].SNSize];
					if(!pSNValue){
						strErMsg.Format("SNScan: Memory alloc for SNValue failed, Bytes=%d",m_vSNGroup.vSNCached[i].SNSize);
						AfxMessageBox(strErMsg);
						Ret=-1; goto __end;
					}
					memset(pSNValue,0,m_vSNGroup.vSNCached[i].SNSize);
					lSerial<<m_vSNGroup.vSNCached[i].SNAddr;
					lSerial<<m_vSNGroup.vSNCached[i].SNSize;
					if(m_bUIDEn){
						IDTag.Format("ID:%s",m_vSNGroup.ChipID);
						lSerialSN<<IDTag;
					}
					if(m_bScannerEn){
						SNTag.Format("SN:%s",m_vSNGroup.vSNCached[i].SNScaned);
						lSerialSN<<SNTag;
					}
					Ret= m_SNRuleOpt.pGetSNValue(m_SNRuleOpt.hRule,Idx,(BYTE*)(LPCTSTR)m_vSNGroup.vSNCached[i].SNName,
						lSerialSN.GetBuffer(),lSerialSN.GetLength(),pSNValue,m_vSNGroup.vSNCached[i].SNSize);
					if(Ret!=0){
						BYTE ErrMsg[256];
						memset(ErrMsg,0,256);
						m_SNRuleOpt.pGetErrMsg(m_SNRuleOpt.hRule,ErrMsg,256);
						strErMsg.Format("SNRule GetSNValue Failed Ret=%d, ErrMsg=%s",Ret,ErrMsg);
						Ret=-1; goto __end;
					}
					lSerial.SerialInBuff(pSNValue,m_vSNGroup.vSNCached[i].SNSize);
					delete[] pSNValue;
					pSNValue=NULL;
				}
			}
			else{
				strErMsg.Format("GetSNValue SNGroup Count is zero");
				Ret=-3; goto __end;
			}
			if(*pSize>=lSerial.GetLength()){
				*pSize=lSerial.GetLength();
				memcpy(pData,lSerial.GetBuffer(),lSerial.GetLength());
			}
			else{
				*pSize=lSerial.GetLength();
				Ret=-2; goto __end;
			}
		}
		else{
			strErMsg.Format("GetSNValue function is NULL");
			Ret=-1; goto __end;
		}
	}
	catch (...){
		Ret=-1;
		strErMsg.Format("SNScan QuerySN Get An Exception");
	}
__end:
	if(pSNValue){
		delete[] pSNValue;
	}
	if(Ret==-1){
		AfxMessageBox(strErMsg);
	}
	return Ret;
}

///外部有可能连续多次调用扫描界面来一次性获取多台多个站点的SN，这个时候由于还没
///到真正获取SN烧录值得时候，所以需要先将其Cache起来
INT CSNScanCfg::PushCache()
{
	INT Cnt,i,bFound=0;
	DRVSNCFGPARA*pCfgPara=(DRVSNCFGPARA*)m_pDrvSNCfgPara;
	tSNCache tmpSNCache;
	tmpSNCache.AdpIdx=pCfgPara->AdapIdx;
	tmpSNCache.strSiteSN.Format("%s",pCfgPara->pSiteSN);
	tmpSNCache.vSNCached=m_vSNGroup.vSNCached;
	tmpSNCache.ChipID=m_vSNGroup.ChipID;
	Cnt=(INT)m_SNCached.size();
	for(i=Cnt-1;i>=0;i--){
		///现在原来的Cache队列中查找是否有之前已经Cache过的SN别名和Adp Idx，如果有，则替换值就行，如果没有在进行Push
		if(m_SNCached[i].AdpIdx==tmpSNCache.AdpIdx && m_SNCached[i].strSiteSN==tmpSNCache.strSiteSN){
			m_SNCached[i].vSNCached=tmpSNCache.vSNCached;
			m_SNCached[i].ChipID=tmpSNCache.ChipID;
			bFound=1;
			break;
		}
	}
	if(bFound==0){
		m_SNCached.push_back(tmpSNCache);
	}
	return 0;
}
///当外部需要开始计算，就需要从之前Cache中获取相应的扫描条码，并将其从Cache中删除
///成功返回0，失败返回-1
INT CSNScanCfg::PopCache()
{
	INT Ret=-1;
	DRVSNCFGPARA*pCfgPara=(DRVSNCFGPARA*)m_pDrvSNCfgPara;
	INT i, Cnt;
	CString strSiteSN;
	strSiteSN.Format("%s",pCfgPara->pSiteSN);
	Cnt=(INT)m_SNCached.size();
	for(i=Cnt-1;i>=0;i--){
		///找到需要的并还原回去，然后在Cache队列中删除
		if(m_SNCached[i].AdpIdx==pCfgPara->AdapIdx && m_SNCached[i].strSiteSN==strSiteSN){
				m_vSNGroup.vSNCached=m_SNCached[i].vSNCached;
				m_vSNGroup.ChipID.Format("%s",m_SNCached[i].ChipID);
				m_SNCached.erase(m_SNCached.begin()+i);
				Ret=0; goto __end;
		}
	}
__end:
	if(Ret==-1){
		CString strErr;
		strErr.Format("Can't find %s Adpidx[%d] SNScanned Cached",pCfgPara->pSiteSN,pCfgPara->AdapIdx);
		AfxMessageBox(strErr,MB_OK|MB_ICONERROR);
	}
	return Ret ;
}

BOOL CSNScanCfg::OpenSNRuleDll( CString strSNRule )
{
	BOOL Ret=TRUE;
	CString	strSNRullPath;
	CString	strErrmsg;
	CloseSNRuleDll();
	if(strSNRule=="Disable"){
		return TRUE;
	}
	strSNRullPath.Format("%s\\snrule\\%s.dll",DllGetCurrentPath(),strSNRule);
	m_SNRullhLib=LoadLibrary(strSNRullPath);
	if(m_SNRullhLib==NULL){
		strErrmsg.Format("SN Config: Load %s.dll failed, ErrCode=%d",strSNRule,GetLastError());
		Ret=FALSE; goto __end;
	}
	else{
		m_SNRuleOpt.pInitSNRule=(FuncInitSNRule)GetProcAddress(m_SNRullhLib, "InitSNRule");
		if(m_SNRuleOpt.pInitSNRule==NULL){
			strErrmsg.Format("[%s.dll] Can't find InitSNRule function, Export it first",strSNRule);
			Ret=FALSE;
			goto __end;
		}

		m_SNRuleOpt.pFreeSNRule=(FuncFreeSNRule)GetProcAddress(m_SNRullhLib, "FreeSNRule");
		if(m_SNRuleOpt.pFreeSNRule==NULL){
			strErrmsg.Format("[%s.dll] Can't find FreeSNRule function, Export it first",strSNRule);
			Ret=FALSE;
			goto __end;
		}

		m_SNRuleOpt.pSetSNCfg=(FuncSetSNCfg)GetProcAddress(m_SNRullhLib, "SetSNCfg");
		if(m_SNRuleOpt.pSetSNCfg==NULL){
			strErrmsg.Format("[%s.dll] Can't find SetSNCfg function, Export it first",strSNRule);
			Ret=FALSE;
			goto __end;
		}
		m_SNRuleOpt.pGetSNValue=(FuncGetSNValue)GetProcAddress(m_SNRullhLib, "GetSNValue");
		if(m_SNRuleOpt.pGetSNValue==NULL){
			strErrmsg.Format("[%s.dll] Can't find GetSNValue function, Export it first",strSNRule);
			Ret=FALSE;
			goto __end;
		}
		m_SNRuleOpt.pGetErrMsg=(FuncGetErrMsg)GetProcAddress(m_SNRullhLib, "GetErrMsg");
		if(m_SNRuleOpt.pGetErrMsg==NULL){
			strErrmsg.Format("[%s.dll] Can't find GetErrMsg function, Export it first",strSNRule);
			Ret=FALSE;
			goto __end;
		}

		m_SNRuleOpt.pSetSNResult=(FuncSetSNResult)GetProcAddress(m_SNRullhLib, "SetSNResult");
		if(strSNRule.CompareNoCase("SNRuleUUIDV4") == 0 && m_SNRuleOpt.pSetSNResult==NULL){
			strErrmsg.Format("[%s.dll] Can't find SetSNResult function, Export it first",strSNRule);
			Ret=FALSE;
			goto __end;
		}
	}

	if(m_SNRuleOpt.pInitSNRule(&m_SNRuleOpt.hRule)!=0){
		BYTE pMsg[256];
		memset(pMsg,0,256);
		m_SNRuleOpt.pGetErrMsg(m_SNRuleOpt.hRule,pMsg,256);
		strErrmsg.Format("Function \"InitSNRule\" return failed, ErrMsg=%s",pMsg);
		Ret=FALSE; goto __end;
	}

	if(m_strIniFile!=""){
		Ret=InitSNRuleDll(strErrmsg);
	}

__end:
	if(Ret==FALSE){
		CloseSNRuleDll();
		AfxMessageBox(strErrmsg,MB_OK|MB_ICONERROR);
	}
	return Ret;
}



BOOL CSNScanCfg::SNUUIDTellResult(DWORD Idx,INT IsPass)
{
	INT Ret = 0;

	if (m_SNRuleOpt.pSetSNResult == NULL)
		return Ret;

	Ret = m_SNRuleOpt.pSetSNResult(m_SNRuleOpt.hRule,Idx,IsPass);
	if(Ret != 0)
	{
		CString strErrmsg;
		strErrmsg.Format("UUID Error Can't be Write to CSV File!");
		AfxMessageBox(strErrmsg,MB_OK|MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}