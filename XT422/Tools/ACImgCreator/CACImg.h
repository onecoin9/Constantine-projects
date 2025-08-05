#pragma once
#include <stdint.h>
#include <QString>
#include <QVector>
#include <QFile>
#include "ILog.h"
#include <QTextCodec>
#pragma pack(1)
#define PACKED

typedef struct _tPartition {
    uint8_t SortIdx;
    uint8_t PartIdx;
    uint8_t IsAddrAdjoin;
    QString strName;
    QString strFileName;
    QString strParser;
    uint32_t FlashAddr; //分区在Flash中的位置
    uint32_t FileAddr;  //分区在文件中的位置
    uint32_t SizeMax;
    uint32_t SizeReal;
    uint16_t CRC16;
    void Init()
    {
        PartIdx = 0;
        IsAddrAdjoin = 0;
        strName = "";
        strFileName = "";
        strParser = "";
        FlashAddr = 0;
        FileAddr = 0;
        SizeMax = 0;
        SizeReal = 0;
        CRC16 = 0;
    }
}tPartition;

//整体大小为256个字节
#define PartInfoHeaderSize (256)
typedef struct _tPartInfoHeader {
    uint8_t UID[16];
    uint8_t PartNum;
    uint8_t Version[3];
    uint32_t Date;
    uint8_t Reserved0[8];
    uint8_t OEMSign[64];
    uint8_t ProductSign[64];
    uint16_t PartInfoSize;
    uint8_t Reserved1[92];
    uint16_t CRC16Sum;
}tPartInfoHeader;

//整体大小32个字节
#define PartInfoOnePartSize (32)
typedef struct _tPartInfoOnePart
{
    uint8_t  PartIndex;
    uint8_t  Reserved0;
    uint32_t FileOffset;  //分区在文件中的偏移位置
    uint32_t FlashAddr;   //分区在Flash中的位置
    uint32_t SizeReal;    //分区的实际字节数
    uint32_t SizeMax;     //分区的最大字节数
    uint16_t CRC16;       //分区实际字节数内容的CRC16值
    uint8_t Reserved[10];
    uint16_t CRC16Sum;
}tPartInfoOnePart;



#define ENVPART_DEFAULTSIZE  (2*1024)
#define ENVPART_ENTRYSIZEMAX  (256)
typedef struct _tEnvPartHeader {
    uint16_t EntryNum;          //条目数量
    uint16_t EntrySizeMax;      //单条环境变量的最大占用字符数，目前这个是256
    uint16_t DataSize;          //参数数据总共占字符数
    uint16_t DataCRC;           //参数数据的 CRC16 校验值
    uint8_t Reserved[22];       //预留
    uint16_t Checksum;          //前面所有字节 CRC16 校验值
    uint8_t Data[0];            //数据部分
}tEnvPartHeader;

typedef struct _tEnvPartItem {
    QString Key;
    QString Value;
    QString Type;
}tEnvPartItem;



#define CONFIGPART_DEFAULTSIZE  (2*1024)


#pragma pack()

class CACImg
{
public:
    void AttachILog(ILog* pILog)
    {
        m_pILog = pILog;
    }

    //创建ACImg
    //strPartsJsonFile： 分区表文件，里面的文件支持相对路径或绝对路径，相对路径是相对于与本分区表文件而言的，建议都放到同意文件夹下
    //成功返回 0， 失败返回负数
	int32_t CreateACImg(QString strPartsJsonFile,QString strVersion, QString strSPIPara);

protected:
    //加载对应的ini配置文件
    //strIniFile： 对应的INI文件路径
    //成功返回 0， 失败返回负数
    int32_t LoadConfig(QString strIniFile);

    //解析分区表配置文件
    //strCfgFile : 对应的分区表配置文件
    //成功返回 0， 失败返回负数
	int32_t ParsePartitionFile(QString strCfgFile);

    //创建ACMPImg文件
    //vParts: 分区信息向量
    //strFileOut ：输出的文件路径
    //成功返回 0， 失败返回负数
    int32_t CreateACMPImg(QVector<tPartition>& vParts, QString strFileOut);

