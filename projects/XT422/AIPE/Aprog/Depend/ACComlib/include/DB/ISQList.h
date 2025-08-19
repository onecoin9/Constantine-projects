#ifndef AC_ISQLITE_H
#define AC_ISQLITE_H

#include <vector>
#include "sqlite/sqlite3.h"

#include "ACLib.h"
#include "DBUtil.hpp"

BEGIN_AC_DB


class AC_COMMONLIB_API ISQList {
	struct AC_COMMONLIB_API StmtRAII
	{
		StmtRAII();
		~StmtRAII();
		sqlite3_stmt*& Get();

	private:
		sqlite3_stmt* stmt_;
	};

public:
	ISQList();
	~ISQList();

	bool Connect(const std::string& db_path);
	void Disconnect();
	bool Execute(const std::string& sql);


	template<typename T, typename... Args>
	bool CreateTable(Args&&... args) 
	{
		std::string&& sql = detail::GetCreateTableSql<T>(std::forward<Args>(args)...);
		return Execute(sql);
	}


	template<typename T, typename... Args>
	bool CreateIndex(Args&&... args) 
	{
		std::string&& sql = detail::GetCreateIndexSql<T>(std::forward<Args>(args)...);
		return Execute(sql);
	}


	template<typename T, typename... Args>
	std::vector<T> Select(Args&&... args);


	template<int Auto, typename T>
	int Insert(T&& t) 
	{
		if (InsertImpl<Auto>(true, std::forward<T>(t)))
			return 1;
		return 0;
	}


	template<typename T>
	int Update(T&& t) 
	{
		if (InsertImpl<AC_DB NONAUTOINCREMENT>(false, std::forward<T>(t)))
			return 1;
		return 0;
	}


	template<typename T, typename... Args>
	int Delete(Args... args);


	inline const char* GetLastError() const noexcept 
	{
		return m_strErr;
	}


private:
	template<int Auto, typename T>
	bool InsertImpl(bool is_insert, T&& t);

	void SetMemVar(std::string& mem_var, sqlite3_stmt* stmt, int i) const;
	void SetMemVar(int32_t& mem_var, sqlite3_stmt* stmt, int i) const;
	void SetMemVar(double& mem_var, sqlite3_stmt* stmt, int i) const;
	template<std::size_t N>
	void SetMemVar(AC_DB Array<N>& mem_var, sqlite3_stmt* stmt, int i) const {
		std::string&& encoding = EncodingFromDB(sqlite3_column_text(stmt, i));
		mem_var = encoding;
	}

	void BindMemVar(std::string& mem_var, sqlite3_stmt* stmt, int i) const;
	void BindMemVar(int32_t& mem_var, sqlite3_stmt* stmt, int i) const;
	void BindMemVar(double& mem_var, sqlite3_stmt* stmt, int i) const;
	template<std::size_t N>
	void BindMemVar(AC_DB Array<N>& mem_var, sqlite3_stmt* stmt, int i) const {
		mem_var[0] != '\0' ? sqlite3_bind_text(stmt, i, mem_var, static_cast<int>(mem_var.RealSize()), nullptr) : sqlite3_bind_null(stmt, i);
	}

	std::string EncodingFromDB(const unsigned char* text) const;
	std::string EncodingToDB(const char* text) const;

private:
	sqlite3* m_ptrDB;
	const char* m_strErr;
};



template<typename T, typename... Args>
std::vector<T> ISQList::Select(Args&&... args)
{
	std::string&& sql = detail::GetSelectSql<T>(false, std::forward<Args>(args)...);
	StmtRAII stmt;
	if (sqlite3_prepare_v2(m_ptrDB, sql.c_str(), sql.size(), &stmt.Get(), nullptr) != SQLITE_OK)
	{
		m_strErr = sqlite3_errmsg(m_ptrDB);
		return {};
	}

	std::vector<T> res;
	while (true)
	{
		if (sqlite3_step(stmt.Get()) != SQLITE_ROW)
			break;

		T t = {};
		detail::ForEach<T>([&t, &stmt, this](auto& item, auto i) {
			this->SetMemVar(t.*item, stmt.Get(), i.value);
		});
		res.push_back(std::move(t));
	}
	return res;
}



template<typename T, typename... Args>
int ISQList::Delete(Args... args)
{
	int before_num = sqlite3_total_changes(m_ptrDB);
	std::string&& sql = detail::GetDeleteSql<T>(std::forward<Args>(args)...);
	StmtRAII stmt;
	if (sqlite3_prepare_v2(m_ptrDB, sql.c_str(), sql.size(), &stmt.Get(), nullptr) != SQLITE_OK)
	{
		m_strErr = sqlite3_errmsg(m_ptrDB);
		return 0;
	}
	if (sqlite3_step(stmt.Get()) != SQLITE_DONE)
	{
		m_strErr = sqlite3_errmsg(m_ptrDB);
		return 0;
	}

	int effect_row = sqlite3_total_changes(m_ptrDB) - before_num;
	return effect_row;
}



template<int Auto, typename T>
bool ISQList::InsertImpl(bool is_insert, T&& t)
{
	std::string&& sql = detail::GetInsertSql<T>(is_insert);
	StmtRAII stmt;
	if (sqlite3_prepare_v2(m_ptrDB, sql.c_str(), sql.size(), &stmt.Get(), nullptr) != SQLITE_OK)
	{
		m_strErr = sqlite3_errmsg(m_ptrDB);
		return false;
	}

	detail::ForEach<T>([&is_insert, &t, &stmt, this](auto& item, auto i) {
		if (is_insert && i.value == 0 && Auto == AC_DB AUTOINCREMENT)
			sqlite3_bind_null(stmt.Get(), 1);
		else
			this->BindMemVar(t.*item, stmt.Get(), i.value + 1);
	});

	if (sqlite3_step(stmt.Get()) != SQLITE_DONE)
	{
		m_strErr = sqlite3_errmsg(m_ptrDB);
		return false;
	}
	return true;
}


END_AC_DB

#endif // !AC_ISQLITE_H

