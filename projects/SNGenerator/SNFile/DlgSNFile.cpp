// DlgSNFile.cpp : 实现文件
//

#include "stdafx.h"
#include "SNFile.h"
#include "DlgSNFile.h"
#include "../Com/cJSON.h"
#include "../Com/ComTool.h"
#include "../Com/ComFunc.h"



#define isdigital(a)	(((a)<='9' && (a)>='0')?1:0)
#define isupper(a)	(((a)>='A' && (a)<='F')?1:0)
#define islower(a)		(((a)>='a' && (a)<='f')?1:0)
#define isalpha_hex(a)	((((a)>='A' && (a)<='F')||((a)>='a'&&(a)<='f'))?1:0)
// CDlgSNFile 对话框

IMPLEMENT_DYNAMIC(CDlgSNFile, CDialog)

CDlgSNFile::CDlgSNFile(DRVSNCFGPARA *pSNCfgPara,CWnd* pParent /*=NULL*/)
	: m_pSNCfgPara(pSNCfgPara),CDialog(CDlgSNFile::IDD, pParent)
	, m_strFile(_T(""))
	, m_ChipName(_T(""))
	, m_ByteMode(_T(""))
	, m_DataMode(_T(""))
{

}

CDlgSNFile::~CDlgSNFile()
{
	CloseSNSer();
	m_vRecordIdxMap.clear();
}

BOOL CDlgSNFile::InitCtrls( CSerial& lSerial )
{
	BOOL Ret=TRUE;
	if(lSerial.GetLength()!=0){
		CString strFile;
		lSerial>>strFile>>m_ChipName>>m_ByteMode>>m_DataMode;
		if(strFile!=m_strFile){
			m_strFile=strFile;
			CloseSNSer();
			if(OpenSNSer()!=0){
				//AfxMessageBox("Open the ser file failed",MB_OK|MB_ICONERROR);
				Ret= FALSE;
			}
			else{
				if(GetSNSerChipInfo()==0){
					PrepareSNInfo();
					BuildIdxMap();
				}
				else{
					Ret=FALSE;
				}
			}
		}
	}
	else{
		m_strFile="";
		m_ChipName="";
		m_ByteMode="";
		m_DataMode="";
	}
	UpdateData(FALSE);
	return Ret;
}

/*
	二次开发采取的
*/
BOOL CDlgSNFile::InitCtrlsWthoutWnd( CSerial& lSerial )
{
	BOOL Ret=TRUE;
	if(lSerial.GetLength()!=0){
		CString strFile;
		lSerial>>strFile>>m_ChipName>>m_ByteMode>>m_DataMode;
		if(strFile!=m_strFile){
			m_strFile=strFile;
			CloseSNSer();
			if(OpenSNSer()!=0){
				AfxMessageBox("Open the ser file failed",MB_OK|MB_ICONERROR);
				UpdateData(FALSE);
				Ret= FALSE;
			}
			else{
				if(GetSNSerChipInfo()==0){
					PrepareSNInfo();
					BuildIdxMap();
				}
				else{
					Ret=FALSE;
				}
			}
		}
	}
	else{
		m_strFile="";
		m_ChipName="";
		m_ByteMode="";
		m_DataMode="";
	}
	return Ret;
}


BOOL CDlgSNFile::GetCtrls( CSerial&lSerial )
{
	lSerial<<m_strFile<<m_ChipName<<m_ByteMode<<m_DataMode;
	return TRUE;
}

static int Trimspace(BYTE* databuf,int len)
{
	int i=0,j=0;
	int reallen=0;
	while(i<len){
		if(databuf[i]==0x20){
			i++;
			continue;
		}
		else if(databuf[i]==0){
			break;
		}
		else{
			if(i!=j){
				databuf[j]=databuf[i];
			}
			i++;
			j++;
		}
	}
	reallen=j;
	if(j!=len){
		for(;j<len;j++)
			databuf[j]=0;
	}
	return reallen;
}

static int GetSNIndex(UINT*p_idx,BYTE*buf,int* offset)
{
	int ret=0;
	int i=0;
	UINT idx=0;
	i=*offset;

	if(strncmp((char*)(buf+i),"<L:",3)!=0){
		ret = -1;
		goto __end;
	}
	i+=3;	
	while(1){///get index
		if(buf[i]=='>'){
			break;
		}
		else{
			if(isdigital(buf[i]))
				idx = idx*10+(buf[i]-'0');
			else{
				ret = -2;
				goto __end;
			}
		}
		i++;
	}
	i++;
	*offset = i;
	*p_idx= idx;
__end:
	return ret;
}

