#ifndef CHIPDBMODEL_H
#define CHIPDBMODEL_H

#include "reflection.hpp"
#include <QString>


struct chip
{
    int id;
    std::string name;
    int manuid;
    int packid;
    int adapterid;
    int typeId;
    int algofileid;
    int fpgafileid;
    int appfileid;
    unsigned long buffsize;
    std::string chipinfo;
    std::string buffinfo;
    int debug;
    int opcfgmask;
    int version;
    std::string modifyinfo;
    unsigned long devid;
    unsigned long drvparam;
    int spcfileid;
    int mstkfileid;
    int fpgafile2id;
    int adapter2id;
    int adapter3id;
    int adapter4id;
    int designerid;
    std::string history;
    unsigned long sectorsize;
    int helpfileid;
    unsigned long progtype;
    std::string opcfgattr;
};

REFLECTION(chip, id, name, manuid, packid, adapterid, typeId, algofileid, fpgafileid, appfileid, buffsize, chipinfo, buffinfo,
           debug, opcfgmask, version, modifyinfo, devid, drvparam, spcfileid, mstkfileid, fpgafile2id, adapter2id, adapter3id, adapter4id,
           designerid, history, sectorsize, helpfileid, progtype, opcfgattr)


struct manufacture	//芯片厂商
{
    int id;
    std::string name;	//厂商名称

    manufacture()
    {
        id = 0;
        name = "";
    }
};

REFLECTION(manufacture, id, name)

struct adapter	//转接板(适配器)型号
{
    int id;
    std::string name;	//转接板名称
    int chipid;         //芯片ID
};

REFLECTION(adapter, id, name, chipid)

struct algofile	//驱动文件
{
    int id;
    std::string name;	//驱动文件名称
};

REFLECTION(algofile, id, name)

struct appfile	//驱动自定义配置文件
{
    int id;
    std::string name;	//文件名称
};

REFLECTION(appfile, id, name)

struct chiptype	//类型操作
{
    int id;
    std::string name;	//类型操作名称
};

REFLECTION(chiptype, id, name)

struct fpgafile	//FPGA算法文件
{
    int id;
    std::string name;	//FPGA算法文件名称
};

REFLECTION(fpgafile, id, name)

struct helpfile	//芯片帮助文件
{
    int id;
    std::string name;	//芯片帮助文件名称
};

REFLECTION(helpfile, id, name)

struct mstkofile	//Master文件
{
    int id;
    std::string name;	//Master文件名称
};

REFLECTION(mstkofile, id, name)

struct package	//芯片封装
{
    int id;
    std::string name;	//芯片封装名称
};

REFLECTION(package, id, name)

struct spcfile	//XML文件，实际没有使用
{
    int id;
    std::string name;	//XML文件，实际没有使用
};

REFLECTION(spcfile, id, name)

struct operAttr
{
    int id;
    QString operAttriStr;
};

REFLECTION(operAttr, id, operAttriStr)

#endif // CHIPDBMODEL_H
