#ifndef AC_THREADUTIL_H
#define AC_THREADUTIL_H

#include <cstdint>
#include "ACLib.h"


BEGIN_AC_THREAD


/**
 * @brief 当前线程ID
 * @param void
 * @return 返回说明
 *     -uint32_t	线程ID
 */
AC_COMMONLIB_API uint32_t ThreadId();



/**
 * @brief 获取当前CPU线程数
 * @param void
 * @return 返回说明
 *     -uint32_t	CPU线程数
 */
AC_COMMONLIB_API uint32_t CpuThreadCount();



/**
 * @brief 当前线程休眠多少毫秒
 * @param[IN] ms		休眠时间，单位：毫秒
 * @return 返回说明
 *     -void
 */
AC_COMMONLIB_API void SleepFor(IN uint32_t ms);


END_AC_THREAD

#endif // !AC_THREADUTIL_H