static int GetSNAddr(UINT64*p_addr,BYTE*buf,int* offset)
{
	UINT64 addr=0;
	int ret=0;
	int i=*offset;
	if(buf[i]!='<'){
		ret = -1;
		goto __end;
	}
	i++;
	while(1){///get addr
		if(buf[i]=='>'){
			break;	
		}
		else{
			if(isdigital(buf[i])){
				addr = addr*16+(buf[i]-'0');
			}
			else if(isupper(buf[i])){
				addr = addr*16+(buf[i]-'A'+0xA);
			}
			else if(islower(buf[i])){
				addr = addr*16+(buf[i]-'a'+0xA);
			}
			else{
				ret = -2;
				goto __end;
			}
		}
		i++;
	}
	i++;
	*offset=i;
	*p_addr = addr;
__end:
	return ret;
}


static int GetSNSize(UINT *p_size,BYTE*buf,int *offset)
{
	UINT size=0;
	int ret=0;
	int i=*offset;
	if(buf[i]!='<'){
		ret = -1;
		goto __end;
	}
	i++;
	while(1){///get addr
		if(buf[i]=='>'){
			break;	
		}
		else{
			if(isdigital(buf[i])){
				size = size*16+(buf[i]-'0');
			}
			else if(isupper(buf[i])){
				size = size*16+(buf[i]-'A'+0xA);
			}
			else if(islower(buf[i])){
				size = size*16+(buf[i]-'a'+0xA);
			}
			else{
				ret = -2;
				goto __end;
			}
		}
		i++;
	}
	i++;
	*offset=i;
	*p_size = size;
__end:
	return ret;
}

/**
get the sn value from line
p_sn : the pointer to the SN_info
pp_snv : where to store the pointer of sn value buffer which is malloced internally
len : the length pp_snv buffer 
buf : the data of line
offset : where to get start

return :
 	-1 : failed
 	0 : pass
***/
#define sn_zmalloc(size)	((BYTE*)malloc(size))
#define sn_free(ptr)		free(ptr);
int CDlgSNFile::GetSNValue(BYTE**pp_snv,int*len,BYTE*buf,int* offset)
{
	BOOL RetCall=FALSE;
	BYTE*sn_str=NULL;
	int sn_len=16;///the length of the buffer where to store the sn value fetch from the line
	BYTE*tmpstr=NULL;
	int ret=0;
	int i=*offset,j=0;
	if(buf[i]!='<'){
		m_strErrMsg.Format("Line[%d] error, Can't find \"<\" when parser sn value",m_SNInfo.LineIndex);
		ret = -1;
		goto __end;
	}
	i++;
	while(1){///get sn value
		if(buf[i]=='>'){
			break;
		}
		else{
			if(buf[i]==0xa){///can't match '>'
				m_strErrMsg.Format("Line[%d] error, Can't match \">\" when parser sn value",m_SNInfo.LineIndex);
				ret=-1;
				goto __end;
			}	
			else{
			
				if(sn_str==NULL){
					sn_str = sn_zmalloc(sn_len);
					if(sn_str==NULL){
						ret = -1;
						goto __end;
					}	
				}else{
					if(j>=sn_len-1){///need to enlarge the space
						sn_len+=16;
						tmpstr=sn_zmalloc(sn_len);
						if(tmpstr==NULL){
							ret = -1;
							goto __end;
						}
						memcpy(tmpstr,sn_str,sn_len-16);
						sn_free(sn_str);
						sn_str = tmpstr;
					}
				}
				sn_str[j] = buf[i];
				j++;		///count the real hex length
			}
		}
		i++;
	}
	i++;
	*offset = i;
	sn_str[j]=0;	///end with zero

	if(m_SNInfo.SNDataMode== DATAMODE_DATA){
		std::vector<BYTE> vSNValue;
		CString strSN;
		BYTE *pSNData=NULL;
		strSN.Format("%s",sn_str);
		strSN.Replace(" ","");
		if(m_SNInfo.SNByteMode==BYTEMODE_BE){
			RetCall=ComTool::Str2HexNoTruncate(strSN,ComTool::ENDIAN_BIG,vSNValue);
		}
		else{
			RetCall=ComTool::Str2HexNoTruncate(strSN,ComTool::ENDIAN_LIT,vSNValue);
		}
		if(RetCall==FALSE){
			m_strErrMsg.Format("Line[%d] error, Parse SN Value Failed, SNInfo:%s",m_SNInfo.LineIndex,strSN.GetBuffer());
			ret=-1; goto __end;
		}
		pSNData=sn_zmalloc(vSNValue.size());
		if(!pSNData){
			m_strErrMsg.Format("Line[%d] error, Memory alloc failed",m_SNInfo.LineIndex);
			ret=-1; goto __end;
		}
		copy(vSNValue.begin(),vSNValue.end(),pSNData);
		*pp_snv = pSNData;
		*len = (INT)vSNValue.size();
	}else if (m_SNInfo.SNDataMode == DATAMODE_FILE || m_SNInfo.SNDataMode == DATAMODE_MIX ){
		
		BYTE *pSNData= sn_zmalloc(j/*+1*/);
		if(!pSNData){
			m_strErrMsg.Format("Line[%d] error, Memory alloc failed",m_SNInfo.LineIndex);
			ret=-1; goto __end;
		}
		memcpy(pSNData, sn_str,  j/*+1*/);
		*pp_snv = pSNData;	
		*len = j/*+1*/;
	}
	else{///need to read data from file
		m_strErrMsg.Format("DataMode=File Not support yet");
		ret=-1;
		goto __end;
	}
	
__end:
	if(sn_str!=NULL)
		sn_free(sn_str);
	if(ret==-1){
		*pp_snv = NULL;
		*len = 0;
	}
	return ret;
}

