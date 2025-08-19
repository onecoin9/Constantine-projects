#pragma once

#include <QString>
#include <vector>
#include "ChipModel.h"
#include "DBController.hpp"
#include "SQLBuilder.hpp"

class ACChipManager
{
public://查询返回类型当前根据 表模型 自行返回
	ACChipManager();
	~ACChipManager();

	//查询芯片数据
	std::vector<chip> OperSelectChipData(QString dbFile);

	//查询其他表数据
	template<typename T>
	const std::vector<T> OperSelectChipDataClassType(QString dbFile)
	{
		db_ptr<ISQList> _db = make_db_ptr<ISQList>();
		QString&& db_path = Utils::AngKPathResolve::localDBPath(dbFile);
		if (!_db->Connect(db_path.toStdString())) {
			return std::vector<T>();
		}

		auto&& res = _db->Select<T>(false, lilaomo::cmp_("name", " order by "), "");

		return res;
	}

	//通过其他表确定对应的id写入chip表
	template<typename T>
	const T OperSelectOtherDataClassType(QString dbFile, QString strName, QString selectSql)
	{
		if (strName.isEmpty())
		{
			return T();
		}
		db_ptr<ISQList> _db = make_db_ptr<ISQList>();
		QString&& db_path = Utils::AngKPathResolve::localDBPath(dbFile);
		if (!_db->Connect(db_path.toStdString())) {
			return T();
		}

		auto&& res = _db->Select<T>(false, lilaomo::and_(selectSql.toStdString().c_str()), strName.toStdString());

		if (res.empty())
		{
			return T();
		}

		return res[0];
	}

	std::vector<chip> GetALLChipData();
	std::vector<manufacture> GetALLManufactureData();
	std::vector<adapter> GetALLAdapterData();
	std::vector<chiptype> GetALLChipTypeData();

	void SetSelectChipData(int nIndex);
	const chip& GetChipData();

private:
	void InitACChipData();
};