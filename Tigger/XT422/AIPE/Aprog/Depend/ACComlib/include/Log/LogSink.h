#ifndef AC_LOGSINK_H
#define AC_LOGSINK_H

#include <memory>

#include "ACLib.h"
#include "LogParams.h"
#include "STL/STLUtility.h"

BEGIN_AC_LOG


class Logger;

class AC_COMMONLIB_API LogSink {
	friend class Logger;

public:
	LogSink(AC_LOG SinkType type);
	LogSink(AC_LOG SinkType type, AC_LOG LogLevel lvl, const char* pattern);
	virtual ~LogSink();

	void SetLevel(AC_LOG LogLevel lvl);
	void SetPattern(const char* pattern);

	AC_LOG SinkType Type() const noexcept;

private:
	std::string FormatPattern(std::string&& content) const;
	virtual void PrintLogImpl(AC_LOG LogLevel lvl, const std::string& text) = 0;


	template<typename... Args>
	void PrintLog(AC_LOG LogLevel lvl, const char* fmt, Args&&... args) 
	{
		if (lvl < m_eLvl)
			return;
		std::string&& text = FormatPattern(AC_STL FormatString(fmt, std::forward<Args>(args)...));
		PrintLogImpl(lvl, text);
	}


	LogSink(const LogSink&) = delete;
	LogSink(LogSink&&) = delete;
	LogSink& operator=(const LogSink&) = delete;
	LogSink& operator=(LogSink&&) = delete;

protected:
	AC_LOG SinkType m_eType;
	AC_LOG LogLevel m_eLvl;
	const char* m_strPattern;
};


using SinkPtr = std::shared_ptr<AC_LOG LogSink>;



/**
 * @brief 实例化sink
 * @param[IN] args		构造函数参数
 * @return 返回说明
 *     -SinkPtr	LogSink共享指针
 */
template<typename T, typename... Args>
std::enable_if_t<std::is_base_of<LogSink, T>::value, SinkPtr> 
MakeSinkPtr(Args&&... args)
{
	SinkPtr sink = std::make_shared<T>(std::forward<Args>(args)...);
	return sink;
}


END_AC_LOG

#endif // !AC_LOGSINK_H