INT CDlgSNFile::GetCurRowFormat(CString strSrc)
{
	INT Rtn = -1;
	if (strSrc.IsEmpty()){
		goto __end;
	}

	if (CComFunc::GetFileExt(strSrc).IsEmpty()){
		Rtn = 0;
	}else{
		Rtn = 1;
	}
	
__end:
	return Rtn;
}

INT CDlgSNFile::GetDataFromFile(CString strFileName, BYTE** pSN, int nExpectSize)
{
	INT nRtn = -1;
	CString strPath;
	CFile file;
	UINT FileLen = 0;
	BYTE* pTemp = NULL;
	int nCharCount = nExpectSize *2;
	std::vector<BYTE> vSNValue;
	CString strSN;

	strPath.Format("%s\\%s", CComFunc::GetCurrentPath(), strFileName);

	if ( file.Open(strPath, CFile::modeRead|CFile::shareDenyNone, NULL ) == FALSE){
		goto __end;
	}
	
	FileLen = file.GetLength();
	if (FileLen < nCharCount){
		m_strErrMsg.Format("Sn size is not correct, expect size = %d, real size = %d", nExpectSize, FileLen);
		goto __end;
	}
	FileLen+=1;
	pTemp = new BYTE[FileLen];
	if (pTemp == NULL){
		goto __end;
	}
	memset(pTemp, 0, FileLen);

	//file.Read(pTemp, FileLen - 1);
	file.Read(pTemp, nCharCount); //只读取设定的几个字节

	strSN.Format("%s", pTemp);
	if(m_SNInfo.SNByteMode==BYTEMODE_BE){
		ComTool::Str2HexNoTruncate(strSN,ComTool::ENDIAN_BIG,vSNValue);
	}
	else{
		ComTool::Str2HexNoTruncate(strSN,ComTool::ENDIAN_LIT,vSNValue);
	}
	
	if (vSNValue.size() != nExpectSize ){
		AfxMessageBox(" the size of file is not  expect");
		goto __end;
	}
	copy(vSNValue.begin(), vSNValue.end(), pTemp);
	
	memcpy(*pSN, pTemp, nExpectSize);	

	nRtn = TRUE;

__end:

	SAFEDELARRAY(pTemp);

	return nRtn;
}

INT CDlgSNFile::GetPrintContent(CString strCurRow, CString& strPrint)
{
	INT Rtn = FALSE;
	int nPos = 0;
	int nEndPos = 0;
	CString strTmp;
	if (strCurRow.IsEmpty()){
		goto __end;
	}
	strCurRow.Replace(" ", "");
	
	nPos = strCurRow.Find("<P:\"", 0);
	if (nPos == -1){
		goto __end;
	}
	strCurRow.Delete(0, nPos  + 4);
	nEndPos = strCurRow.Find("\">", 0);
	if (nEndPos == -1){
		goto __end;
	}

	strPrint.Format("%s", strCurRow.Left(nEndPos));
	Rtn = TRUE;

__end:
	return Rtn;
}

INT CDlgSNFile::FreeSNGroups(std::vector<tSNOneGroup> &vSNGroups)
{
	INT i;
	for(i=0;i<(INT)vSNGroups.size();i++){
		if(vSNGroups[i].pData){
			sn_free(vSNGroups[i].pData);
			vSNGroups[i].pData=NULL;
			vSNGroups[i].SNSize=0;
		}
	}
	vSNGroups.clear();
	return 0;
}

