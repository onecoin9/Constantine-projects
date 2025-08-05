#ifndef AC_LOGPARAMS_H
#define AC_LOGPARAMS_H

#include "ACLib.h"

BEGIN_AC_LOG


//-------------------------------------------------------------------
// 日志定义
//-------------------------------------------------------------------
enum class SinkType
{
	null,
	file,
	ui,
	console
};


enum class LogLevel 
{
	debug,
	info,
	warn,
	error,
	critical
};


constexpr const char* LevelName[] = {
	"Debug",
	"Info",
	"Warn",
	"Error",
	"Critical"
};


//constexpr COLORREF LevelColor[] = {
//	RGB(128, 128, 128),
//	RGB(0, 0, 0),
//	RGB(205, 149, 12),
//	RGB(255, 0, 0),
//	RGB(153, 0, 0)
//};


#define L_DEBUG LogLevel::debug
#define L_INFO LogLevel::info
#define L_WARN LogLevel::warn
#define L_ERROR LogLevel::error
#define L_CRITICAL LogLevel::critical


END_AC_LOG

#endif // !AC_LOGPARAMS_H

