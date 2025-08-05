#ifndef  _COMFUNC_H_
#define _COMFUNC_H_

#ifndef SAFEDEL
#define SAFEDEL(a) do { if (a) {delete a; a = NULL;} } while (0)
#endif

#ifndef SAFEDELARRAY
#define SAFEDELARRAY(a) do { if (a) {delete[] a; a = NULL;} } while (0)
#endif 
#include <string>

#define SETBIT(x, y)  (x) |= (1 << (y))
#define CLRBIT(x, y)  (x) &= ~(1 << (y))
#define REVERTBIT(x, y)  (x) ^= (1 << (y))
#define GETBIT(x, y)   ((x) >> (y)& 1)

namespace CComFunc{
	std::string GetCurrentPath(void);   ///获取当前执行路径
	std::string GetFileName(const std::string &strFilePath);  ///给定一个全路径，返回文件名称，不包含后缀
	std::string GetFilePath(const std::string &strFileFullPath);///给定一个全路径，返回文件路径
	void DeleteDir(std::string str);
	std::string GetFileExt(const std::string &strFilePath);///给定一个全路径，返回后缀名
	std::string GetFileNameWithSuffix(const std::string	& strFilePath);///给定一个全路径，返回文件名称，包含后缀
	bool IsFileExist(const std::string& csFile);
	bool IsDirExist(const std::string & csDir);

};
#endif