/**
函数说明：获取SN的特性，在界面信息更新给DLL之后调用该函数获取SN的特性
参数说明：
strJson: Json字符串
Size： strJson可以使用的字节数
返回值：
0表示成功，负数表示失败
**/
INT CDlgSNFile::GetSNFeature(const char*strJsonIn, char* strJsonOut, INT SNSize)
{
	//std::string strJson;
	//Json::StyledWriter JWriter;
	//Json::Value Root;
	
	INT CountTemp = 0,CountIndex = 0,CountEnd = 0,CountTempTwo = 0;
	INT SNCountCanBeUse;//SN 序列号大小
	std::string strBuildJson;
	
	CString strTmp,strTempTwo;
	m_SNFile.Seek(0,CFile::begin);
	while(1){
		if(m_SNFile.ReadString(strTmp)==FALSE){
			break;
		}
		CountTemp++;
		if(strTmp=="[SN Info]"){
			break;
		}
	}

	m_SNFile.Seek(0,CFile::begin);
	while(1){
		if(m_SNFile.ReadString(strTempTwo)==FALSE){
			break;
		}
		CountIndex++;
		if(strTempTwo=="[SN Info]"){
			CountTempTwo = CountIndex;
		}

		if(CountTempTwo >= CountTemp){
			if(strTempTwo.IsEmpty()){
				CountEnd = CountIndex - 1;
				break;
			}
			strTempTwo.Trim();
			if(strTempTwo=="END"){
				CountEnd = CountIndex - 1;
				break;
			}
		}
	}
	SNCountCanBeUse = CountEnd - CountTemp;

	cJSON* RootBuild = cJSON_CreateObject();
	cJSON_AddNumberToObject(RootBuild, "SNCountCanBeUse", SNCountCanBeUse);

	strBuildJson = cJSON_Print(RootBuild);
	if(strBuildJson.size() < SNSize){
		memset(strJsonOut, 0, SNSize);
		strncpy(strJsonOut, strBuildJson.c_str(), strBuildJson.size());
	}else {return -1;}

	return 0;	
}


