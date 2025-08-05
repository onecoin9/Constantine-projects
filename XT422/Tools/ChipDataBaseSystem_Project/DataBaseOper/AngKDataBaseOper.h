#pragma once

#include <QString>
#include <vector>
#include "ChipModel.h"
#include "ChipDBModel.h"
#include "GlobalDefine.h"
#include "DBController.hpp"
#include "SQLBuilder.hpp"
namespace Utils
{

    static db_ptr<ISQList> _db = make_db_ptr<ISQList>();

	class AngKDataBaseOper
	{
	public://查询返回类型当前根据 表模型 自行返回

        //查询芯片数据
        static std::vector<LoaclChipData::chip> OperSelectChipData(QString dbFile);

        template<typename T>
        static const std::vector<T> OperSelectChipDataClassType(QString dbFile)
        {
            QString&& db_path = std::move(dbFile);
            //db_ptr<ISQList> db = make_db_ptr<ISQList>();
            if (!_db->Connect(db_path.toStdString())) {
                //qDebug() << "Open database file fail";
                return std::vector<T>();
            }

            auto&& res = _db->Select<T>(false, lilaomo::cmp_("name ASC", " order by "), "");	//0.03s

            return res;
        }

        //数据插入chip表
        static int OperInsertChipData(QString dbFile, chip& chip_Info);

        //通过其他表确定对应的id写入chip表
        template<typename T>
        static const T OperSelectOtherDataClassType(QString dbFile, QString strName, QString selectSql)
        {
            if(strName.isEmpty())
            {
                return T();
            }

            QString&& db_path = std::move(dbFile);
            //db_ptr<ISQList> db = make_db_ptr<ISQList>();
            if (!_db->Connect(db_path.toStdString())) {
                //qDebug() << "Open database file fail";
                return T();
            }

            auto&& res = _db->Select<T>(false, lilaomo::and_(selectSql.toStdString().c_str()), strName.toStdString());

            if(res.empty())
            {
                return T();
            }

            return res[0];
        }

        //插入表格数据
        template<typename T>
        static int OperInsertTableData(QString dbFile, T& tData)
        {
            QString&& db_path = std::move(dbFile);
            //db_ptr<ISQList> db = make_db_ptr<ISQList>();
            if (!_db->Connect(db_path.toStdString())) {
                qDebug() << "Open database file fail";
                return 0;
            }

            return _db->Insert<std::true_type::value>(tData);
        }

        //更新表格数据
        template<typename T>
        static int OperUpdateTableData(QString dbFile, T& tData)
        {
            QString&& db_path = std::move(dbFile);
            if (!_db->Connect(db_path.toStdString())) {
                qDebug() << "Open database file fail";
                return 0;
            }

            return _db->Update(tData);
        }

        //删除表格数据
        template<typename T>
        static int OperDeleteTableData(QString dbFile, int& tData)
        {
            QString&& db_path = std::move(dbFile);
            if (!_db->Connect(db_path.toStdString())) {
                qDebug() << "Open database file fail";
                return 0;
            }

            return _db->Delete<T>(lilaomo::and_("id = "), tData);
        }
	};
}

