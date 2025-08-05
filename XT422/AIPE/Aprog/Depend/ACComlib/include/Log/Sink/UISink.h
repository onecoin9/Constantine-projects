#ifndef AC_UISINK_H
#define AC_UISINK_H

#include "Log/LogSink.h"

BEGIN_AC_LOG


namespace detail {
class UISinkImpl;
}

class AC_COMMONLIB_API UISink : public LogSink
{
public:
	using FuncType = std::function<void(AC_LOG LogLevel, const std::string&)>;

	UISink(FuncType&& func);
	UISink(FuncType&& func, AC_LOG LogLevel lvl, const char* pattern);
	~UISink();

private:
	void PrintLogImpl(AC_LOG LogLevel lvl, const std::string& text);

private:
	detail::UISinkImpl* m_impl;
};


END_AC_LOG

#endif // !AC_UISINK_H