/**
parser configure line 

return:
-1 :failed
0: pass
1: not match idx
**/
INT CDlgSNFile::GetSNGroups(UINT& SNIdx,std::vector<tSNOneGroup> &vSNGroups,BYTE* buf,INT len, CString& strPrint)
{
	int i=0,ret=0,j=0;
	int offset=0;
	int reallen=0;
	UINT idx=0;
	UINT64 addr=0;
	UINT size=0;
	BYTE*snv=NULL;
	int snv_len=0;
	int group=1;
	tSNOneGroup SNGroupTmp;

	CString strFileName;
	BYTE pFileName[512] = {0};
	CString strTemp;
	CString strOneRow;
	strOneRow.Format("%s", buf);
	BYTE* pTemp = NULL; 
	CString strDelete;

	reallen=Trimspace(buf,len); ///trim the space first
	ret = GetSNIndex(&idx,buf,&offset);
	if(ret!=0){
		if(ret==-1)
			m_strErrMsg.Format("Line[%d] error, Please start label with \"<L:\"",m_SNInfo.LineIndex);
		else 
			m_strErrMsg.Format("Line[%d] error, Nn label has invalid character",m_SNInfo.LineIndex);
		goto __end;
	}

	if(idx==0 || idx>99999999){
		if(idx==0)
			m_strErrMsg.Format("Line[%d] error, sn label=0, please start with 1",m_SNInfo.LineIndex);
		else
			m_strErrMsg.Format("Line[%d] error, sn label>99999999 is not allowed",m_SNInfo.LineIndex);
		ret =-1;
		return ret;
	}
	SNIdx=idx;

__next:	
	if (GetPrintContent(strOneRow, strPrint) == TRUE){
		strDelete.Format("<P:\"%s\">",strPrint );
		strOneRow.Replace(strDelete, "");
		memset(buf, 0, len);
		memcpy(buf, strOneRow.GetBuffer(), strOneRow.GetLength());
	}
	
	memset(&SNGroupTmp,0,sizeof(tSNOneGroup));
	
	
	ret = GetSNAddr(&addr,buf,&offset);
	if(ret!=0){
		if(ret==-1){
			m_strErrMsg.Format("Line[%d] error, Can't find \"<\" when parser address",m_SNInfo.LineIndex);
		}
		else if (ret==-2){
			m_strErrMsg.Format("Line[%d] error, Sn addr had invalid character",m_SNInfo.LineIndex);
		}
		goto __end;
	}
	else{ 
		SNGroupTmp.SNAddr=addr;
	}
		
	ret = GetSNSize(&size,buf,&offset);
	
	if(ret!=0){
		if(ret==-1){
			m_strErrMsg.Format("Line[%d] error, Can't find \"<\" when parser size",m_SNInfo.LineIndex);
		}
		else if (ret==-2){
			m_strErrMsg.Format("Line[%d] error, Sn size had invalid character",m_SNInfo.LineIndex);
		}
		goto __end;
	}

	ret = GetSNValue(&snv,&snv_len,buf,&offset);
		
	if (m_SNInfo.SNDataMode== DATAMODE_MIX){
		strTemp.Format("%s", snv);
		int nRet=GetCurRowFormat(strTemp);

		if ( nRet == 0){
		}else if ( nRet == 1){	
			if ( ret != 0){
				goto __end;
			}
			memset(pFileName, 0, 512);
			memcpy(pFileName,snv,  snv_len);
			strFileName.Format("%s", pFileName/*snv*/);

			GetDataFromFile(strFileName, &snv, size);
			snv_len = size;
		}
	}else if (m_SNInfo.SNDataMode== DATAMODE_FILE){
		if ( ret != 0){
			goto __end;
		}

		memset(pFileName, 0, 512);
		memcpy(pFileName,snv,  snv_len);
		strFileName.Format("%s", pFileName/*snv*/);
		
		if (GetDataFromFile(strFileName, &snv, size) != TRUE){
			//ret = -2;goto __end; //让其出错提示
		}
		snv_len = size; //只copy目的长度
	}
	
	if(ret!=0)
		goto __end;
	else{
		SNGroupTmp.pData=snv;
		SNGroupTmp.SNSize=snv_len;
	}

	if( m_SNInfo.SNDataMode== DATAMODE_DATA ){
		if(SNGroupTmp.SNSize!=size){
			m_strErrMsg.Format("Line[%d] error,group[%d] size not match,you fill %d bytes,but actual %d bytes",(UINT)m_SNInfo.LineIndex,group,size,SNGroupTmp.SNSize);
			ret = -1;
			goto __end;
		}
	}
	
	vSNGroups.push_back(SNGroupTmp);
	/****
	skip the sparator, space or comma
	****/
	while(buf[offset]==0x20 || buf[offset]==0x2c){
		offset++;
	}

	if(buf[offset]=='<'){///find the second group sn value
		group ++;
		goto __next;
	}
	
__end:

	SAFEDELARRAY(pTemp);

	if(ret!=0){
		FreeSNGroups(vSNGroups);
	}
	else{
	}
	return ret;
}
/**************
QuerySN:查询SN
Idx：   请求查询的SN的索引值，该值大于等于1,
pData： 内部将查询得到的SN的信息放到该buffer中
pSize:     pData指向的Buffer的大小
返回值：
 -1： 查询失败
 -2:  pData的内存太小，需要重新指定，pSize返回真正需要的大小。
 >=0:  实际填充的pData的大小。
************************/

