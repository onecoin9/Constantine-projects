#ifndef TESTERDEFINITIONS_H
#define TESTERDEFINITIONS_H
#pragma once

#include <string>
#include <vector>
extern int globalCmdID;
extern QString globalCmdRun;

enum class TesterSubCmdID
{
    UnEnable = 0x000,
    CheckID = 0x400,
    PinCheck = 0x401,
    InsertionCheck = 0x402,
    AutoSensing = 0x403,

    DevicePowerOn = 0x600,
    DevicePowerOff = 0x601,
    PowerOn = 0x602,
    PowerOff = 0x603,

    SubProgram = 0x800,
    SubErase = 0x801,
    SubVerify = 0x802,
    SubBlankCheck = 0x803,
    SubSecure = 0x804,
    SubIllegalCheck = 0x805,
    SubRead = 0x806,
    EraseIfBlankCheckFailed = 0x807,
    LowVerify = 0x808,
    HighVerify = 0x809,
    ChecksumCompare = 0x80A,
    SubReadTesterUID = 0x80B,
    HighLowVerify = 0x80C
};

typedef enum class _eSubCmdID {
    SubCmd_DataTrans_FIBER2SSD	= 0x01,
    SubCmd_DataTrans_FIBER2SKT	= 0x02,
    SubCmd_DataTrans_FIBER2DDR	= 0x06,
    SubCmd_DataTrans_SSD2FIBER	= 0x04,
    SubCmd_DataTrans_SSD2SKT	= 0x03,
    SubCmd_DataTrans_SKT2FIBER	= 0x05,
    SubCmd_DataTrans_DDR2FIBER	= 0x07,
    SubCmd_DataTrans_SSD2DDR	= 0x08,
    SubCmd_DataTrans_DDR2SSD	= 0x09,
    SubCmd_ReadCapacity_SSD		= 0x0A,
    SubCmd_ReadCapacity_DDR		= 0x0B,
    SubCmd_ReadCapacity_SKT		= 0x0C,
    SubCmd_Regist_Read			= 0x10,
    SubCmd_Regist_Write			= 0x11,
    SubCmd_ReadCRC32			= 0x18,
    SubCmd_MU_Start				= 0x400, //MU能够处理的命令起始, 最开始定义为0x40,根据文档修改为0x410
    SubCmd_MU_InstallFPGA		= 0x410,
    SubCmd_MU_InstallDriver		= 0x411,
    SubCmd_MU_SetChipInfo		= 0x412,
    SubCmd_MU_SetDriverSelfPara	= 0x413,
    SubCmd_MU_SetDriverCommon	= 0x414,
    SubCmd_MU_SetDriverPinMap	= 0x415,
    SubCmd_MU_SetDriverPartitionTable = 0x416,
    SubCmd_MU_DoCmdSequence		= 0x417,
    SubCmd_MU_SetDataBufferInfo = 0x418,
    SubCmd_MU_DownloadSSDComplete = 0x419,
    SubCmd_MU_SetBPUAttribute	= 0x420,
    SubCmd_MU_SetSNWithJson		= 0x421,
    SubCmd_MU_SetPartitionTableHeadAddr = 0x422,

    SubCmd_MU_AdapterRead		= 0x430,
    SubCmd_MU_AdapterWrite		= 0x431,
    SubCmd_MU_AdapterIncreaseCount = 0x432,
    SubCmd_MU_GetBPUInfo		= 0x433,
    SubCmd_MU_GetDeviceInfo		= 0x434,
    SubCmd_MU_SetDeviceAlias	= 0x435,
    SubCmd_MU_GetSktInfo		= 0x436,
    SubCmd_MU_GetSktInfoSimple	= 0x437,
    SubCmd_MU_GetMainBoardInfo	= 0x438,

    SubCmd_MU_RebootBPU			= 0x440,
    SubCmd_MU_SetBufferMapInfo	= 0x450,
    SubCmd_MU_UpdateFw			= 0x458, //更新FPGA固件到FW区
    SubCmd_MU_RebootMU			= 0x459, //固件升级完成后，PC主动发送Reset
    SubCmd_MU_UpdateDeviceTime	= 0x461,
    SubCmd_MU_DebugSetting		= 0x462,
    SubCmd_MU_GetRebootCause	= 0x463,
    SubCmd_MU_ProgramSetting	= 0x464,
    SubCmd_MU_GetProgramSetting = 0x465,
    SubCmd_MU_ProgrammerSelfTest = 0x466,
    SubCmd_MU_MasterChipAnalyze = 0x467,
    SubCmd_MU_ReadChipExtcsd	= 0x468,
    SubCmd_MU_GetSKTEnable		= 0x469,
    SubCmd_MU_DoSendCustom		= 0x490,

    //MU主动上报的
    SubCmd_MU_SetProgress		= 0x510,
    SubCmd_MU_SetLog			= 0x511,
    SubCmd_MU_SetEvent			= 0x514,
    SubCmd_ReadBuffData			= 0x520,
    SubCmd_MU_DoCustom			= 0x590,
    SubCmd_MU_End				= 0x9FF, //MU能够处理的命令终点
}eSubCmdID;

