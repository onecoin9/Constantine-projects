#pragma once
#ifndef SAFE_QUEUE_BASE_H
#define SAFE_QUEUE_BASE_H
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <vector>

struct TypeErasedData {
	virtual ~TypeErasedData() {}
};

template <typename T>
struct ErasedData : public TypeErasedData {
	ErasedData(T& value) : data(value) {}
	T& data;
};

class SafeQueueBase
{
public:
	SafeQueueBase() = default;
	SafeQueueBase(SafeQueueBase&&) = delete;
	SafeQueueBase(const SafeQueueBase&) = delete;
	SafeQueueBase& operator=(SafeQueueBase&&) = delete;
	SafeQueueBase& operator=(const SafeQueueBase&) = delete;

	bool empty()
	{
		std::shared_lock<std::shared_timed_mutex> lck(mux);
		return primaryEmpty();
	}
	size_t size()
	{
		std::shared_lock<std::shared_timed_mutex> lck(mux);
		return primarySize();
	}

	template <typename T>
	void pop(T& _data_output)
	{
		std::unique_lock<std::shared_timed_mutex> lck(mux);
		ErasedData<T> data_output(_data_output);
		primaryPop(&data_output);
	}

	template <typename T>
	void push(const T& _data)
	{
		std::unique_lock<std::shared_timed_mutex> lck(mux);
		ErasedData<const T> data(_data);
		primaryPush(&data);
	}
	template <typename T>
	void see(std::vector<T>& _data_output)
	{
		std::shared_lock<std::shared_timed_mutex> lck(mux);
		ErasedData<std::vector<T>> data_output(_data_output);
		primarySee(&data_output);
	}

	virtual ~SafeQueueBase() {};

protected:
	virtual void primaryPush(TypeErasedData* _data) = 0;
	virtual void primaryPop(TypeErasedData* _data) = 0;
	virtual void primarySee(TypeErasedData* _data) = 0;
	virtual bool primaryEmpty() = 0;
	virtual size_t primarySize() = 0;

private:
	std::shared_timed_mutex mux;  // WARN: need to support c++14
};

template <typename T>
class SafeQueue : public SafeQueueBase
{
public:
	SafeQueue() = default;
	~SafeQueue() = default;

	bool primaryEmpty() override
	{
		return m_queue.empty();
	}
	virtual size_t primarySize() override
	{
		return m_queue.size();
	}
	virtual void primaryPush(TypeErasedData* _data) override
	{
		auto data = dynamic_cast<ErasedData<const T>*>(_data);
		std::cout << data << std::endl;
		if (data == nullptr) {
			throw "Error data type";
		}
		m_queue.emplace(data->data);  // if c++11, use push instead of emplace
	}
	virtual void primaryPop(TypeErasedData* _data) override
	{
		auto data = dynamic_cast<ErasedData<T>*>(_data);
		if (data == nullptr) {
			throw "Error data type";
		}
		data->data = m_queue.front();
		m_queue.pop();
	}
	virtual void primarySee(TypeErasedData* _data) override
	{
		auto data = dynamic_cast<ErasedData<std::vector<T>>*>(_data);
		if (data == nullptr) {
			throw "Error data type";
		}
		size_t size = m_queue.size();
		for (size_t i = 0; i < size; ++i) {
			data->data.emplace_back(m_queue.front());
			m_queue.pop();
		}
	}

private:
	std::queue<T> m_queue;
};

#endif  // SAFE_QUEUE_BASE_H