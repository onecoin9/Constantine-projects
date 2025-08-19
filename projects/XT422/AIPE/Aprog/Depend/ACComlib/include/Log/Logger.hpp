#ifndef AC_LOGGER_HPP
#define AC_LOGGER_HPP

#include "LogSink.h"

BEGIN_AC_LOG


class Logger
{
public:
	using LoggerPtr = std::unique_ptr<Logger>;

	static LoggerPtr& Instance() 
	{
		static LoggerPtr logger(new Logger());
		return logger;
	}


	void AppenSink(AC_LOG SinkPtr sink);
	void AppenSink(std::vector<AC_LOG SinkPtr>&& sinks);


	template<typename... Args>
	void PrintLog(AC_LOG SinkType type, AC_LOG LogLevel lvl, const char* fmt, Args&&... args);

	~Logger() {}

private:
	Logger() {}

	Logger(const Logger&) = delete;
	Logger(Logger&&) = delete;
	Logger& operator=(const Logger&) = delete;
	Logger& operator=(Logger&&) = delete;

private:
	std::vector<AC_LOG SinkPtr> sink_;
};



template<typename... Args>
void Logger::PrintLog(AC_LOG SinkType type, AC_LOG LogLevel lvl, const char* fmt, Args&&... args) 
{
	for (auto& sink : sink_) 
	{
		if (type != AC_LOG SinkType::null && sink->Type() != type)
			continue;
		sink->PrintLog(lvl, fmt, std::forward<Args>(args)...);
	}
}



void Logger::AppenSink(AC_LOG SinkPtr sink)
{
	sink_.push_back(std::move(sink));
}



void Logger::AppenSink(std::vector<AC_LOG SinkPtr>&& sinks)
{
	for (auto& sink : sinks)
	{
		sink_.push_back(std::move(sink));
	}
}


END_AC_LOG


#define APPEND_SINK(sink) Acro::Log::Logger::Instance()->AppenSink(sink);
#define LOG_D(fmt, ...) Acro::Log::Logger::Instance()->PrintLog(Acro::Log::SinkType::null, Acro::Log::LogLevel::debug, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...) Acro::Log::Logger::Instance()->PrintLog(Acro::Log::SinkType::null, Acro::Log::LogLevel::info, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) Acro::Log::Logger::Instance()->PrintLog(Acro::Log::SinkType::null, Acro::Log::LogLevel::warn, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) Acro::Log::Logger::Instance()->PrintLog(Acro::Log::SinkType::null, Acro::Log::LogLevel::error, fmt, ##__VA_ARGS__)
#define LOG_C(fmt, ...) Acro::Log::Logger::Instance()->PrintLog(Acro::Log::SinkType::null, Acro::Log::LogLevel::critical, fmt, ##__VA_ARGS__)

#endif // !AC_LOGGER_HPP

