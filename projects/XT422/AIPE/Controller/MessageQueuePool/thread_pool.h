#pragma once
#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <string>

/* #define DEBUG */
#include "safe_queue_base.h"

class ThreadPool
{
public:
	ThreadPool(std::size_t _thread_count);
	ThreadPool(ThreadPool&&) = delete;
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(ThreadPool&&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	~ThreadPool();

	template <typename T>
	void addQueue();

	template <typename T>
	std::shared_ptr<SafeQueue<T>> getQueue();

	template <typename Func, typename... Args>
	auto addTask(Func&& f, Args &&...args)->std::future<typename std::result_of<Func(Args...)>::type>;

private:
	using TaskType = typename std::function<void()>;
	std::mutex mux;
	void startBuildPool();
	std::atomic<bool> m_running_state{ false };
	std::size_t m_thread_count = 0;
	std::vector<std::shared_ptr<std::thread>> m_threads;
	SafeQueue<TaskType> m_task_queue;
	std::vector<std::shared_ptr<SafeQueueBase>> m_message_queue;
	std::condition_variable m_cv;
};


inline ThreadPool::ThreadPool(std::size_t _thread_count)
{
	auto sys_max_threads = std::thread::hardware_concurrency();// get system max threads number
	if (_thread_count <= 0 || _thread_count >= sys_max_threads) {
		m_thread_count = sys_max_threads;
		std::cerr << "WARNING: use capable thread count:" << m_thread_count << std::endl;
	}
	else {
		m_thread_count = _thread_count;
	}

	m_threads.reserve(m_thread_count);
	m_running_state = true;
	startBuildPool();
}

inline ThreadPool::~ThreadPool()
{
	m_running_state = false;
	m_cv.notify_all();// break all thread in block state
	for (auto& thread : m_threads) {
		if (thread->joinable()) {
			thread->join();
		}
	}
}


template <typename T>
inline void ThreadPool::addQueue()
{
	std::shared_ptr<SafeQueueBase> queue = std::make_shared<SafeQueue<T>>();
	m_message_queue.push_back(std::move(queue));
}

template <typename T>
inline std::shared_ptr<SafeQueue<T>> ThreadPool::getQueue()
{
	for (const auto& e : m_message_queue) {
		// use dynamic_pointer_cast to transform shared_ptr from base class to subclass
		if (auto ptr = std::dynamic_pointer_cast<SafeQueue<T>>(e)) {
			return ptr;
		}
	}
	return nullptr;
}

template <typename Func, typename... Args>
inline auto ThreadPool::addTask(Func&& f, Args &&...args) -> std::future<typename std::result_of<Func(Args...)>::type>
{
	using return_type = typename std::result_of<Func(Args...)>::type;
	auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
	TaskType thread_task = [task]() { (*task)(); };// 不需要移动了，使用 shared_ptr
	m_task_queue.push(thread_task);
	m_cv.notify_one();
	return task->get_future();
}

inline void ThreadPool::startBuildPool()
{
	for (size_t i = 0; i < m_thread_count; ++i) {
		auto t = [this, i]() {
			while (this->m_running_state) {
				TaskType task;
				std::unique_lock<std::mutex> lck(mux);
				if (this->m_task_queue.empty()) {
					m_cv.wait(lck);
				}
				// need to check again when thread pool destructed can also make wait() pass by notice_all
				// and when the condition variable is waiting, it well unlock lck, this may cause many threads are waiting at the same time
				if (this->m_running_state == false || m_task_queue.empty()) {
					std::this_thread::sleep_for(std::chrono::milliseconds(2));
					continue;
				}
				this->m_task_queue.pop(task);
				lck.unlock();
				try {
					task();// run task;
				}
				catch (...) {
				}
			}
		};
		m_threads.emplace_back(std::make_shared<std::thread>(std::thread(t)));
	}
}
#endif// THREAD_POOL_H