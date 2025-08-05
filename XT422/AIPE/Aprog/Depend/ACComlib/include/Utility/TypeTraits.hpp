#ifndef AC_TYPETRAITS_HPP
#define AC_TYPETRAITS_HPP

#include <type_traits>
#include "ACLib.h"

//-------------------------------------------------------------------
// 元函数
//-------------------------------------------------------------------
BEGIN_AC


//-------------------------------------------------------------------
// 给定类型T，T是否存是C风格字符串
// IsCStr<char*>::value  IsCStr<const char*>::value  IsCStr<char[]>::value
// char*, const char*, char[]是否存是C风格字符串 - 是
//-------------------------------------------------------------------
template<typename T>
struct IsCStr : std::false_type {};

template<std::size_t N>
struct IsCStr<char[N]> : std::true_type {};

template<std::size_t N>
struct IsCStr<unsigned char[N]> : std::true_type {};

template<std::size_t N>
struct IsCStr<const char[N]> : std::true_type {};

template<std::size_t N>
struct IsCStr<const unsigned char[N]> : std::true_type {};

template<>
struct IsCStr<char[]> : std::true_type {};

template<>
struct IsCStr<unsigned char[]> : std::true_type {};

template<>
struct IsCStr<const char[]> : std::true_type {};

template<>
struct IsCStr<const unsigned char[]> : std::true_type {};

template<>
struct IsCStr<const char*> : std::true_type {};

template<>
struct IsCStr<const unsigned char*> : std::true_type {};

template<>
struct IsCStr<char*> : std::true_type {};

template<>
struct IsCStr<unsigned char*> : std::true_type {};



//-------------------------------------------------------------------
// 给定类型T，T是否存在于Args...中
// Contains<std::string, int, double, char>::value
// std::string是否存在于int double char中 - 不存在
//-------------------------------------------------------------------
template<typename... Args>
struct Contains {};

template<typename T>
struct Contains<T> : std::false_type {};

template<typename T, typename U, typename... Args>
struct Contains<T, U, Args...> : std::conditional_t<std::is_same<T, U>::value, std::true_type, Contains<T, Args...>>  {};



//-------------------------------------------------------------------
// 给定类型Args...，Args...是都否符合可格式化类型
// IsFormatType<std::string, int, double, char>::value
// std::string, int, double, char是都都符合可格式化类型 - std::string不符合
//-------------------------------------------------------------------
template<typename... Args>
struct IsFormatType {};

template<>
struct IsFormatType<> : std::true_type {};

template<typename T, typename... Args>
struct IsFormatType<T, Args...> 
	: std::conditional_t<std::is_arithmetic<std::remove_reference_t<T>>::value || AC IsCStr<std::remove_reference_t<T>>::value,
						 IsFormatType<Args...>, std::false_type>
{};



//-------------------------------------------------------------------
// 给定类型T，T是否是stringv tuple vector list
// IsVector<std::vector<int>>::value
// std::vector<int>是否是vector类型 - 是
//-------------------------------------------------------------------
template <template <typename...> class U, typename T>
struct IsTemplateInstant : std::false_type {};

template <template <typename...> class U, typename... Args>
struct IsTemplateInstant<U, U<Args...>> : std::true_type {};

template <typename T>
struct IsStdString : IsTemplateInstant<std::basic_string, T> {};

template <typename T>
struct IsTuple : IsTemplateInstant<std::tuple, T> {};

template <typename T>
struct IsVector : IsTemplateInstant<std::vector, T> {};


END_AC

#endif // !AC_TYPETRAITS_HPP