INT CDlgSNFile::QuerySN( DWORD Idx,BYTE*pData,INT*pSize )
{
	INT Ret=0;
	CString strPrint;
	std::vector<tSNOneGroup> vSNGroups;
	if ( m_SNInfo.SNInfoStartPos ==0 ){
		Ret=-1; goto __end;
	}

	//if (true)
	//{
	//	QueryPrint(Idx, pData, pSize);
	//	return 10;
	//}

	if (m_SNInfo.SNDataMode == DATAMODE_DATA){
		CString strTmp;
		CSerial lSerial;
		UINT SnIdx=0;
		m_SNInfo.LineIndex=m_SNInfo.SNInfoStartLine+Idx;
		if(PathFileExists((LPCTSTR)m_strFile)==FALSE){
			Ret=-1; goto __end;
		}
		
		m_SNFile.Seek(m_SNInfo.SNInfoStartPos+(Idx-1)*m_SNInfo.RecordSize,CFile::begin);
		if(m_SNFile.ReadString(strTmp)==FALSE){
			Ret=-1; goto __end;
		}

		if(strTmp=="" || strTmp=="END"){
			Ret=-1; goto __end;
		}

		Ret=GetSNGroups(SnIdx,vSNGroups,(BYTE*)(LPCSTR)strTmp,strTmp.GetLength(), strPrint);
		if(Ret==-1){
			AfxMessageBox(m_strErrMsg,MB_OK|MB_ICONERROR);
			goto __end;
		}
		
		UINT GroupCnt=(UINT)vSNGroups.size();
		try{
			lSerial<<GroupCnt;
			for(int i=0;i<GroupCnt;i++){
				lSerial<<vSNGroups[i].SNAddr<<vSNGroups[i].SNSize;
				lSerial.SerialInBuff(vSNGroups[i].pData,vSNGroups[i].SNSize);
			}
			if(*pSize<(INT)lSerial.GetLength()){
				Ret=-2;
			}
			else{
				Ret=lSerial.GetLength();
				lSerial.SerialOutBuff(pData,lSerial.GetLength());
			}
		}
		catch (...){
			Ret=-1;
		}	
	}
	else if ( m_SNInfo.SNDataMode == DATAMODE_FILE ||  m_SNInfo.SNDataMode == DATAMODE_MIX ){
		if ( Idx >m_vRecordIdxMap.size()){
			AfxMessageBox("there is no more sn, please check the total");
			goto __end;
		}

		tSNInfo itemSN = m_vRecordIdxMap[Idx-1];

		UINT SnIdx=0;
		CSerial lSerial;

		Ret=GetSNGroups(SnIdx,vSNGroups,(BYTE*)(LPCSTR)itemSN.strCurRecord ,itemSN.strCurRecord.GetLength(), strPrint);
		if(Ret==-1){
			goto __end;
		}

		UINT GroupCnt=(UINT)vSNGroups.size();
		try{
			lSerial<<GroupCnt;
			for(int i=0;i<GroupCnt;i++){
				lSerial<<vSNGroups[i].SNAddr<<vSNGroups[i].SNSize;
				lSerial.SerialInBuff(vSNGroups[i].pData,vSNGroups[i].SNSize);
			}
			if(*pSize<(INT)lSerial.GetLength()){
				Ret=-2;
			}
			else{
				Ret=lSerial.GetLength();
				lSerial.SerialOutBuff(pData,lSerial.GetLength());
			}
		}
		catch (...){
			Ret=-1;
		}	
		
	}
__end:
	FreeSNGroups(vSNGroups);
	return Ret;
}


////////////////////////获取print内容///////////
INT CDlgSNFile::QueryPrint(DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	CString strPrint;
	std::vector<tSNOneGroup> vSNGroups;
	if ( m_SNInfo.SNInfoStartPos ==0 ){
		Ret=-1; goto __end;
	}

	if (m_SNInfo.SNDataMode == DATAMODE_DATA){
		CString strTmp;
		CSerial lSerial;
		UINT SnIdx=0;
		m_SNInfo.LineIndex=m_SNInfo.SNInfoStartLine+Idx;
		if(PathFileExists((LPCTSTR)m_strFile)==FALSE){
			Ret=-1; goto __end;
		}
		m_SNFile.Seek(m_SNInfo.SNInfoStartPos+(Idx-1)*m_SNInfo.RecordSize,CFile::begin);
		if(m_SNFile.ReadString(strTmp)==FALSE){
			Ret=-1; goto __end;
		}

		if(strTmp=="" || strTmp=="END"){
			Ret=-1; goto __end;
		}

		Ret=GetSNGroups(SnIdx,vSNGroups,(BYTE*)(LPCSTR)strTmp,strTmp.GetLength(), strPrint);
		if(Ret==-1){
			AfxMessageBox(m_strErrMsg,MB_OK|MB_ICONERROR);
			goto __end;
		}

		UINT GroupCnt=(UINT)vSNGroups.size();
		try{
			lSerial<<strPrint;
			/*lSerial<<GroupCnt;
			for(int i=0;i<GroupCnt;i++){
				lSerial<<vSNGroups[i].SNAddr<<vSNGroups[i].SNSize;
				lSerial.SerialInBuff(vSNGroups[i].pData,vSNGroups[i].SNSize);
			}*/
			if(*pSize<(INT)lSerial.GetLength()){
				Ret=-2;
			}
			else{
				Ret=lSerial.GetLength();
				lSerial.SerialOutBuff(pData,lSerial.GetLength());
			}
		}
		catch (...){
			Ret=-1;
		}	
	}
	else if ( m_SNInfo.SNDataMode == DATAMODE_FILE ||  m_SNInfo.SNDataMode == DATAMODE_MIX ){
		if ( Idx >m_vRecordIdxMap.size()){
			AfxMessageBox("there is no more sn, please check the total");
			goto __end;
		}

		tSNInfo itemSN = m_vRecordIdxMap[Idx-1];

		UINT SnIdx=0;
		CSerial lSerial;

		Ret=GetSNGroups(SnIdx,vSNGroups,(BYTE*)(LPCSTR)itemSN.strCurRecord ,itemSN.strCurRecord.GetLength(), strPrint);
		if(Ret==-1){
			goto __end;
		}

		UINT GroupCnt=(UINT)vSNGroups.size();
		try{
			lSerial<<strPrint;
			/*lSerial<<GroupCnt;
			for(int i=0;i<GroupCnt;i++){
				lSerial<<vSNGroups[i].SNAddr<<vSNGroups[i].SNSize;
				lSerial.SerialInBuff(vSNGroups[i].pData,vSNGroups[i].SNSize);
			}*/
			if(*pSize<(INT)lSerial.GetLength()){
				Ret=-2;
			}
			else{
				Ret=lSerial.GetLength();
				lSerial.SerialOutBuff(pData,lSerial.GetLength());
			}
		}
		catch (...){
			Ret=-1;
		}	

	}
__end:
	FreeSNGroups(vSNGroups);
	return Ret;
}
///////////////////////