enum class TesterCmdID
{
    Program = 0x1800,
    Erase = 0x1801,
    Verify = 0x1802,
    BlankCheck = 0x1803,
    Secure = 0x1804,
    IllegalCheck = 0x1805,
    Read = 0x1806,
    ReadTesterUID = 0x1807,
    Custom = 0x1901,
};

struct TesterOperationConfig
{
    TesterCmdID cmdID;
    TesterSubCmdID subCmdID;
    std::string description;
    uint32_t parameters;
};

const std::vector<TesterOperationConfig> TesterOperationConfigs = {
    {TesterCmdID::Program, TesterSubCmdID::SubProgram, "Program operation", 0},
    {TesterCmdID::Erase, TesterSubCmdID::SubErase, "Erase operation", 0},
    {TesterCmdID::Verify, TesterSubCmdID::SubVerify, "Verify operation", 0},
    {TesterCmdID::Read, TesterSubCmdID::SubRead, "Read operation", 0},
    {TesterCmdID::ReadTesterUID, TesterSubCmdID::SubReadTesterUID, "Read Tester UID", 0},
};

struct MainBoardInfo {
    QString hardwareOEM;
    QString hardwareSN;
    QString hardwareUID;
    QString hardwareVersion;
};

struct DeviceInfo {
    int chainID;
    QString dpsFpgaVersion;
    QString dpsFwVersion;
    QString firmwareVersion;
    QString firmwareVersionDate;
    QString fpgaLocation;
    QString fpgaVersion;
    int hopNum;
    QString ip;
    bool isLastHop;
    int linkNum;
    QString mac;
    MainBoardInfo mainBoardInfo;
    QString muAppVersion;
    QString muAppVersionDate;
    QString muLocation;
    QString port;
    QString siteAlias;
    QString ipHop;

    void clear() {
        chainID = 0;
        dpsFpgaVersion.clear();
        dpsFwVersion.clear();
        firmwareVersion.clear();
        firmwareVersionDate.clear();
        fpgaLocation.clear();
        fpgaVersion.clear();
        hopNum = 0;
        ip.clear();
        isLastHop = false;
        linkNum = 0;
        mac.clear();
        mainBoardInfo = MainBoardInfo();
        muAppVersion.clear();
        muAppVersionDate.clear();
        muLocation.clear();
        port.clear();
        siteAlias.clear();
        ipHop.clear();
    }
};

struct ProjectInfo {
    int errorCode;
    QString message;
    QString details;

    void clear() {
        errorCode = 0;
        message.clear();
        details.clear();
    }
};

struct JobResult {
    QString BPUID;
    int SKTIdx;
    int nHopNum;
    QString result;
    QString strip;

    void clear() {
        BPUID.clear();
        SKTIdx = 0;
        nHopNum = 0;
        result.clear();
        strip.clear();
    }
};
#endif