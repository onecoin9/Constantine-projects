#include "StdAfx.h"
#include "SNDirFiles_VarSizeCfg.h"

void CSNDirFiles_VarSizeCfg::ReInit()
{
	SNGROUP SnGroup;
	bSNGroupNum=1;
	m_vSNGroup.clear();
	m_vSNGroup.push_back(SnGroup);
}

BOOL CSNDirFiles_VarSizeCfg::SerialInCfgData(CSerial& lSerial)
{
	SNGROUP SNGroup;
	m_vSNGroup.clear();
	try{
		INT i;
		lSerial>>bSNGroupNum;
		for(i=0;i<bSNGroupNum;++i){
			SNGroup.ReInit();
			lSerial>>SNGroup.llSNStartAddr>>SNGroup.dwSNLen>>SNGroup.strSeperated>>SNGroup.strSampleBin;
			lSerial>>SNGroup.strSNDir>>SNGroup.strMaxSize>>SNGroup.strFillData;
			m_vSNGroup.push_back(SNGroup);
		}
	}
	catch (CACException){
		return FALSE;
	}
	return TRUE;
}

BOOL CSNDirFiles_VarSizeCfg::SerialOutCfgData(CSerial& lSerial)
{
	INT i;
	try{
		lSerial<<bSNGroupNum;
		for(i=0;i<bSNGroupNum;++i){
			SNGROUP& SNGroup=m_vSNGroup[i];
			lSerial<<SNGroup.llSNStartAddr<<SNGroup.dwSNLen<<SNGroup.strSeperated<<SNGroup.strSampleBin;
			lSerial<<SNGroup.strSNDir<<SNGroup.strMaxSize<<SNGroup.strFillData;
		}
	}
	catch (CACException){
		return FALSE;
	}
	return TRUE;
}

BOOL CSNDirFiles_VarSizeCfg::AppendGroup(INT nNum)
{
	SNGROUP SNGroup;
	for(INT i=0;i<nNum;++i){
		SNGroup.ReInit();
		m_vSNGroup.push_back(SNGroup);
	}
	bSNGroupNum=(INT)m_vSNGroup.size();
	return TRUE;
}

BOOL CSNDirFiles_VarSizeCfg::RemoveGroup(INT nNum)
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

SNGROUP* CSNDirFiles_VarSizeCfg::GetGroup(INT idx)
{
	if(idx>=bSNGroupNum){
		return NULL;
	}
	else{
		return &m_vSNGroup[idx];
	}
}

//SNGROUP* CSNDirFiles_VarSizeCfg::GetGroup(CString strFilePath)
//{
//	if (strFilePath.IsEmpty()){
//		return NULL;
//	}
//	for (UINT i = 0; i<m_vSNGroup.size(); ++i){
//		if (strFilePath.CompareNoCase(m_vSNGroup[i].strSNDir) == 0){
//			return &m_vSNGroup[i];
//		}
//		if (i=m_vSNGroup.size() && strFilePath.CompareNoCase(m_vSNGroup[i].strSNDir) != 0){
//			return NULL;
//		}
//	}
//}

BOOL CSNDirFiles_VarSizeCfg::CheckDataValid(CString& strErrMsg)
{
	return FALSE;
}

INT CSNDirFiles_VarSizeCfg::GetSNTotalSize()
{
	INT i;
	INT Size=4;
	for(i=0;i<bSNGroupNum;++i){
		SNGROUP& SNGroup=m_vSNGroup[i];
		Size +=12;
		Size +=SNGroup.dwSNLen;
	}
	return Size;
}