void CDlgSNFile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SNFILE, m_strFile);
	DDX_Text(pDX, IDC_EDITCHIPNAME, m_ChipName);
	DDX_Text(pDX, IDC_EDITBYTEMODE, m_ByteMode);
	DDX_Text(pDX, IDC_EDITDATAMODE, m_DataMode);
}


BEGIN_MESSAGE_MAP(CDlgSNFile, CDialog)
	ON_BN_CLICKED(IDC_BTNSNFILESEL, &CDlgSNFile::OnBnClickedBtnsnfilesel)
END_MESSAGE_MAP()


// CDlgSNFile 消息处理程序

void CDlgSNFile::OnBnClickedBtnsnfilesel()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strFilePath;
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_OVERWRITEPROMPT, "SER File(*.ser)|*.ser||");
	if (dlgFile.DoModal() != IDOK){
		return;
	}
	m_strFile=dlgFile.GetPathName();

	if(OpenSNSer()==0){
		if(GetSNSerChipInfo()==0)
			PrepareSNInfo();
			BuildIdxMap();
	}

	UpdateData(FALSE);
}

INT CDlgSNFile::CloseSNSer()
{
	if(m_SNFile.m_hFile!=CFile::hFileNull){
		m_SNFile.Close();
	}
	return 0;
}

INT CDlgSNFile::OpenSNSer()
{
	INT Ret=0;
	if(m_strFile.IsEmpty()){
		AfxMessageBox("Please select a ser file first",MB_OK|MB_ICONERROR);
		Ret=-1; goto __end;
	}
	//memset(&m_SNInfo,0,sizeof(tSNInfo));
	if(m_SNFile.Open(m_strFile, CFile::modeRead | CFile::shareDenyWrite, NULL) == FALSE){
		AfxMessageBox("Open ser file failed",MB_OK|MB_ICONERROR);
		Ret=-1; goto __end;
	}
	
__end:
	return Ret;
}

INT CDlgSNFile::BuildIdxMap()
{
	INT Rtn = FALSE;
	int nIdx = 0;
	UINT64 uStartPos = 0;
	CString strOneRow;
	if (m_SNInfo.SNInfoStartPos == 0){
		goto __end;
	}
	
	uStartPos = m_SNInfo.SNInfoStartPos ;
	m_SNFile.Seek(m_SNInfo.SNInfoStartPos,CFile::begin);//定位到内容开始的地方
	while(m_SNFile.ReadString(strOneRow) == TRUE){
		if (strOneRow.CompareNoCase("END") == 0){
			break;
		}
		tSNInfo itemSNInfo;
		itemSNInfo.LineIndex = nIdx;
		itemSNInfo.SNInfoStartPos = uStartPos;
		itemSNInfo.strCurRecord.Format("%s", strOneRow);
		itemSNInfo.RecordSize = strOneRow.GetLength();
		m_vRecordIdxMap.push_back(itemSNInfo);

		nIdx++;
		uStartPos = m_SNFile.GetPosition();
	}

__end:
	Rtn = TRUE;
	return Rtn;
}

