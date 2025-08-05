#include "AngKDataBaseOper.h"
#include <vector>
#include <time.h>
#include <QDebug>

std::vector<LoaclChipData::chip> Utils::AngKDataBaseOper::OperSelectChipData(QString dbFile)
{
    QString&& db_path = std::move(dbFile);
    //db_ptr<ISQList> db = make_db_ptr<ISQList>();
    if (!_db->Connect(db_path.toStdString())) {
        qDebug() << "Open database file fail";
        return std::vector<LoaclChipData::chip>();
    }

    auto sql1 = SQLBuilder()
        .SELECT("chip.id, chip.name AS ChipName, manufacture.name AS Manufacture, TableAdp1.name AS Adapter1, TableAdp2.name AS Adapter2")
        .SELECT("TableAdp3.name AS Adapter3, package.name AS PackageName, chiptype.name AS ChipType, algofile.name AS AlogFile, TableFPGA1.name AS FPGAFile1")
        .SELECT("TableFPGA2.name AS FPGAFile2, appfile.name AS APPFile, TableSPC.name AS SPCFileName, TableMstko.name AS MstkFile, chip.buffsize AS BuffSize")
        .SELECT("chip.chipinfo AS ChipInfo, chip.buffinfo AS BufferInfo, chip.debug AS Debug, chip.opcfgmask AS OpertionCfgMask, chip.version AS Version")
        .SELECT("chip.modifyinfo AS ModifyInfo, chip.devid AS DeviceID, chip.drvparam AS DriverParam, chip.sectorsize AS SectorSize, TableHelp.name AS HelpFile, chip.progtype AS ProgType, chip.opcfgAttr AS OperAttr")
        .FROM("chip")
        .INNER_JOIN("manufacture on chip.manuid = manufacture.id")
        .INNER_JOIN("adapter AS TableAdp1 ON chip.adapterid = TableAdp1.id")
        .LEFT_OUTER_JOIN("adapter AS TableAdp2 ON chip.adapter2id = TableAdp2.id")
        .LEFT_OUTER_JOIN("adapter AS TableAdp3 ON chip.adapter3id = TableAdp3.id")
        .INNER_JOIN("package ON chip.packid = package.id")
        .INNER_JOIN("chiptype ON chip.typeid = chiptype.id")
        .INNER_JOIN("algofile ON chip.algofileid = algofile.id")
        .INNER_JOIN("fpgafile AS TableFPGA1 ON chip.fpgafileid = TableFPGA1.id")
        .LEFT_OUTER_JOIN("fpgafile AS TableFPGA2 ON chip.fpgafile2id = TableFPGA2.id")
        .INNER_JOIN("appfile ON chip.appfileid = appfile.id")
        .LEFT_OUTER_JOIN("mstkofile AS TableMstko ON TableMstko.id = chip.mstkfileid")
        .LEFT_OUTER_JOIN("spcfile AS TableSPC ON TableSPC.id = chip.spcfileid")
        .LEFT_OUTER_JOIN("helpfile AS TableHelp ON TableHelp.id = chip.helpfileid ")
        .ORDER_BY("manufacture.name ASC")
        .ORDER_BY("chip.name ASC")
        .toString();

    auto&& res = _db->Select<LoaclChipData::chip>(true, lilaomo::cmp_("", sql1.c_str()), "");	//1.7w数据 查询加输出返回vector用时1s

    return res;
}

int Utils::AngKDataBaseOper::OperInsertChipData(QString dbFile, chip& chip_Info)
{
    QString&& db_path = std::move(dbFile);
    //db_ptr<ISQList> db = make_db_ptr<ISQList>();
    if (!_db->Connect(db_path.toStdString())) {
        qDebug() << "Open database file fail";
        return 0;
    }

    return _db->Insert<std::true_type::value>(chip_Info);
}