    //解析分区信息
    //pOneParts: 一个分区信息
    //pFileOut ：信息输出的文件
    //成功返回 0， 失败返回负数
    int32_t ParsePartition(tPartition* pOneParts, QFile* pFileOut);

    //将Bit文件转为Bin文件并读取相关烧录数据
    //pOneParts： 一个分区信息
    //strBitFile： 待转换的bit文件的全路径
    //pFileOut:    解析后的信息被写入的文件
    //成功返回 0， 失败返回负数
    //备注：函数内部会修改pOneParts的SizeReal，表示该分区的实际数据量
    int32_t Bit2BinParser(tPartition* pOneParts,QString strBitFile,QFile* pFileOut);

    //读取Bin档案相关数据
    //pOneParts： 一个分区信息
    //strFile： 待转换的bit文件的全路径
    //pFileOut:    解析后的信息被写入的文件
    //成功返回 0， 失败返回负数
    //备注：函数内部会修改pOneParts的SizeReal，表示该分区的实际数据量
    int32_t BinParser(tPartition* pOneParts, QString strFile, QFile* pFileOut);

    //自动创建Tcl脚本，该脚本用来传递给Vivado的批处理文件，用来将bit转为bin档案
    //strTmpTclFile: Tcl文件的保存路径
    //BitFile : 解析的Bit文件的路径
    //BinFile : 输出的Bin文件的路径
    //成功返回 0， 失败返回负数
    int32_t CreateTmpTcl(QString strTmpTclFile, QString BitFile, QString BinFile);

    //打印所有分区表的信息
    void DumpParts(QVector<tPartition>& vParts);
    //确认分区表的分区是否有相互覆盖，如果有返回true，否则返回false，需要先将分区表的Flash地址按照从小到达排列
    bool IsPartOverlap(QVector<tPartition>& vParts);

    //自动调整分区表的Flash地址，这个要在分区信息中的IsAddrAdjoin设置为1的情况下才会自动对齐到前面一个分区表
    bool AdjustFlashAddr(QVector<tPartition>& vParts);


    int32_t CreateACUPImg(QVector<tPartition>& vParts, QString strFileOut);

    int32_t CheckDataStructSize();
    int32_t ReInitRes();
    int32_t ConstructPartInfoHeader(QVector<tPartition>& vParts);
    int32_t WritePartInfoPartitionInMPImg(QVector<tPartition>& vParts, QFile* pFileOut);
    int32_t WritePartInfoPartitionInUPImg(QVector<tPartition>& vParts, QFile* pFileOut);
    tPartition* FindPartInfoPartiton(QVector<tPartition>& vParts, bool isBackUpPart);


    int32_t CreatePartBin(const QString& partName, QString strBinFile);
    int32_t ParsePartJson(QString strJsonFile,QVector< tEnvPartItem>& vItems,const QString& rootNodeKey);
    int32_t ChangeEnvParaItemValueInAutoMode(tEnvPartItem& OneItem);

    //计算分区表分区需要的实际字节数
    uint32_t CalcPartInfoPartitionRealSize(QVector<tPartition>& vParts);
private:
    ILog* m_pILog;
    QVector<tPartition> m_Parts;   //从Json文件中提取到的分区表的信息
    QString m_PartsJsonFilePath;   //Json分区文件所在路径
    QString m_strVersion;          //固件版本号   
    QString m_strAppPath;          //应用程序所在路径

    ///分区信息表中的信息
    tPartInfoHeader m_PartInfoHeader;
    QVector<tPartInfoOnePart>m_vPartInfo;
    /// <summary>
    /// 下面来自设置
    /// </summary>
    uint32_t m_DefaultV;           //ini文件中的默认值
    QString m_MPImgExt;            //输出文件MPImg的后缀名
    QString m_UPImgExt;            //输出文件UPImg的后缀名
    bool m_bRemoveTmp;             //是否删除临时文件
    QString m_strVivadoPath;       //系统配置的Vivado属性，来自Windows的系统环境变量
    QTextCodec* m_pTextCodec;
};

