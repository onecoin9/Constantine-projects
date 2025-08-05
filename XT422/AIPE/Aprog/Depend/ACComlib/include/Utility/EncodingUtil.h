#ifndef AC_ENCODINGUTIL_H
#define AC_ENCODINGUTIL_H

#include <string>
#include "ACLib.h"

BEGIN_AC_UTIL


/**
 * @brief utf8编码转gbk编码
 * @param[IN] utf8		utf8编码字符串
 * @return 返回说明
 *     -std::string		gbk编码字符串
 */
AC_COMMONLIB_API std::string UTF8ToGBK(const std::string& utf8);



/**
 * @brief gbk编码转utf8编码
 * @param[IN] utf8		gbk编码字符串
 * @return 返回说明
 *     -std::string		utf8编码字符串
 */
AC_COMMONLIB_API std::string GBKToUTF8(const std::string& utf8);


END_AC_UTIL

#endif // !AC_ENCODINGUTIL_H