INT CDlgSNFile::PrepareSNInfo()
{
	INT Ret=0;
	BOOL bSNInfoFind=FALSE;
	BYTE *pData=NULL;

	m_vRecordIdxMap.clear();

	if(m_SNFile.m_hFile!=CFile::hFileNull){
		CString strTmp;
		while(1){
			if(m_SNFile.ReadString(strTmp)==FALSE){
				break;
			}
			m_SNInfo.LineIndex++;
			if(strTmp=="[SN Info]"){
				bSNInfoFind=TRUE;
				m_SNInfo.SNInfoStartLine=m_SNInfo.LineIndex;
				m_SNInfo.SNInfoStartPos=m_SNFile.GetPosition();
				m_SNFile.ReadString(strTmp);
				m_SNInfo.RecordSize=strTmp.GetLength();///这个地方没有加上回车换行，具体回车换行是2个字节还是只有一个0x0D,需要确认一下
				pData=new BYTE[m_SNInfo.RecordSize+2];
				if(!pData){
					AfxMessageBox("Memory alloc failed when finding [SN Info]");
					Ret=-1; goto __end;
				}
				memset(pData,0,m_SNInfo.RecordSize+2);
				m_SNFile.Seek(m_SNInfo.SNInfoStartPos,CFile::begin);//在回到这个起点
				m_SNFile.Read(pData,m_SNInfo.RecordSize+2);
				if(pData[m_SNInfo.RecordSize+1]==0x0D){///需要加上回车换行
					m_SNInfo.RecordSize +=1;
				}
				else{
					m_SNInfo.RecordSize +=2;
				}
				m_SNFile.Seek(m_SNInfo.SNInfoStartPos,CFile::begin);//在回到这个起点,准备为后面获取SN
				break;
			}
		}
	}
	if(bSNInfoFind==FALSE){
		AfxMessageBox("Can't find [SN Info], Please check the file format");
		Ret=-1;
	}
__end:
	if(pData){
		delete[] pData;
	}
	return Ret;
}

INT CDlgSNFile::GetSNSerChipInfo()
{
	INT Ret=0;
	INT ChipInfoFindState=0;
	BOOL bChipNameFind=FALSE;
	BOOL bByteModeFind=FALSE;
	BOOL bDataModeFind=FALSE;
	if(m_SNFile.m_hFile!=CFile::hFileNull){
		CString strTmp;
		m_SNFile.Seek(0,CFile::begin);
		while(1){
			if(m_SNFile.ReadString(strTmp)==FALSE){
				break;
			}
			m_SNInfo.LineIndex++;
			if(strTmp=="[Chip Info]"){
				ChipInfoFindState=1;
			}
			else{
				if(ChipInfoFindState==1){
					INT Pos;
					if(bChipNameFind==FALSE){
						Pos=strTmp.Find("ChipName:",0);
						if(Pos>=0){
							m_ChipName=strTmp.Mid(Pos+(INT)strlen("ChipName:"));
							bChipNameFind=TRUE;
						}
					}
					if(bByteModeFind==FALSE){
						Pos=strTmp.Find("ByteMode:",0);
						if(Pos>=0){
							m_ByteMode=strTmp.Mid(Pos+(INT)strlen("ByteMode:"));
							bByteModeFind=TRUE;
							if(m_ByteMode=="LE"){
								m_SNInfo.SNByteMode=BYTEMODE_LE;
							}
							else{
								m_SNInfo.SNByteMode=BYTEMODE_BE;
							}
						}
					}
					if(bDataModeFind==FALSE){
						Pos=strTmp.Find("DataMode:",0);
						if(Pos>=0){
							m_DataMode=strTmp.Mid(Pos+(INT)strlen("DataMode:"));
							bDataModeFind=TRUE;
							if(m_DataMode.CompareNoCase("File") == 0){
								m_SNInfo.SNDataMode=DATAMODE_FILE;
							}
							else if (m_DataMode.CompareNoCase("Mix") == 0){
								m_SNInfo.SNDataMode=DATAMODE_MIX;
							}
							else{
								m_SNInfo.SNDataMode=DATAMODE_DATA;
							}
						}
					}

					if(bDataModeFind&&bChipNameFind&&bByteModeFind){
						ChipInfoFindState=2;
						break;
					}
				}
			}
		}
		if(ChipInfoFindState !=2){
			AfxMessageBox("Get ChipInfo in the ser file failed, Please check whether the format is correct",MB_OK|MB_ICONERROR);
			Ret=-1;
		}
	}
	else{
		Ret=-1;
	}

	if(Ret==0 && m_SNInfo.SNDataMode==DATAMODE_FILE){
		//AfxMessageBox("DataMode=File Not support yet",MB_OK|MB_ICONERROR);
		//Ret=-1;
	}
	return Ret;
}

