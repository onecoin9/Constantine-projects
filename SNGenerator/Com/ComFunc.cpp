#include "ComFunc.h"

CString CComFunc::GetCurrentPath( void )
{
	TCHAR szFilePath[MAX_PATH + 1]; 
	TCHAR *pPos=NULL;
	CString str_url;
	GetModuleFileName(NULL, szFilePath, MAX_PATH); 
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

CString CComFunc::GetFileName(const CString &strFilePath)
{
	CString FileName="";
	INT Pot=strFilePath.ReverseFind('.');
	INT Slash=strFilePath.ReverseFind('\\');
	if(Slash!=-1 &&Pot!=-1){
		FileName=strFilePath.Mid(Slash+1,Pot-Slash-1);
	}
	else{
		if(Slash==-1){
			Slash=strFilePath.ReverseFind('/');
			if(Slash!=-1 &&Pot!=-1){
				FileName=strFilePath.Mid(Slash+1,Pot-Slash-1);
			}
		}
		else{
			FileName=strFilePath;///全部是文件名
		}
	}
	return FileName;
}

CString CComFunc::GetFileNameWithSuffix(const CString	& strFilePath)
{
	CString FileName="";
	INT Pot=strFilePath.ReverseFind('.');
	INT Slash=strFilePath.ReverseFind('\\');

	if(Slash!=-1 &&Pot!=-1){
		FileName=strFilePath.Mid(Slash+1,256);
	}
	else{
		Slash=strFilePath.ReverseFind('/');
		if(Slash!=-1 &&Pot!=-1){
			FileName=strFilePath.Mid(Slash+1,256);
		}
		else{	
			FileName=strFilePath;///全部是文件名
		}
	}
	return FileName;
}

///获取文件后缀名
CString CComFunc::GetFileExt(const CString &strFilePath)
{
	CString FileExt="";
	INT Pot=strFilePath.ReverseFind('.');;
	if(Pot!=-1){
		FileExt=strFilePath.Mid(Pot+1,strFilePath.GetLength());
	}
	return FileExt;
}

CString CComFunc::GetFilePath(const CString &strFileFullPath)
{
	CString FileName="";
	INT Pot=strFileFullPath.ReverseFind('.');
	INT Slash=strFileFullPath.ReverseFind('\\');
	if(Slash!=-1 &&Pot!=-1){
		FileName=strFileFullPath.Mid(0,Slash);
	}
	else{
		if(Slash==-1){
			Slash=strFileFullPath.ReverseFind('/');
			if(Slash!=-1 && Pot!=-1)
				FileName=strFileFullPath.Mid(0,Slash);
		}
	}
	return FileName;
}

void CComFunc::DeleteDir(CString str)
{
	CFileFind finder; //文件查找类
	CString strdel,strdir;//strdir:要删除的目录，strdel：要删除的文件
	strdir=str+"\\*";//删除文件夹，先要清空文件夹,加上路径,注意加"\\"
	BOOL b_finded=(BOOL)finder.FindFile(strdir); 
	while(b_finded) 
	{ 
		b_finded=(BOOL)finder.FindNextFile(); 
		if (finder.IsDots())  continue;//找到的是当前目录或上级目录则跳过
		strdel=finder.GetFileName(); //获取找到的文件名
		if(finder.IsDirectory())   //如果是文件夹
		{ 
			strdel=str + "\\" + strdel;//加上路径,注意加"\\"
			DeleteDir(strdel); //递归删除文件夹
		} 
		else //不是文件夹
		{ 
			strdel=str + "\\" + strdel;
			if(finder.IsReadOnly())//清除只读属性
			{    
				SetFileAttributes(strdel,GetFileAttributes(strdel)&(~FILE_ATTRIBUTE_READONLY));
			}
			DeleteFile(strdel); //删除文件(API)
		} 
	} 
	finder.Close(); 
	RemoveDirectory(str); //删除文件夹(API)
}