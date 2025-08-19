#ifndef _ISQLITE_H_
#define _ISQLITE_H_

#include "reflection.hpp"

#include "../sqlite3/sqlite3.h"

class ISQList {
	struct StmtRAII {
		StmtRAII() : stmt_(nullptr) { }

		~StmtRAII() {
			if (stmt_ != nullptr)
				sqlite3_finalize(stmt_);
		}

		sqlite3_stmt*& GetStmt() {
			return stmt_;
		}

		sqlite3_stmt* stmt_;
	};

public:
	ISQList();
	~ISQList();

	bool Connect(const std::string& db_path);
	void Disconnect();
	bool Execute(const std::string& sql);

	template<typename T, typename... Args>
	bool CreateTable(Args&&... args) {
		std::string&& sql = lilaomo::get_create_table_sql<T>(std::forward<Args>(args)...);
		if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
			last_err_ = sqlite3_errmsg(db_);
			return false;
		}
		return true;
	}

	template<typename T, typename... Args>
	std::vector<T> Select(bool bSelf, Args&&... args) {
		std::string&& sql = bSelf ? lilaomo::get_select_self_sql<T>(std::forward<Args>(args)...) : lilaomo::get_select_sql<T>(false, std::forward<Args>(args)...);
		StmtRAII stmt;
		int ret = sqlite3_prepare_v2(db_, sql.c_str(), sql.size(), &stmt.GetStmt(), nullptr);
		if (ret != SQLITE_OK) {
			last_err_ = sqlite3_errmsg(db_);
			return {};
		}

		std::vector<T> res;
		while (true) {
			if (sqlite3_step(stmt.GetStmt()) != SQLITE_ROW)
				break;

			T t = {};
			lilaomo::for_each<T>([&t, &stmt, this](auto& item, auto i) {
				this->SetMemVar(t.*item, stmt.GetStmt(), i.value);
			});
			res.push_back(std::move(t));
		}
		return res;
	}

	template<int Auto, typename T>
	int Insert(T&& t) {
		if (InsertImpl<Auto>(true, std::forward<T>(t)))
			return 1;
		return 0;
	}

	/*template<int Auto, typename T>
	int Insert(T&& t) {
		int row = 0;
		for (const auto& i : t) {
			if (InsertImpl<Auto>(true, i))
				++row;
		}
		return row;
	}*/

	template<typename T>
	int Update(T&& t) {
		if (InsertImpl<lilaomo::NONAUTOINCREMENT>(false, std::forward<T>(t)))
			return 1;
		return 0;
	}

	/*template<typename T, typename = std::enable_if_t<lilaomo::is_reflection<T>::value>>
	int Update(std::vector<T>& t) {
		int row = 0;
		for (auto& i : t) {
			if (InsertImpl<0>(false, i))
				++row;
		}
		return row;
	}*/

	template<typename T, typename... Args>
	int Delete(Args... args) {
		int before_num = sqlite3_total_changes(db_);

		std::string&& sql = lilaomo::get_delete_sql<T>(std::forward<Args>(args)...);
		StmtRAII stmt;
		if (sqlite3_prepare_v2(db_, sql.c_str(), sql.size(), &stmt.GetStmt(), nullptr) != SQLITE_OK) {
			last_err_ = sqlite3_errmsg(db_);
			return 0;
		}
		if (sqlite3_step(stmt.GetStmt()) != SQLITE_DONE) {
			last_err_ = sqlite3_errmsg(db_);
			return 0;
		}

		int effect_row = sqlite3_total_changes(db_) - before_num;
		return effect_row;
	}

	inline std::string GetLastError() const noexcept {
		return last_err_;
	}


private:
	template<int Auto, typename T>
	bool InsertImpl(bool is_insert, T&& t) {
		std::string&& sql = lilaomo::get_insert_sql<T>(is_insert);
		StmtRAII stmt;
		if (sqlite3_prepare_v2(db_, sql.c_str(), sql.size(), &stmt.GetStmt(), nullptr) != SQLITE_OK) {
			last_err_ = sqlite3_errmsg(db_);
			return false;
		}

		lilaomo::for_each<T>([&is_insert, &t, &stmt, this](auto& item, auto i) {
			if (is_insert && i.value == 0 && Auto == lilaomo::AUTOINCREMENT)
				sqlite3_bind_null(stmt.GetStmt(), 1);
			else
				this->BindMemVar(t.*item, stmt.GetStmt(), i.value + 1);
		});

		if (sqlite3_step(stmt.GetStmt()) != SQLITE_DONE) {
			last_err_ = sqlite3_errmsg(db_);
			return false;
		}
		return true;
	}

	void SetMemVar(std::string& mem_var, sqlite3_stmt* stmt, int i) const;
	void SetMemVar(int& mem_var, sqlite3_stmt* stmt, int i) const;
	void SetMemVar(double& mem_var, sqlite3_stmt* stmt, int i) const;
	void SetMemVar(bool& mem_var, sqlite3_stmt* stmt, int i) const;
	void SetMemVar(unsigned long& mem_var, sqlite3_stmt* stmt, int i) const;

	void BindMemVar(std::string& mem_var, sqlite3_stmt* stmt, int i) const;
	void BindMemVar(int& mem_var, sqlite3_stmt* stmt, int i) const;
	void BindMemVar(double& mem_var, sqlite3_stmt* stmt, int i) const;

	std::string utf82gbk(const char* utf8) const;

private:
	sqlite3* db_;
	std::string last_err_;
};

#endif // !_ISQLITE_H_

