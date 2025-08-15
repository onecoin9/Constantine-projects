#include "CSVFile.h"

CCSVFile::CCSVFile(void)
:m_IsReady(FALSE)
,m_PosStart(0)
,m_strErrMsg("")
{
}

CCSVFile::~CCSVFile(void)
{
}

typedef unsigned char INT8U;
#define isDigitalChar(a)		(((a)<='9' && (a)>='0')?1:0)
#define isUpperChar(a)			(((a)>='A' && (a)<='F')?1:0)
#define isLowerChar(a)			(((a)>='a' && (a)<='f')?1:0)
#define isAlphaHexChar(a)		((isUpperChar(a) || isLowerChar(a))?1:0)
#define isHexChar(a)			((isDigitalChar(a) || isAlphaHexChar(a))?1:0)

static inline void Split(CString source, CStringArray& dest, CString division)
{
	dest.RemoveAll();
	int pos = 0;
	while( -1 != pos ){
		CString tmp = source.Tokenize(division, pos);
		if (!tmp.IsEmpty()){
			dest.Add(tmp);
		}
	}
}


static inline int _puthigh(INT8U *place,INT8U data)
{
	int ret=0;
	if(isDigitalChar(data)){///get high 4 bits
		*place |= (data-'0')<<4;
	}
	else if(isAlphaHexChar(data)){
		if(data>='a'){
			*place |= (data-'a'+0x0A)<<4;
		}
		else{
			*place |= (data-'A'+0x0A)<<4;
		}
	}
	else{
		ret = -1;
	}
	return ret;
}

static inline int _putlow(INT8U*place,INT8U data)
{
	int ret=0;
	if(isDigitalChar(data)){///get low 4 bits
		*place |= data-'0';
	}
	else if(isAlphaHexChar(data)){
		if(data>='a'){
			*place |= data-'a'+0x0A;
		}
		else{
			*place |= data-'A'+0x0A;
		}
	}
	else{
		ret = -1;
	}
	return ret;
}

static inline int Str2HexArray(const UCHAR *str,UCHAR *hex,int size)
{
	size_t reallen=0,buf_len=0;
	size_t i,ret=0;
	size_t iseven=0;

	reallen=strlen((char*)str);
	buf_len=(reallen+1)/2;
	if((size_t)size<buf_len){
		return -2;
	}
	iseven=(reallen%2)?0:1;

	memset(hex,0,size);

	if(iseven){///the length is even
		for(i=0;i<reallen;i++){
			if(i%2==0){
				ret =_puthigh(hex+i/2,str[i]);
				if(ret==-1)
					goto __end;
			}
			else{
				ret = _putlow(hex+i/2,str[i]);
				if(ret==-1)
					goto __end;
			}
		}
	}
	else{
		for(i=0;i<reallen;i++){///add zero at head
			if(i%2==0){
				ret = _putlow(hex+(i+1)/2,str[i]);
				if(ret==-1)
					goto __end;
			}
			else{
				ret =_puthigh(hex+(i+1)/2,str[i]);
				if(ret==-1)
					goto __end;
			}
		}
	}

__end:
	if(ret!=0){
		return -1;
	}
	return (int)buf_len;
}

INT CCSVFile::SeparatePara(CString&OneLine,CStringArray &strArrayCol)
{
	INT Ret=0;
	try{
		Split(OneLine, strArrayCol, ",");
		if (strArrayCol.GetCount() != CSV_COL_NUM){
			m_strErrMsg.Format("Catch An Exception, Content:%s",OneLine);
			Ret = -1; goto __end;
		}
	}
	catch (...){
		m_strErrMsg.Format("Catch An Exception, Content:%s",OneLine);
		Ret=-1; goto __end;
	}
__end:
	return Ret;
}



INT CCSVFile::ReadSN(UINT Index, tParamCol &ParamCol, INT &nTotalLen)
{
	INT Ret=0;

	CString strOneLine;
	int convertLen = 0;
	CStringArray strArrayCol;

	Seek(m_PosStart+(Index-1)*m_EachLineSize,CFile::begin);
	if(ReadString(strOneLine)==FALSE){
		m_strErrMsg.Format("Read File Failed: Read OneLine Fail");
		Ret=-1; goto __end;
	}

	Ret=SeparatePara(strOneLine, strArrayCol);
	if (Ret != 0){
		m_strErrMsg.Format("SeparatePara error, Content:%s",strOneLine);
		Ret = -1; goto __end;
	}

	ParamCol.strProdKeyCol = strArrayCol[0];
	nTotalLen += strArrayCol[0].GetLength();

	ParamCol.strDevNameCol = strArrayCol[1];
	nTotalLen += strArrayCol[1].GetLength();

	ParamCol.strDevSecretCol = strArrayCol[2];
	nTotalLen += strArrayCol[2].GetLength();

	if(Ret!=0){
		goto __end;
	}

__end:
	return Ret;
}

INT CCSVFile::CloseFile()
{
	if(CFile::hFileNull!=m_hFile){
		Close();
	}
	m_IsReady=FALSE;
	m_PosStart=0;
	m_strErrMsg="";
	m_EachLineSize=0;
	return 0;
}

INT CCSVFile::OpenFile(CString strFilePath)
{
	int Ret=0;
	CString strOneLine;
	CStringArray strArrayCol;
	CString strDevNameCol,strDevSecretCol, strProdKeyCol;
	m_IsReady=FALSE;
	if(Open(strFilePath,CFile::modeRead|CFile::shareDenyNone,NULL)==FALSE){
		m_strErrMsg.Format("Open File Failed: %s",strFilePath);
		Ret=-1; goto __end;
	}
	
	if(ReadString(strOneLine)==FALSE){
		m_strErrMsg.Format("Read File Failed: Read First Line Failed");
		Ret=-1; goto __end;
	}


	Ret=SeparatePara(strOneLine, strArrayCol);
	if(Ret!=0){
		goto __end;
	}

	strProdKeyCol = strArrayCol[0];
	strDevNameCol = strArrayCol[1];
	strDevSecretCol = strArrayCol[2];

	if(strDevSecretCol.CompareNoCase("DeviceSecret")==0 && strProdKeyCol.CompareNoCase("ProductKey")==0 &&
		strDevNameCol.CompareNoCase("DeviceName")==0){
		///Fix the
		m_PosStart=GetPosition() ;
		m_IsReady=TRUE;

		if(ReadString(strOneLine)==FALSE){
			m_strErrMsg.Format("Read File Failed: Read OneLine Fail");
			Ret=-1; goto __end;
		}

		m_EachLineSize = (UINT)(GetPosition()-m_PosStart);
	}
	else{
		m_strErrMsg.Format("First Line is invalid, Content: %s",strOneLine);
		Ret=-1; goto __end;
	}
__end:
	return Ret;
}
