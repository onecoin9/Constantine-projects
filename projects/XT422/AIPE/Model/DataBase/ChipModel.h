#ifndef _CHIPMODEL_H_
#define _CHIPMODEL_H_

#include "reflection.hpp"

struct chip {	//数据表命名,跟数据库中保持一致
	int				id;						
	std::string		strName;				//芯片名称
	std::string		strManu;				//芯片厂商
	std::string		strAdapter;				//芯片转接板1
	std::string		strAdapter2;			//芯片转接板2
	std::string		strAdapter3;			//芯片转接板3
	std::string		strPack;				//芯片封装   TSOP48等
	std::string		strType;				//芯片类型	 NAND,MCU等
	std::string		strAlgoFile;			//驱动算法文件
	std::string		strFPGAFile;			//FPGA算法文件1
	std::string		strFPGAFile2;			//FPGA算法文件2  GANG使用
	std::string		strAppFile;				//应用层程序，后面修改为驱动自定义配置文件，由驱动自己负责解析，PC传入的时候会保存到卡中
	std::string		strAdapterID;				//（自25.3.13之后该字段取自于adapter中的chipid字段）strSpcFileXML文件，实际没有使用，与算法文件同名 ，24.9.11用于表示Adapter名称
	std::string		strMstkoFile;			//Master 文件
	unsigned long	ulBufferSize;			//缓冲区大小
	std::string		strChipInfo;			//芯片信息
	std::string		strBuffInfo;			//缓冲区信息
	bool			bDebug;					
	unsigned long	ulOperateConfigMask;	//操作配置
	int				nVersion;				
	std::string		strModifyInfo;			
	std::string		strChipId;				//芯片ID
	unsigned long	ulDrvParam;				//AlgoID
	unsigned long	ulSectorSize;			//ALV分区大小
	std::string		strHelpFile;			//Help档案名称
	int				nProgType;				//最低字节表示编程器类型0--S1, 1—S2, 2—S4, 3—S8，最高字节表示insmode:00 DMM, 01 DIO 02 G8,03 eMMC 04 AG07
	std::string     strOperCfgJson;			//AG06操作配置保存到Json中
	std::string		strCurSbk;				
	int				nSbkId;					
	unsigned long	ulBufferSizeHigh;		//因为文件可能大于4G，所以这个地方存放高4字节
	std::string		strBottomBoard;			//底座编号，数据库中不使用
};
REFLECTION(chip, id, strName, strManu, strAdapter, strAdapter2, strAdapter3, strPack, strType, strAlgoFile, strFPGAFile, strFPGAFile2, strAppFile, strAdapterID \
	, strMstkoFile, ulBufferSize, strChipInfo, strBuffInfo, bDebug, ulOperateConfigMask, nVersion, strModifyInfo, strChipId, ulDrvParam, ulSectorSize, strHelpFile \
	, nProgType, strOperCfgJson, strCurSbk, nSbkId, ulBufferSizeHigh)

struct manufacture	//芯片厂商
{
	int id;
	std::string name;	//厂商名称
};

REFLECTION(manufacture, id, name)

struct adapter	//转接板(适配器)型号
{
	int id;
	std::string name;	//转接板名称
};

REFLECTION(adapter, id, name)

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

#endif // !_CHIPMODEL_H_