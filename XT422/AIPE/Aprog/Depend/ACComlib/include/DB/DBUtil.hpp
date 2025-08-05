#ifndef AC_DBUTIL_HPP
#define AC_DBUTIL_HPP

#include <string>
#include <array>
#include <tuple>

#include "ACLib.h"
#include "Reflection.hpp"
#include "Utility/TypeTraits.hpp"

#define CHAR_LEN(N) "VARCHAR("##N")"

BEGIN_AC_DB


//-------------------------------------------------------------------
// sql条件
//-------------------------------------------------------------------
//----------主键----------
struct PrimaryKey 
{
    const char* key;
    const char* condition;

    PrimaryKey(const char* k) : key(k), condition(" PRIMARY KEY") {}
};



//----------主键且为自增----------
struct Autoincrement 
{
    const char* key;
    const char* condition;

    Autoincrement(const char* k) : key(k), condition(" AUTOINCREMENT") {}
};



//----------不为空----------
struct NotNull 
{
    const char* key;
    const char* condition;

    NotNull(const char* k) : key(k), condition(" NOT NULL") {}
};



//----------不对外可用---------
namespace detail {
struct cmp_ 
{
    const char* key;
    const char* condition;
    const char* symbol;

    cmp_(const char* k, const char* c, const char* s) : key(k), condition(c), symbol(s) {}
};
}



//----------或----------
struct Or : public detail::cmp_
{
    Or(const char* condition) : cmp_("", condition, " or ") {}
};



//----------与----------
struct And : public detail::cmp_
{
    And(const char* condition) : cmp_("", condition, " and ") {}
};



//----------排序----------
constexpr static const char* ASC = " ASC ";         //降序
constexpr static const char* DESC = " DESC ";       //升序
struct OrderBy : public detail::cmp_
{
    OrderBy(const char* condition, const char* _sort) : cmp_(" ORDER BY ", condition, _sort) {}
};



//----------分组----------
struct GroupBy : public detail::cmp_
{
    GroupBy(const char* condition) : cmp_(" GROUP BY ", condition, "") {}
};



//-------------------------------------------------------------------
// 自增
//-------------------------------------------------------------------
constexpr static int AUTOINCREMENT = std::true_type::value;
constexpr static int NONAUTOINCREMENT = std::false_type::value;



//-------------------------------------------------------------------
// db元函数
//-------------------------------------------------------------------
//-------------------------------------------------------------------
// 给定类型T，T是否存Reflection类型
//-------------------------------------------------------------------
template <typename T>
using ReflectMembers = decltype(iguana_reflect_members(std::declval<T>()));

template <typename T, typename = void>
struct IsReflection : std::false_type {};

template <typename T>
struct IsReflection<T, std::enable_if_t<std::is_same<decltype(ReflectMembers<T>::size()), std::size_t>::value>>
    : std::true_type {};



