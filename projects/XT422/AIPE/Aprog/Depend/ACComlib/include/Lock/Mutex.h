#ifndef AC_MUTEX_H
#define AC_MUTEX_H

#include <mutex>

#include "ACLib.h"

BEGIN_AC_LOCK


// 共享互斥
class SharedMutex 
{

};


// 共享锁使能接收共享互斥类型
template<typename Mtx, typename = std::enable_if_t<std::is_same<Mtx, SharedMutex>::value>>
class SharedLock 
{

};


END_AC_LOCK

#endif // !AC_MUTEX_H

