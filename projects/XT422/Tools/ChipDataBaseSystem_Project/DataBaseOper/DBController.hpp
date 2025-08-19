#ifndef _DBCONTROLLER_HPP_
#define _DBCONTROLLER_HPP_

#include "ISQList.h"

#include <memory>

template<typename DB>
class DBController {
	using db_type = DB;
	using db_t = std::unique_ptr<db_type>;

public:
	DBController() : db_proxy_(std::make_unique<db_type>()) {

	}

	~DBController() {

	}

	template<typename T, typename... Args>
	bool CreateTable(Args&&... args) {
		return db_proxy_->CreateTable<T>(std::forward<Args>(args)...);
	}

	template<typename... Args>
	bool Connect(Args&&... args) {
		return db_proxy_->Connect(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	std::vector<T> Select(bool bSelf, Args&&... args) {
		return db_proxy_->Select<T>(bSelf, std::forward<Args>(args)...);
	}

	template<int Auto, typename T>
	int Insert(T&& t) {
		return db_proxy_->Insert<Auto>(std::forward<T>(t));
	}

	template<typename T>
	int Update(T&& t) {
		return db_proxy_->Update(std::forward<T>(t));
	}

	template<typename T,typename... Args>
	int Delete(Args&&... args) {
		return db_proxy_->Delete<T>(std::forward<Args>(args)...);
	}

	std::string GetLastError() const noexcept {
		return db_proxy_->GetLastError();
	}

private:
	db_t db_proxy_;
};



template<typename DB>
using db_ptr = std::unique_ptr<DBController<DB>>;

template<typename DB, typename... Args>
decltype(auto) make_db_ptr(Args&&... args) {
	return std::make_unique<DBController<DB>>(std::forward<Args>(args)...);
}

#endif // !_DBCONTROLLER_HPP_

