#pragma once
#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>

/// <summary>
/// 消息队列
/// </summary>
/// <typeparam name="T">消息类型</typeparam>
template<class T>
class AngKMessageQueue
{
public:
	/// <summary>
	/// 推入消息
	/// </summary>
	/// <param name="msg">消息对象</param>
	void push(const T& msg) {
		QMutexLocker lck(&_mtx);
		_queue.enqueue(msg);
		_cv.notify_one();
	}
	/// <summary>
	/// 轮询消息
	/// </summary>
	/// <param name="msg">消息对象</param>
	/// <returns>是否接收到消息</returns>
	bool poll(T& msg) {
		QMutexLocker lck(&_mtx);
		if (_queue.size())
		{
			msg = _queue.front();
			_queue.dequeue();
			return true;
		}
		return false;
	}
	/// <summary>
	/// 等待消息
	/// </summary>
	/// <param name="msg">消息对象</param>
	void wait(T& msg) {
		QMutexLocker lck(&_mtx);
		while (!_queue.size()) _cv.wait(&_mtx);
		msg = _queue.front();
		_queue.dequeue();
	}
	//队列长度
	size_t size() {
		QMutexLocker lck(&_mtx);
		return _queue.size();
	}

private:
	//队列
	QQueue<T> _queue;
	//互斥变量
	QMutex _mtx;
	//条件变量
	QWaitCondition _cv;
};
