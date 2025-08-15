#include "SNStepCfg.h"



BOOL CSNStepCfg::AppendGroup(INT nNum)
{
	SNGROUP SNGroup;
	for(INT i=0;i<nNum;++i){
		SNGroup.ReInit();
		m_vSNGroup.push_back(SNGroup);
	}
	bSNGroupNum=(INT)m_vSNGroup.size();
	return TRUE;
}

BOOL CSNStepCfg::RemoveGroup(INT nNum)
{
	if(nNum>bSNGroupNum){
		nNum=bSNGroupNum;
	}
	for(INT i=0;i<nNum;++i){
		m_vSNGroup.pop_back();
	}
	bSNGroupNum=(INT)m_vSNGroup.size();
	return TRUE;
}

BOOL CSNStepCfg::SerialInCfgData( CSerial& lSerial )
{
	SNGROUP SNGroup;
	m_vSNGroup.clear();
	try{
		INT i;
		lSerial>>bSNGroupNum;
		for(i=0;i<bSNGroupNum;++i){
			SNGroup.ReInit();
			lSerial>>SNGroup.llSNStartAddr>>SNGroup.dwSNLen>>SNGroup.dwSNMode>>SNGroup.dwSNStyle;
			lSerial>>SNGroup.dwSNMSB>>SNGroup.strSNStartValue>>SNGroup.dwStep;
			m_vSNGroup.push_back(SNGroup);
		}
	}
	catch (CACException){
		return FALSE;
	}
	return TRUE;
}

BOOL CSNStepCfg::CheckDataValid(CString& strErrMsg)
{
	return TRUE;
}

BOOL CSNStepCfg::SerialOutCfgData( CSerial& lSerial )
{
	INT i;
	try{
		lSerial<<bSNGroupNum;
		for(i=0;i<bSNGroupNum;++i){
			SNGROUP& SNGroup=m_vSNGroup[i];
			lSerial<<SNGroup.llSNStartAddr<<SNGroup.dwSNLen<<SNGroup.dwSNMode<<SNGroup.dwSNStyle;
			lSerial<<SNGroup.dwSNMSB<<SNGroup.strSNStartValue<<SNGroup.dwStep;
		}
	}
	catch (CACException){
		return FALSE;
	}
	return TRUE;
}

void CSNStepCfg::ReInit()
{
	SNGROUP SnGroup;
	bSNGroupNum=1;
	m_vSNGroup.clear();
	m_vSNGroup.push_back(SnGroup);
}	

SNGROUP* CSNStepCfg::GetGroup( INT idx )
{
	if(idx>=bSNGroupNum){
		return NULL;
	}
	else{
		return &m_vSNGroup[idx];
	}
}

