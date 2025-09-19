#include "ComFunc.h"

CString CComFunc::GetCurrentPath( void )
{
	TCHAR szFilePath[MAX_PATH + 1]; 
	TCHAR *pPos=NULL;
	CString str_url;
	GetModuleFileName(NULL, szFilePath, MAX_PATH); 
	pPos=_tcsrchr(szFilePath, _T('\\'));
	if(pPos!=NULL){
		pPos[0] = 0;//ɾ���ļ�����ֻ���·��
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
			FileName=strFilePath;///ȫ�����ļ���
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
			FileName=strFilePath;///ȫ�����ļ���
		}
	}
	return FileName;
}

///��ȡ�ļ���׺��
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
	CFileFind finder; //�ļ�������
	CString strdel,strdir;//strdir:Ҫɾ����Ŀ¼��strdel��Ҫɾ�����ļ�
	strdir=str+"\\*";//ɾ���ļ��У���Ҫ����ļ���,����·��,ע���"\\"
	BOOL b_finded=(BOOL)finder.FindFile(strdir); 
	while(b_finded) 
	{ 
		b_finded=(BOOL)finder.FindNextFile(); 
		if (finder.IsDots())  continue;//�ҵ����ǵ�ǰĿ¼���ϼ�Ŀ¼������
		strdel=finder.GetFileName(); //��ȡ�ҵ����ļ���
		if(finder.IsDirectory())   //������ļ���
		{ 
			strdel=str + "\\" + strdel;//����·��,ע���"\\"
			DeleteDir(strdel); //�ݹ�ɾ���ļ���
		} 
		else //�����ļ���
		{ 
			strdel=str + "\\" + strdel;
			if(finder.IsReadOnly())//���ֻ������
			{    
				SetFileAttributes(strdel,GetFileAttributes(strdel)&(~FILE_ATTRIBUTE_READONLY));
			}
			DeleteFile(strdel); //ɾ���ļ�(API)
		} 
	} 
	finder.Close(); 
	RemoveDirectory(str); //ɾ���ļ���(API)
}