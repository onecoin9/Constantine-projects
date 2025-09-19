#include "stdafx.h"
#include "FileSearcher.h"
#include <algorithm>
#include "../Com/ComFunc.h"

CFileSearcher::CFileSearcher(void)
{
}

CFileSearcher::~CFileSearcher(void)
{
}

void CFileSearcher::Sort()
{
	std::sort(v_Files.begin(),v_Files.end());
}

INT CFileSearcher::GetListSize()
{
	return (INT)v_Files.size();
}

CString CFileSearcher::GetIdx(INT idx)
{
	if(idx>=v_Files.size()){
		return CString("");
	}
	return v_Files[idx];
}

/*!
 * @brief: 查找文件
 *
 * @param[in]: dir 搜索的目录
 * @param[in]: Size 指定的大小，字节为单位，与该Size相等的文件会被保存到列表中
 * @param[in]：strExtName 文件的后缀名
 * @return
 *  TRUE 表示成功， FALSE表示失败
 *
 */
#if 0
BOOL CFileSearcher::ScanFiles(CString& dir,UINT Size,CString strExtName)
{
	CFileFind ff;
	if (dir.Right(1) != "\\")
		dir += "\\";
	dir += "*.*";
	BOOL ret = ff.FindFile(dir);
	v_Files.clear();
	while (ret){
		ret = ff.FindNextFile();
		if (ret != 0){
			if (ff.IsDirectory() && !ff.IsDots()){///子文件夹，不要进行遍历
				//CString path = ff.GetFilePath();
				//ScanFiles(path,Size,strExtName);
			}
			else if (!ff.IsDirectory() && !ff.IsDots()){
				CString name = ff.GetFileName();
				CString path = ff.GetFilePath();
				CString strExt= CComFunc::GetFileExt(path);
				if(Size>0 && ff.GetLength()==Size && strExt.CompareNoCase(strExtName)==0 ){//符合条件大小的才能进入
					v_Files.push_back(path);
				}
			}
		}
		
	}
	return TRUE;
}

#else
BOOL CFileSearcher::ScanFiles(CString& dir,UINT Size,CString strExtName,BOOL Var)
{
	CFileFind ff;
	HANDLE file;
	CString path;
	CString strMsg;
	CString strSerchDir=dir;
	CString strSNFileDir=dir;
	WIN32_FIND_DATA pNextInfo;
	if (strSerchDir.Right(1) != "\\")
		strSerchDir += "\\";
	strSerchDir += "*.*";
	v_Files.clear();
	file=FindFirstFile(strSerchDir, &pNextInfo);
	if (file == INVALID_HANDLE_VALUE){
		return FALSE;
	}
	while (FindNextFile(file, &pNextInfo)){
		if (pNextInfo.cFileName[0] == '.')
			continue;
		path.Format("%s\\%s",strSNFileDir,pNextInfo.cFileName);
		CString strExt= CComFunc::GetFileExt(path);
		if (Var){
			if(Size>0 && pNextInfo.nFileSizeLow==Size && strExt.CompareNoCase(strExtName)==0 ){//MaxSize为空则判断大小是否一致
				v_Files.push_back(path);
			}
		}else{
			if(Size>0 && strExt.CompareNoCase(strExtName)==0 ){
				v_Files.push_back(path);
			}
		}
		
	}
	return TRUE;
}

#endif 
