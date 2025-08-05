#ifndef AC_CHRONOUTIL_H
#define AC_CHRONOUTIL_H

#include <time.h>
#include <string>

#include "ACLib.h"

BEGIN_AC_CHRONO


//-------------------------------------------------------------------
// 时间结构体
//-------------------------------------------------------------------
struct ACTime : public tm
{
    //tm 结构体中包含的
    //int tm_sec;   // seconds after the minute - [0, 60] including leap second
    //int tm_min;   // minutes after the hour - [0, 59]
    //int tm_hour;  // hours since midnight - [0, 23]
    //int tm_mday;  // day of the month - [1, 31]
    //int tm_mon;   // months since January - [0, 11]
    //int tm_year;  // years since 1900
    //int tm_wday;  // days since Sunday - [0, 6]
    //int tm_yday;  // days since January 1 - [0, 365]
    //int tm_isdst; // daylight savings time flag
	int tm_ms;      //毫秒
};



/**
 * @brief 获取当前时间
 * @param void
 * @return 返回说明
 *     -ACTime	当前时间结构体
 */
AC_COMMONLIB_API ACTime GetCurrentTime();



/**
 * @brief 获取当前时间字符串
 * @param[IN] fmt		时间格式
 * @return 返回说明
 *     -std::string		当前时间字符串
 */
AC_COMMONLIB_API std::string GetCurrentTime(const char* fmt);


END_AC_CHRONO

#endif // !AC_CHRONOUTIL_H

