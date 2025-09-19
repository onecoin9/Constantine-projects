#ifndef  _COMFUNC_H_
#define _COMFUNC_H_
#include <afxcmn.h>

#ifndef SAFEDEL
#define SAFEDEL(a) do { if (a) {delete a; a = NULL;} } while (0)
#endif

#ifndef SAFEDELARRAY
#define SAFEDELARRAY(a) do { if (a) {delete[] a; a = NULL;} } while (0)
#endif 

namespace CComFunc{
	CString GetCurrentPath(void);   ///获取当前执行路径
	CString GetFileName(const CString &strFilePath);  ///给定一个全路径，返回文件名称，不包含后缀
	CString GetFilePath(const CString &strFileFullPath);///给定一个全路径，返回文件路径
	void DeleteDir(CString str);
	CString GetFileExt(const CString &strFilePath);///给定一个全路径，返回后缀名
	CString GetFileNameWithSuffix(const CString	& strFilePath);///给定一个全路径，返回文件名称，包含后缀
};
#endif