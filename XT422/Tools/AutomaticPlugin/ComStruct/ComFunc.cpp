#include "ComFunc.h"
#include <windows.h>
#include <tchar.h>  
#include <iostream> 

static std::string TCHARToString(const TCHAR* tcharStr) {
#ifdef _UNICODE  
	// If TCHAR is wchar_t, convert to std::wstring first  
	std::wstring wstr(tcharStr);
	// Convert std::wstring to std::string (narrowing)  
	std::string str(wstr.begin(), wstr.end());
	return str;
#else  
	// If TCHAR is char, directly convert to std::string  
	return std::string(tcharStr);
#endif  
}

std::string CComFunc::GetCurrentPath( void )
{
	TCHAR szFilePath[MAX_PATH + 1]; 
	TCHAR*pPos=NULL;
	std::string str_url;
	GetModuleFileNameW(NULL, szFilePath, MAX_PATH); 
	pPos=_tcsrchr(szFilePath, _T('\\'));
	if(pPos!=NULL){
		pPos[0] = 0;//删除文件名，只获得路径
		str_url= TCHARToString(szFilePath);
	}
	else{
		pPos=_tcsrchr(szFilePath, _T('/'));
		if(pPos==NULL){
			str_url="";
		}
		else{
			str_url= TCHARToString(szFilePath);
		}
	}	
	return str_url;
}

std::string CComFunc::GetFileName(const std::string &strFilePath)
{
	std::string FileName="";
	int Pot=strFilePath.find_last_of('.');
	int Slash=strFilePath.find_last_of('\\');
	if(Slash!=-1 &&Pot!=-1){
		FileName=strFilePath.substr(Slash+1,Pot-Slash-1);
	}
	else{
		if(Slash==-1){
			Slash=strFilePath.find_last_of('/');
			if(Slash!=-1 &&Pot!=-1){
				FileName=strFilePath.substr(Slash+1,Pot-Slash-1);
			}
		}
		else{
			FileName=strFilePath;///全部是文件名
		}
	}
	return FileName;
}

std::string CComFunc::GetFileNameWithSuffix(const std::string	& strFilePath)
{
	std::string FileName="";
	int Pot=strFilePath.find_last_of('.');
	int Slash=strFilePath.find_last_of('\\');

	if(Slash!=-1 &&Pot!=-1){
		FileName=strFilePath.substr(Slash+1);
	}
	else{
		Slash=strFilePath.find_last_of('/');
		if(Slash!=-1 &&Pot!=-1){
			FileName=strFilePath.substr(Slash+1);
		}
		else{	
			FileName=strFilePath;///全部是文件名
		}
	}
	return FileName;
}

///获取文件后缀名
std::string CComFunc::GetFileExt(const std::string &strFilePath)
{
	std::string FileExt="";
	int Pot=strFilePath.find_last_of('.');;
	if(Pot!=-1){
		FileExt=strFilePath.substr(Pot+1);
	}
	return FileExt;
}

std::string CComFunc::GetFilePath(const std::string &strFileFullPath)
{
	std::string FileName="";
	int Pot=strFileFullPath.find_last_of('.');
	int Slash=strFileFullPath.find_last_of('\\');
	if(Slash!=-1 &&Pot!=-1){
		FileName=strFileFullPath.substr(0,Slash);
	}
	else{
		if(Slash==-1){
			Slash=strFileFullPath.find_last_of('/');
			if(Slash!=-1 && Pot!=-1)
				FileName=strFileFullPath.substr(0,Slash);
		}
	}
	return FileName;
}

//void DeleteDir(const std::string& path) {
//	try {
//		// Check if the path exists and is a directory  
//		if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
//			// Recursively remove the directory and its contents  
//			std::filesystem::remove_all(path);
//			std::cout << "Directory deleted: " << path << std::endl;
//		}
//		else {
//			std::cout << "Directory does not exist: " << path << std::endl;
//		}
//	}
//	catch (const std::filesystem::filesystem_error& e) {
//		std::cerr << "Error deleting directory: " << e.what() << std::endl;
//	}
//}

bool CComFunc::IsFileExist(const std::string& csFile)
{
	DWORD dwAttrib = GetFileAttributesA((LPSTR)(LPCSTR)csFile.c_str());
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
// 判断文件夹是否存在
bool CComFunc::IsDirExist(const std::string & csDir)
{
	DWORD dwAttrib = GetFileAttributesA((LPSTR)(LPCSTR)csDir.c_str());
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
