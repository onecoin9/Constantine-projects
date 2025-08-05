#ifndef AC_THREADPOOL_H
#define AC_THREADPOOL_H

#include <functional>

#include "ACLib.h"
#include "Utility/TypeTraits.hpp"

BEGIN_AC_THREAD


namespace detail {
class ThreadPoolImpl;
}

class AC_COMMONLIB_API ThreadPool 
{
public:
    ThreadPool();
    ~ThreadPool();

    void Start(uint32_t count) noexcept;
    void Stop() noexcept;


    template <typename Function, typename... Args>
    std::enable_if_t<!std::is_member_function_pointer<Function>::value, void>
    PushTask(Function&& func, Args&&... args) {
        PushTaskImpl(std::bind(std::forward<Function>(func), std::forward<Args>(args)...), 0);
    }


    template <typename RetType, class ClassType, typename T, typename... Args>
    std::enable_if_t<std::is_same<ClassType*, T>::value, void>
    PushTask(RetType ClassType::* f, T t, Args&&... args) {
        PushTaskImpl(std::bind(f, t, std::forward<Args>(args)...), 0);
    }


    template <typename Function, typename... Args>
    std::enable_if_t<!std::is_member_function_pointer<Function>::value, void>
    PushTaskDelay(uint32_t delay, Function&& func, Args&&... args) {
        PushTaskImpl(std::bind(std::forward<Function>(func), std::forward<Args>(args)...), delay);
    }


    template <typename RetType, class ClassType, typename T, typename... Args>
    std::enable_if_t<std::is_same<ClassType*, T>::value, void>
    PushTaskDelay(uint32_t delay, RetType ClassType::* f, T t, Args&&... args) {
        PushTaskImpl(std::bind(f, t, std::forward<Args>(args)...), delay);
    }


private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    void PushTaskImpl(std::function<void()>&& func, uint32_t delay);

private:
    detail::ThreadPoolImpl* m_impl;
};


END_AC_THREAD

#endif // !AC_THREADPOOL_H

