#ifndef AC_ACLIB_H


/**
*  @brief  动态库导入导出定义
*  @brief  Import and export definition of the dynamic library
* 
*  @brief  1、导出函数与导出类必须带上 AC_COMMONLIB_API 的宏定义
*  @brief  2、入参带上 IN 的宏定义，出参带上 OUT 的宏定义
*  @brief  例如：AC_COMMONLIB_API void AddFunc(IN int32_t v1, IN int32_t v2, OUT int32_t& ret);
*  @brief  3、函数、类命名采用首字母大写的方式，例如：void GetFilePath();
*  @brief  4、变量命名采用小写+下划线的方式，例如：file_path;
*  @brief  5、类成员采用m_+类型+名称的方式，例如：m_strPath;
*/
#ifndef AC_COMMONLIB_API

	#if (defined (_WIN32) || defined(WIN64))
		#if defined(AC_COMMONLIB_EXPORTS)
			#define AC_COMMONLIB_API __declspec(dllexport)
		#else
			#define AC_COMMONLIB_API __declspec(dllimport)
		#endif
	#else
		#ifndef __stdcall
			#define __stdcall
		#endif

		#ifndef AC_COMMONLIB_API
			#define  AC_COMMONLIB_API
		#endif
	#endif

#endif

#ifndef IN
	#define IN
#endif

#ifndef OUT
	#define OUT
#endif



//-------------------------------------------------------------------
// 命名空间
//-------------------------------------------------------------------
#define AC					Acro::
#define AC_STL				Acro::STL::
#define AC_CHRONO			Acro::Chrono::
#define AC_FILESYSTEM		Acro::FileSystem::
#define AC_LOCK				Acro::Lock::
#define AC_THREAD			Acro::Thread::
#define AC_LOG				Acro::Log::
#define AC_UTIL				Acro::Util::
#define AC_PARSER			Acro::Parser::
#define AC_DB				Acro::DB::


#define BEGIN_AC				namespace Acro {
#define END_AC					}

#define BEGIN_AC_STL			namespace Acro { namespace STL {
#define END_AC_STL				}}

#define BEGIN_AC_FILESYSTEM		namespace Acro { namespace FileSystem {
#define END_AC_FILESYSTEM		}}

#define BEGIN_AC_CHRONO			namespace Acro { namespace Chrono {
#define END_AC_CHRONO			}}

#define BEGIN_AC_LOCK			namespace Acro { namespace Lock {
#define END_AC_LOCK				}}

#define BEGIN_AC_THREAD			namespace Acro { namespace Thread {
#define END_AC_THREAD			}}

#define BEGIN_AC_LOG			namespace Acro { namespace Log {
#define END_AC_LOG				}}

#define BEGIN_AC_UTIL			namespace Acro { namespace Util {
#define END_AC_UTIL				}}

#define BEGIN_AC_Parser			namespace Acro { namespace Parser {
#define END_AC_Parser			}}

#define BEGIN_AC_DB				namespace Acro { namespace DB {
#define END_AC_DB				}}


#endif // !AC_ACLIB_H

