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

INT CCSVFile::SeparatePara(CString&OneLine,CString&strFstCol,CString&strSecCol)
{
	INT Ret=0;
	try{
		int Pos=0;
		Pos=OneLine.Find(',',0);
		if(Pos<=0){
			m_strErrMsg.Format("It's an invalid line, Content:%s",OneLine);
			Ret=-1; goto __end;
		}
		strFstCol=OneLine.Mid(0,Pos);
		strSecCol=OneLine.Mid(Pos+1,OneLine.GetLength());
	}
	catch (...){
		m_strErrMsg.Format("Catch An Exception, Content:%s",OneLine);
		Ret=-1; goto __end;
	}
__end:
	return Ret;
}

INT CCSVFile::ReadSN(UINT Index,CString&strFstCol,CString&strSecCol)
{
	INT Ret=0;
	CString strOneLine;
	Seek(m_PosStart+(Index-1)*m_EachLineSize,CFile::begin);
	if(ReadString(strOneLine)==FALSE){
		m_strErrMsg.Format("Read File Failed: Read OneLine Fail");
		Ret=-1; goto __end;
	}

	Ret=SeparatePara(strOneLine,strFstCol,strSecCol);
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
	CString strOneLine,strFstCol,strSecCol;
	m_IsReady=FALSE;
	if(Open(strFilePath,CFile::modeRead|CFile::shareDenyNone,NULL)==FALSE){
		m_strErrMsg.Format("Open File Failed: %s",strFilePath);
		Ret=-1; goto __end;
	}
	
	if(ReadString(strOneLine)==FALSE){
		m_strErrMsg.Format("Read File Failed: Read First Line Failed");
		Ret=-1; goto __end;
	}
	
	Ret=SeparatePara(strOneLine,strFstCol,strSecCol);
	if(Ret!=0){
		goto __end;
	}

	if(strSecCol.CompareNoCase("secret")==0){
		///Fix the
		m_PosStart=GetPosition() ;
		m_IsReady=TRUE;

		if(ReadString(strOneLine)==FALSE){
			m_strErrMsg.Format("Read File Failed: Read OneLine Fail");
			Ret=-1; goto __end;
		}

		m_EachLineSize=GetPosition()-m_PosStart;
	}
	else{
		m_strErrMsg.Format("First Line is invalid, Content: %s",strOneLine);
		Ret=-1; goto __end;
	}
__end:
	return Ret;
}
