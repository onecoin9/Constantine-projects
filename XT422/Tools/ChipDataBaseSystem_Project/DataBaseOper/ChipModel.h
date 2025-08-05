#ifndef _CHIPMODEL_H_
#define _CHIPMODEL_H_

#include "reflection.hpp"
#include <QString>

namespace LoaclChipData {
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
        std::string		strSpcFile;				//XML文件，实际没有使用，与算法文件同名
        std::string		strMstkoFile;			//Master 文件
        unsigned long	ulBufferSize;			//缓冲区大小
        std::string		strChipInfo;			//芯片信息
        std::string		strBuffInfo;			//缓冲区信息
        bool			bDebug;
        unsigned long	ulOperateConfigMask;	//操作配置
        int				nVersion;
        std::string		strModifyInfo;
        unsigned long	ulChipId;				//芯片ID
        unsigned long	ulDrvParam;				//AlgoID
        unsigned long	ulSectorSize;			//ALV分区大小
        std::string		strHelpFile;			//Help档案名称
        int				nProgType;				//最低字节表示编程器类型0--S1, 1—S2, 2—S4, 3—S8，最高字节表示insmode:00 DMM, 01 DIO 02 G8,03 eMMC 04 AG07      
        QString         strOperCfgJson;
        std::string		strCurSbk;
        int				nSbkId;
        unsigned long	ulBufferSizeHigh;		//因为文件可能大于4G，所以这个地方存放高4字节
        std::string		strBottomBoard;			//底座编号，数据库中不使用
    };
    REFLECTION(chip, id, strName, strManu, strAdapter, strAdapter2, strAdapter3, strPack, strType, strAlgoFile, strFPGAFile, strFPGAFile2, strAppFile, strSpcFile \
        , strMstkoFile, ulBufferSize, strChipInfo, strBuffInfo, bDebug, ulOperateConfigMask, nVersion, strModifyInfo, ulChipId, ulDrvParam, ulSectorSize, strHelpFile \
        , nProgType, strOperCfgJson, strCurSbk, nSbkId, ulBufferSizeHigh)
}



#endif // !_CHIPMODEL_H_
