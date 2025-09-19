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
	CString GetCurrentPath(void);   ///��ȡ��ǰִ��·��
	CString GetFileName(const CString &strFilePath);  ///����һ��ȫ·���������ļ����ƣ���������׺
	CString GetFilePath(const CString &strFileFullPath);///����һ��ȫ·���������ļ�·��
	void DeleteDir(CString str);
	CString GetFileExt(const CString &strFilePath);///����һ��ȫ·�������غ�׺��
	CString GetFileNameWithSuffix(const CString	& strFilePath);///����һ��ȫ·���������ļ����ƣ�������׺
};
#endif