namespace detail {
//-------------------------------------------------------------------
// 获取reflection结构体信息
//-------------------------------------------------------------------
template<typename T>
inline constexpr const char* TableName() 
{
    using M = ReflectMembers<T>;
    return M::name();
}

template<typename T>
inline constexpr const char* GetFields() 
{
    using M = ReflectMembers<T>;
    return M::fields();
}

template<typename T>
inline constexpr std::size_t MembersCnt() 
{
    using M = ReflectMembers<T>;
    return M::size();
}



//-------------------------------------------------------------------
// 遍历reflection结构体
//-------------------------------------------------------------------
template<typename... Args, typename F>
inline constexpr void GetMembItem(const std::tuple<Args...>& tup, F&& f) {}

template<std::size_t I, std::size_t... N, typename... Args, typename F>
inline constexpr void GetMembItem(const std::tuple<Args...>& tup, F&& f) 
{
    f(std::get<I>(tup), std::integral_constant<std::size_t, I>{});
    GetMembItem<N...>(tup, std::forward<F>(f));
}

template<typename... Args, typename F, std::size_t... Idx>
inline constexpr void ForEachImpl(const std::tuple<Args...>& tup, F&& f, std::index_sequence<Idx...>&&) 
{
    GetMembItem<Idx...>(tup, std::forward<F>(f));
}

template<typename T, typename F>
inline constexpr void ForEach(F&& f) 
{
    using M = ReflectMembers<T>;
    ForEachImpl(M::apply_impl(), std::forward<F>(f), std::make_index_sequence<M::size()>{});
}



//-------------------------------------------------------------------
// 数据库对应的类型名
//-------------------------------------------------------------------
template<typename T>
struct type_container {};

inline constexpr auto Type2Name(type_container<bool>) noexcept 
{
    return "INTEGER";
}
inline constexpr auto Type2Name(type_container<char>) noexcept 
{
    return "INTEGER";
}
inline constexpr auto Type2Name(type_container<short>) noexcept 
{
    return "INTEGER";
}
inline constexpr auto Type2Name(type_container<int>) noexcept 
{
    return "INTEGER";
}
inline constexpr auto Type2Name(type_container<float>) noexcept 
{
    return "FLOAT";
}
inline constexpr auto Type2Name(type_container<double>) noexcept 
{
    return "DOUBLE";
}
inline constexpr auto Type2Name(type_container<int64_t>) noexcept 
{
    return "INTEGER";
}
inline constexpr auto Type2Name(type_container<std::string>) noexcept 
{
    return "TEXT";
}
template<std::size_t N>
inline auto Type2Name(type_container<AC_DB Array<N>>) noexcept
{
    std::string s = "VARCHAR(" + std::to_string(N) + ")";
    return s;
}

template<typename T>
inline auto GetTypeNames() 
{
    constexpr auto SIZE = MembersCnt<T>();
    std::array<std::string, SIZE> arr = {};
    ForEach<T>([&](auto&, auto i) {
        constexpr auto INDEX = decltype(i)::value;
        using M = ReflectMembers<T>;
        using U = std::remove_reference_t<decltype(std::declval<T>().*(std::get<INDEX>(M::apply_impl())))>;
        arr[INDEX] = Type2Name(type_container<U>{});
    });
    return arr;
}



//-------------------------------------------------------------------
// sql语句 create table构造
//-------------------------------------------------------------------
template<typename... Args>
inline void GetKeyConditionImpl(std::string&, const char*, std::tuple<Args...>&) { }

template<std::size_t I, std::size_t... N, typename... Args>
inline void GetKeyConditionImpl(std::string& cond, const char* key, std::tuple<Args...>& tup)
{
    const auto& item = std::get<I>(tup);
    if (strcmp(item.key, key) == 0)
        cond.append(item.condition);
    GetKeyConditionImpl<N...>(cond, key, tup);
}

template<typename... Args, std::size_t... Idx>
inline std::string GetKeyConditionImpl(const char* key, std::tuple<Args...>&& tup, std::index_sequence<Idx...>&&)
{
    std::string cond;
    GetKeyConditionImpl<Idx...>(cond, key, tup);
    return cond;
}

template<typename... Args>
inline std::string GetKeyCondition(const char* key, Args&&... args) 
{
    return GetKeyConditionImpl(key, std::make_tuple<Args...>(std::forward<Args>(args)...),
        std::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>{});
}



//-------------------------------------------------------------------
// sql语句 where构造
//-------------------------------------------------------------------
inline void GetWhereSql(std::string& sql) 
{ 
    sql.append(";"); 
}

inline void GetWhereSql(std::string& sql, const std::string& where_sql) 
{ 
    sql.append(where_sql).append(";"); 
}

inline void GetWhereSql(std::string& sql, cmp_&& condition) 
{
    sql.append(condition.key).append(condition.condition).append(condition.symbol);
    GetWhereSql(sql);
}

template<typename... Args>
inline void GetWhereSql(std::string& sql, cmp_&& condition, const std::string& value, Args&&... args) 
{
    sql.append(condition.symbol).append(condition.condition).append(" '").append(value).append("'");
    GetWhereSql(sql, std::forward<Args>(args)...);
}

template<std::size_t N, typename... Args>
inline void GetWhereSql(std::string& sql, cmp_&& condition, const AC_DB Array<N>& value, Args&&... args)
{
    sql.append(condition.symbol).append(condition.condition).append(" '").append(value).append("'");
    GetWhereSql(sql, std::forward<Args>(args)...);
}

template<typename... Args>
inline void GetWhereSql(std::string& sql, cmp_&& condition, int value, Args&&... args) 
{
    sql.append(condition.symbol).append(condition.condition).append(" ").append(std::to_string(value));
    GetWhereSql(sql, std::forward<Args>(args)...);
}

template<typename... Args>
inline void GetWhereSql(std::string& sql, cmp_&& condition, double value, Args&&... args) 
{
    sql.append(condition.symbol).append(condition.condition).append(" ").append(std::to_string(value));
    GetWhereSql(sql, std::forward<Args>(args)...);
}



//-------------------------------------------------------------------
// sql语句 create index构造
//-------------------------------------------------------------------
inline void GetIndexName(std::string& sql, const std::string& field)
{
    sql.append(field);
}

template<typename... Args>
inline void GetIndexName(std::string& sql, const std::string& field, Args&&... args) 
{
    sql.append(field).append(", ");
    GetIndexName(sql, std::forward<Args>(args)...);
}



//-------------------------------------------------------------------
// 各sql语句构造
//-------------------------------------------------------------------
template<typename T, typename... Args>
std::string GetSelectSql(bool is_excel, Args&&... args) 
{
    std::string sql{"select * from "};
    is_excel ? sql.append("[").append(TableName<T>()).append("$]").append(" where 1 = 1") : sql.append(TableName<T>()).append(" where 1 = 1 ");
    GetWhereSql(sql, std::forward<Args>(args)...);
    return sql;
}



template <typename T>
std::string GetInsertSql(bool is_insert) 
{
    std::string sql = is_insert ? "insert into " : "replace into ";
    constexpr std::size_t SIZE = MembersCnt<T>();
    auto name = TableName<T>();
    auto fields = GetFields<T>();

    sql.append(name).append("(").append(fields).append(") values(");
    for (std::size_t i = 0; i < SIZE; ++i) {
        sql += "?";
        sql += i < SIZE - 1 ? ", " : ");";
    }
    return sql;
}



template<typename T, typename... Args>
std::string GetDeleteSql(Args&&... args) {
    std::string sql{"delete from "};
    sql.append(TableName<T>()).append(" where 1 = 1");

    GetWhereSql(sql, std::forward<Args>(args)...);

    return sql;
}



template<typename T, typename... Args>
std::string GetCreateTableSql(Args&&... args) {
    std::string sql{"CREATE TABLE IF NOT EXISTS "};
    sql.append(TableName<T>()).append("(");

    const auto type_name_arr = GetTypeNames<T>();
    std::string fields = GetFields<T>();
    std::size_t slow = 0, fast = 0, index = 0;
    while (fast != std::string::npos) {
        fast = fields.find(',', slow);
        std::string&& field_name = fields.substr(slow, fast - slow);
        if (field_name[0] == ' ')
            field_name.erase(0, 1);

        std::string&& key_cond = GetKeyCondition(field_name.c_str(), std::forward<Args>(args)...);
        sql.append(field_name).append(" ").append(type_name_arr[index]).append(key_cond);
        if (fast != std::string::npos)
            sql.append(",");
        slow = fast + 1;
        ++index;
    }

    sql.append(");");
    return sql;
}



template<typename T, typename... Args>
std::string GetCreateIndexSql(const std::string& index_name, Args&&... args) {
    std::string sql{"CREATE INDEX "};
    sql.append(index_name).append(" ON ").append(TableName<T>()).append("(");
    GetIndexName(sql, std::forward<Args>(args)...);
    sql.append(");");
    return sql;
}

}  // namespace detail


END_AC_DB

#endif // !AC_DBUTIL_HPP

