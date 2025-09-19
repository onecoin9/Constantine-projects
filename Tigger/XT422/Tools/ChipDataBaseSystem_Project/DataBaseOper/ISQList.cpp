#include "ISQList.h"

#include <Windows.h>
#include "iostream"
//std::vector<void*> ISQList::m_select_ret = {};

ISQList::ISQList() : db_(nullptr) {

}

ISQList::~ISQList() {
    if (db_ != nullptr) {
        std::cout<<"~ISQList()";
        Disconnect();
    }
}



bool ISQList::Connect(const std::string& db_path) {
    int ret = sqlite3_open(db_path.c_str(), &db_);
    if (ret) {
        last_err_ = sqlite3_errmsg(db_);
        return false;
    }
    return true;
}



void ISQList::Disconnect() {
    sqlite3_close(db_);
    db_ = nullptr;
}



bool ISQList::Execute(const std::string& sql) {
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
        last_err_ = sqlite3_errmsg(db_);
        return false;
    }
    return true;
}



void ISQList::SetMemVar(std::string& mem_var, sqlite3_stmt* stmt, int i) const {
    const char* text = (const char*)sqlite3_column_text(stmt, i);
    mem_var = text ? utf82gbk(text) : "NULL";
}



void ISQList::SetMemVar(int& mem_var, sqlite3_stmt* stmt, int i) const {
    mem_var = sqlite3_column_int(stmt, i);
}



void ISQList::SetMemVar(double& mem_var, sqlite3_stmt* stmt, int i) const {
    mem_var = sqlite3_column_double(stmt, i);
}



void ISQList::SetMemVar(bool& mem_var, sqlite3_stmt* stmt, int i) const{
    mem_var = (bool)sqlite3_column_text(stmt, i);
}



void ISQList::SetMemVar(unsigned long& mem_var, sqlite3_stmt* stmt, int i) const
{
    mem_var = (unsigned long)sqlite3_column_int64(stmt, i);
}


void ISQList::SetMemVar(QString&mem_var, sqlite3_stmt *stmt, int i) const
{
    mem_var = (char*)sqlite3_column_blob(stmt, i);
}



void ISQList::BindMemVar(std::string& mem_var, sqlite3_stmt* stmt, int i) const {
    if (mem_var.compare("NULL") == 0)
        sqlite3_bind_null(stmt, i);
    else
        sqlite3_bind_text(stmt, i, mem_var.c_str(), mem_var.size(), nullptr);
}



void ISQList::BindMemVar(int& mem_var, sqlite3_stmt* stmt, int i) const {
    sqlite3_bind_int(stmt, i, mem_var);
}



void ISQList::BindMemVar(double& mem_var, sqlite3_stmt* stmt, int i) const {
    sqlite3_bind_double(stmt, i, mem_var);
}


void ISQList::BindMemVar(unsigned long& mem_var, sqlite3_stmt* stmt, int i) const{
    sqlite3_bind_int64(stmt, i, mem_var);
}


void ISQList::BindMemVar(bool &mem_var, sqlite3_stmt *stmt, int i) const{
    sqlite3_bind_int(stmt, i, mem_var);
}


void ISQList::BindMemVar(QString &mem_var, sqlite3_stmt *stmt, int i) const{
    sqlite3_bind_blob(stmt, i, mem_var.toStdString().c_str(), mem_var.size(), SQLITE_TRANSIENT);
}


std::string ISQList::utf82gbk(const char* utf8) const {
    int len = ::MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    wchar_t* w_gbk = new wchar_t[len + 1];
    memset(w_gbk, 0, len * 2 + 2);
    ::MultiByteToWideChar(CP_UTF8, 0, utf8, -1, w_gbk, len);
    len = ::WideCharToMultiByte(CP_ACP, 0, w_gbk, -1, NULL, 0, NULL, NULL);
    char* gbk = new char[len + 1];
    memset(gbk, 0, len + 1);
    ::WideCharToMultiByte(CP_ACP, 0, w_gbk, -1, gbk, len, NULL, NULL);

    std::string ret(gbk);
    if (w_gbk) delete[] w_gbk;
    if (gbk) delete[] gbk;
    return ret;
}
