#ifndef AC_ACSTLUTILITY_H
#define AC_ACSTLUTILITY_H

#include <string>
#include <vector>
#include <list>

#include "ACLib.h"
#include "Utility/TypeTraits.hpp"

BEGIN_AC_STL


//-------------------------------------------------------------------
// 命名拓展
//-------------------------------------------------------------------
using StringList = std::list<std::string>;



/**
 * @brief C风格，格式化字符串
 * @param[IN] fmt		格式化格式
 * @param[IN] args		格式化参数
 * @return 返回说明
 *     -std::string		格式化后字符串
 */
template<typename... Args>
std::string FormatString(IN const char* fmt, IN Args&&... args)
{
	static_assert(AC IsFormatType<Args...>::value, "Exist not supported format type");
	int size = std::snprintf(nullptr, 0, fmt, std::forward<Args>(args)...) + 1;
	std::vector<char> buff(size, '\0');
	std::snprintf(&buff[0], size, fmt, std::forward<Args>(args)...);
	return std::string(&buff[0]);
}



/**
 * @brief 分割字符串
 * @param[IN] str		需分割的字符串
 * @param[IN] flag		分割标识
 * @return 返回说明
 *     -StringList	字符串数组
 */
AC_COMMONLIB_API AC_STL StringList Split(IN const std::string& str, IN char flag);


END_AC_STL

#endif // !AC_ACSTLUTILITY_H

