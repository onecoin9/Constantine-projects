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

INT CCSVFile::SeparatePara(CString&OneLine,CString&strSecCol, CString&strThirdCol)
{
	INT Ret=0;
	try{
		int Pos=0;
		Pos=OneLine.Find(',',0);
		if(Pos<=0){
			m_strErrMsg.Format("It's an invalid line, Content:%s",OneLine);
			Ret=-1; goto __end;
		}
		CString strTempOne;
		strTempOne.Format("%s", OneLine);

		strTempOne.Delete(0, Pos+1);
		
		//deviceId
		Pos = strTempOne.Find(',', 0);
		if(Pos<=0){
			m_strErrMsg.Format("It's an invalid line, Content:%s",OneLine);
			Ret=-1; goto __end;
		}
		strSecCol.Format("%s", strTempOne.Left(Pos));
		strTempOne.Delete(0, Pos+1);

		//connPublicKey
		Pos = strTempOne.Find(',', 0);
		if(Pos<=0){
			m_strErrMsg.Format("It's an invalid line, Content:%s",OneLine);
			Ret=-1; goto __end;
		}
		strThirdCol.Format("%s", strTempOne.Left(Pos));
		strTempOne.Delete(0, Pos+1);
	}
	catch (...){
		m_strErrMsg.Format("Catch An Exception, Content:%s",OneLine);
		Ret=-1; goto __end;
	}
__end:
	return Ret;
}

INT CCSVFile::ReadSN(UINT Index,CString&strSecCol, CString&strThirdCol)
{
	INT Ret=0;
	CString strOneLine;
	Seek(m_PosStart+(Index-1)*m_EachLineSize,CFile::begin);
	if(ReadString(strOneLine)==FALSE){
		m_strErrMsg.Format("Read File Failed: Read OneLine Fail");
		Ret=-1; goto __end;
	}

	Ret=SeparatePara(strOneLine,strSecCol, strThirdCol);
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
	CString strOneLine,strFstCol,strSecCol, strThirdCol;
	m_IsReady=FALSE;
	if(Open(strFilePath,CFile::modeRead|CFile::shareDenyNone,NULL)==FALSE){
		m_strErrMsg.Format("Open File Failed: %s",strFilePath);
		Ret=-1; goto __end;
	}
	
	if(ReadString(strOneLine)==FALSE){
		m_strErrMsg.Format("Read File Failed: Read First Line Failed");
		Ret=-1; goto __end;
	}
	
	Ret=SeparatePara(strOneLine,strSecCol, strThirdCol);
	if(Ret!=0){
		goto __end;
	}

	if(strSecCol.CompareNoCase("deviceId")==0){
		///Fix the
		m_PosStart=GetPosition();
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
