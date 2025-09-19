#ifndef AC_FILESINK_H
#define AC_FILESINK_H

#include "Log/LogSink.h"

BEGIN_AC_LOG


namespace detail {
class FileSinkImpl;
}

class AC_COMMONLIB_API FileSink : public LogSink
{
public:
	FileSink(const std::string& log_path);
	FileSink(const std::string& log_path, AC_LOG LogLevel lvl, const char* pattern);
	~FileSink();

private:
	void PrintLogImpl(AC_LOG LogLevel lvl, const std::string& text);

private:
	detail::FileSinkImpl* m_impl;
};


END_AC_LOG

#endif // !AC_FILESINK_H

