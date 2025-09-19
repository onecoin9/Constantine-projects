#include "CACImg.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QFileInfo>
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QTextCodec>
#include <QDir>
#include <QUuid>
#include "CRC.h"
#include <QTextCodec>

typedef enum _ePartIdx {
    PART_GOLDENBS = 1,
    PART_NORMALBS = 2,
    PART_PARTINFO = 3,
    PART_PARTINFOBK = 4,
    PART_ENVPARA = 5,
    PART_ENVPPARABK = 6,
    PART_CONFIG = 7,
    PART_CONFIGBK = 8,
    PART_SAFEFW = 9,
    PART_NORMALFW = 10,
    PART_DPSFW =11,
    PART_DPSFPGA = 12,
}ePartIdx;

typedef struct _tPartIdxMap {
    uint8_t PartIdx;
    QString strPartName;
}tPartIdxMap;


tPartIdxMap gPartIdxMap[] = {
    {PART_GOLDENBS,"GoldenBS"},
    {PART_NORMALBS,"NormalBS"},
    {PART_PARTINFO,"PartInfo"},
    {PART_PARTINFOBK,"PartInfoBK"},
    {PART_ENVPARA,"EnvPara"},
    {PART_ENVPPARABK,"EnvParaBK"},
    {PART_CONFIG,"Config"},
    {PART_CONFIGBK,"ConfigBK"},
    {PART_SAFEFW,"SafeFW"},
    {PART_NORMALFW,"NormalFW"},
    {PART_DPSFW,"DPSFW"},
    {PART_DPSFPGA,"DPSFPGA"},
    {0,""}
};


#define _PrintLog(fmt,...) \
    if (m_pILog) {\
		m_pILog->PrintLog(1, fmt, __VA_ARGS__);\
	}

#define _PrintErr(fmt,...) \
    if (m_pILog) {\
		m_pILog->PrintLog(3, fmt, __VA_ARGS__);\
	}


bool PartCompare(tPartition a, tPartition b)
{
    return a.SortIdx < b.SortIdx; //升序排列
}

#if 0
//Size需要为4的整数倍
uint32_t __CalcChecksum(uint8_t* pHeader, uint32_t Size)
{
    int32_t i = 0;
    uint32_t Checksum = 0;
    uint32_t* pTemp = (uint32_t*)pHeader;
    for (i = 0; i < Size / 4 ; ++i) {
        Checksum += *pTemp;
        pTemp++;
    }
    return (Checksum ^ 0xFFFFFFFF);
}

#endif 

#define Bytes2M(_Size) ((float)_Size/(1024*1024))
#define Bytes2K(_Size) ((float)_Size/(1024))

/*delete the files endswith ref
dir_name: the dir to remove files
ref: the Suffix of files need to be removed
*/
void RemoveFilesWithSuffix(QString& dir_name, QString& ref)
{
    if (dir_name.isEmpty() || ref.isEmpty())
        return;
    QDir dir(dir_name);
    //得到目录下的所有文件
    QFileInfoList infolist = dir.entryInfoList(QDir::Files, QDir::Time);
    //遍历获取的文件
    foreach(auto item, infolist){
        if (item.fileName().endsWith(ref)){
            QString strFilePath = item.absoluteFilePath();
            QFile::remove(strFilePath);
        }
    }
}

/*
* 给定一个分区表的名称确认分区索引
* 返回值：
*   true表示成功找到，false表示失败，成功时PartIdx可用
*/
bool CheckAndGetPartIdx(QString strPartName, uint8_t& PartIdx)
{
    tPartIdxMap* pPartIdx = gPartIdxMap;
    PartIdx = 0;
    while (pPartIdx->PartIdx != 0) {
        if (pPartIdx->strPartName == strPartName) {
            PartIdx = pPartIdx->PartIdx;
            return true;
        }
        pPartIdx++;
    }
    return false;
};

void CACImg::DumpParts(QVector<tPartition>& vParts)
{
    int32_t i;
    _PrintLog("=================Partition Information=============\r\n");
    _PrintLog("PartsTotal: %d\r\n", vParts.size());
    for (i = 0; i < vParts.size(); ++i) {
        tPartition& OnePart = vParts[i];
        _PrintLog("====>Partition[%d]\r\n", i);
        _PrintLog("PartName   : %s\r\n", OnePart.strName.toStdString().c_str());
        _PrintLog("FileName   : %s\r\n", OnePart.strFileName.toStdString().c_str());
        _PrintLog("SortIdx    : %d\r\n", OnePart.SortIdx);
        _PrintLog("PartIdx    : %d\r\n", OnePart.PartIdx);
        _PrintLog("IsAddrAdj  : %d\r\n", OnePart.IsAddrAdjoin);
        _PrintLog("FlashAddr  : 0x%08X  (%0.2f M, %0.2f K)\r\n", OnePart.FlashAddr, Bytes2M(OnePart.FlashAddr), Bytes2K(OnePart.FlashAddr));
        _PrintLog("FileAddr   : 0x%08X  (%0.2f M, %0.2f K)\r\n", OnePart.FileAddr, Bytes2M(OnePart.FileAddr), Bytes2K(OnePart.FileAddr));
        _PrintLog("SizeMax    : 0x%08X  (%0.2f M, %0.2f K)\r\n", OnePart.SizeMax, Bytes2M(OnePart.SizeMax), Bytes2K(OnePart.SizeMax));
        _PrintLog("SizeReal   : 0x%08X  (%0.2f M, %0.2f K)\r\n", OnePart.SizeReal, Bytes2M(OnePart.SizeReal), Bytes2K(OnePart.SizeReal));
        _PrintLog("CRC16      : 0x%04X\r\n", OnePart.CRC16);
        _PrintLog("Parser     : %s\r\n", OnePart.strParser.toStdString().c_str());
    }
    _PrintLog("===================================================\r\n");
}

/*
* 判断分区表是否有重叠
*/
bool CACImg::IsPartOverlap(QVector<tPartition>& vParts)
{
    int32_t i;
    uint32_t Offset=0;
    for (i = 0; i < vParts.size(); ++i) {
        tPartition& OnePart = vParts[i];
        if (Offset > OnePart.FlashAddr) {
            _PrintErr("Partition[%s] OverLap With Previous, FlashAddr:0x%08X\r\n", OnePart.strName.toStdString().c_str(),OnePart.FlashAddr);
            return true;
        }
        Offset = OnePart.FlashAddr + OnePart.SizeMax;
    }
    return false;
}

bool CACImg::AdjustFlashAddr(QVector<tPartition>& vParts)
{
    int32_t i;
    uint32_t Offset = 0;
    for (i = 0; i < vParts.size(); ++i) {
        tPartition& OnePart = vParts[i];
        if (OnePart.IsAddrAdjoin==1){
            OnePart.FlashAddr = Offset;
        }
        Offset = OnePart.FlashAddr + OnePart.SizeMax;
    }
    return false;
}

int32_t CACImg::ParsePartitionFile(QString strCfgFile)
{
    int32_t Ret = 0;
    bool Rtn;
    QJsonParseError err_rpt;
    QByteArray ByteArray;
    QFile File(strCfgFile);
    uint32_t Offset = 0;
    m_Parts.clear();
    Rtn=File.open(QIODevice::ReadOnly);
    if (Rtn == false) {
        _PrintErr("Open File Failed:%s\r\n", strCfgFile.toUtf8().toStdString().c_str());
        Ret = -1; goto __end;
    }
    else {
        ByteArray = File.readAll();
        QJsonDocument RootDoc = QJsonDocument::fromJson(ByteArray, &err_rpt); // 字符串格式化为JSON
        if (RootDoc.isEmpty() == true) {
            _PrintErr("Parse Json  Failed : %s\r\n", err_rpt.errorString().toUtf8().data());
            Ret = -1; goto __end;
        }
        QJsonValue Parts = RootDoc["Partitions"];
        if (Parts.isArray()) {
            int32_t i;
            QJsonArray JsonArray = Parts.toArray();
            for (i = 0; i < JsonArray.count(); ++i) {
                tPartition OnePart;
                bool bOK;
                QJsonValue JsonOnePart = JsonArray.at(i);
                OnePart.Init();
                OnePart.strName = JsonOnePart["Name"].toString();
                OnePart.strFileName= JsonOnePart["File"].toString();
                OnePart.SortIdx = JsonOnePart["SortIdx"].toInt();
                OnePart.IsAddrAdjoin = JsonOnePart["IsAddrAdjoin"].toInt();
                OnePart.strParser = JsonOnePart["Parser"].toString();
                if (OnePart.IsAddrAdjoin == 0) {//不自动对齐Flash地址，需要进行解析
                    OnePart.FlashAddr = JsonOnePart["FlashAddr"].toString().toUInt(&bOK, 16);
                    if (bOK == false) {
                        _PrintErr("Parse FlashAddr Failed, Not Valid Hex Data");
                        Ret = -1;  goto __end;
                    }
                }
                else {

                }
                OnePart.SizeMax = JsonOnePart["SizeMax"].toString().toUInt(&bOK,16);
                if (bOK == false) {
                    _PrintErr("Parse SizeMax Failed, Not Valid Hex Data");
                    Ret = -1;  goto __end;
                }
                if (CheckAndGetPartIdx(OnePart.strName, OnePart.PartIdx) == false) {
                    _PrintErr("PartName[%s] Is Not Support, Please Check!\r\n", OnePart.strName.toStdString().c_str());
                    Ret = -1; goto __end;
                }
                m_Parts.push_back(OnePart);
            }
        }
    }

    //根据FlashAddr进行排序
    std::sort(m_Parts.begin(), m_Parts.end(), PartCompare);
    
    AdjustFlashAddr(m_Parts);
    if (IsPartOverlap(m_Parts) == true) {
        Ret = -1; goto __end;
    }

__end:
    return Ret;
}

/*
* 在文件中添加指定的字节数的值
*/
bool AppendDefaultV(QFile*pFile,int32_t Size,uint8_t Value)
{
    bool Rtn = false;
    uint8_t* pData = new uint8_t[Size];
    if (pData) {
        memset(pData, Value, Size);
        if (pFile->write((char*)pData, (qint64)Size) != Size) {
            goto __end;
        }
        else {
            Rtn = true;
        }
        pFile->flush();
    }
    else {
        goto __end;
    }
__end:
    if (pData) {
        delete[] pData;
    }
    return Rtn;
}

int32_t CACImg::ParsePartition(tPartition* pOneParts, QFile* pFileOut)
{
    int32_t Ret = 0;
    QString strFilePath;
    if (pOneParts->strFileName.contains(":\\")) {//是一个绝对路径
        strFilePath = pOneParts->strFileName;
    }
    else {
        if (pOneParts->strFileName.contains("/") || pOneParts->strFileName.contains("\\")) {
            //是一个相对于与Partition.json文件的相对路径
            strFilePath = m_PartsJsonFilePath + "/" + pOneParts->strFileName;
        }
        else {
            strFilePath = m_strAppPath + "/tmpfile/" + pOneParts->strFileName;
        }
    }
    _PrintLog("PartName:%-24s, FilePath:%s\r\n", pOneParts->strName.toStdString().c_str(),strFilePath.toStdString().c_str());

    if (pOneParts->strParser == "Bit2Bin") {
        Ret = Bit2BinParser(pOneParts,strFilePath,pFileOut);
    }
    else if (pOneParts->strParser == "Binary") {
        Ret = BinParser(pOneParts, strFilePath, pFileOut);
    }
    return Ret;
}

int32_t CACImg::CreateTmpTcl(QString strTmpTclFile,QString BitFile,QString BinFile)
{
    int32_t Ret = 0;
    bool Rtn;
    QString str;
    QFile File(strTmpTclFile);
    QTextStream txtStream(&File);
    Rtn=File.open(QIODevice::Truncate | QIODevice::WriteOnly);
    if (Rtn == false) {
        Ret = -1; goto __end;
    }
    str.sprintf("write_cfgmem -format bin -size 64 -interface SPIx4 \
-loadbit { up 0x00000000 \"%s\" } \
-force -file \"%s\"", BitFile.toStdString().c_str(),BinFile.toStdString().c_str());

    txtStream << str;
    txtStream.flush();

    File.close();

__end:
    return Ret;
}

int32_t CACImg::Bit2BinParser(tPartition* pOneParts,QString strFile, QFile*pFileOut)
{
    int32_t Ret = 0;
    uint32_t FilePosBegin;
    QFile BinFile;
    QByteArray BinData;
    QFileInfo FileInfo(strFile);
    QString strFileName=FileInfo.baseName();
    QString strFilePath = FileInfo.path();
    QString strBinName = strFilePath +"/"+strFileName + ".bin";
    QString strTmpTclFile = strFilePath + "/"+ strFileName+"_tmpfile.tcl";
    QString strExecCmd;
    Ret = CreateTmpTcl(strTmpTclFile, strFile, strBinName);

    strExecCmd.sprintf("%s\\bin\\vivado.bat -mode batch -source %s", m_strVivadoPath.toStdString().c_str(), strTmpTclFile.toStdString().c_str());
    Ret=QProcess::execute(strExecCmd);
    if (Ret != 0) {
        _PrintErr("Exec VivadoBat Failed, RtnCode:%d\r\n", Ret);
        goto __end;
    }
    BinFile.setFileName(strBinName);
    BinFile.setPermissions(QFileDevice::ReadGroup);
    if (BinFile.open(QIODevice::ReadOnly) == false) {
        Ret = -1; _PrintErr("Open File Failed: %s\r\n", strBinName.toStdString().c_str());
        goto __end;
    }

    BinData=BinFile.readAll();

    

    FilePosBegin = (uint32_t)pFileOut->pos();
    _PrintLog("=====>Write At FileAddr:0x%08X\r\n", FilePosBegin);
    pFileOut->write(BinData);
    pFileOut->flush();
    pOneParts->SizeReal = BinData.size();
    pOneParts->CRC16 = 0;
    calc_crc16sum((uint8_t*)BinData.constData(), pOneParts->SizeReal, &pOneParts->CRC16);
    
    //删除中间文件,TCL脚本和生成的Bin
    if (m_bRemoveTmp) {
        QFile::remove(strTmpTclFile);
        QFile::remove(strBinName);
    }
    

__end:
    return Ret;
}

int32_t CACImg::BinParser(tPartition* pOneParts, QString strFile, QFile* pFileOut)
{
    int32_t Ret = 0;
    uint32_t FilePosBegin = 0;
    QFile BinFile;
    QByteArray BinData;
    BinFile.setFileName(strFile);
    BinFile.setPermissions(QFileDevice::ReadGroup);
    if (BinFile.open(QIODevice::ReadOnly) == false) {
        Ret = -1; 
        //QString ErrMsg = "===错误===";
        _PrintErr("Open File Failed: %s\r\n", strFile.toStdString().c_str());
        //_PrintErr("%s\r\n", m_pTextCodec->fromUnicode(ErrMsg).data());
        //_PrintErr("ErrMsg : %s\r\n", m_pTextCodec->tol(BinFile.errorString().tou).toUtf8().data());
        goto __end;
    }

    BinData = BinFile.readAll();

    FilePosBegin = (uint32_t)pFileOut->pos();
    _PrintLog("=====>Write At FileAddr:0x%08X\r\n", FilePosBegin);
    pFileOut->write(BinData);
    pFileOut->flush();
    pOneParts->SizeReal = BinData.size();
    pOneParts->CRC16 = 0;
    calc_crc16sum((uint8_t*)BinData.constData(), pOneParts->SizeReal, &pOneParts->CRC16);
__end:
    return Ret;
}

/*
* isBackUpPart 为true表示要查找一个备份分区
*/
tPartition* CACImg::FindPartInfoPartiton(QVector<tPartition>& vParts,bool isBackUpPart)
{
    int32_t Cnt = vParts.size();
    int32_t i;
    QString strPartName;
    if (isBackUpPart) {
        strPartName = "PartInfoBK";
    }
    else {
        strPartName = "PartInfo";
    }
    for (i = 0; i < Cnt; ++i) {
        if (vParts[i].strName == strPartName) {
            return &vParts[i];
        }
    }
    return NULL;
}

uint32_t CACImg::CalcPartInfoPartitionRealSize(QVector<tPartition>& vParts)
{
    uint32_t RealSize = 0;
    RealSize += sizeof(tPartInfoHeader);
    RealSize += vParts.size() * sizeof(tPartInfoOnePart);
    return RealSize;
}

int32_t CACImg::WritePartInfoPartitionInMPImg(QVector<tPartition>& vParts,QFile*pFileOut)
{
    int32_t Ret = 0;
    int32_t i;
    uint32_t FileCurOffset = 0;
    tPartition* pPart = NULL;
    uint32_t PartInfoPartitonRealSize = 0;
    FileCurOffset = pFileOut->pos();
    ReInitRes();
    ConstructPartInfoHeader(vParts); 
    for (i = 0; i < vParts.size(); ++i) {
        tPartInfoOnePart OnePart;
        memset(&OnePart, 0, sizeof(tPartInfoOnePart));
        OnePart.CRC16=vParts[i].CRC16;
        OnePart.FlashAddr = vParts[i].FlashAddr;
        OnePart.PartIndex = vParts[i].PartIdx;
        OnePart.SizeMax = vParts[i].SizeMax;
        OnePart.SizeReal = vParts[i].SizeReal;
        OnePart.FileOffset = vParts[i].FileAddr;
        calc_crc16sum((uint8_t*)&OnePart, sizeof(tPartInfoOnePart) - 2, &OnePart.CRC16Sum);
        m_vPartInfo.push_back(OnePart);
    }

    /// <summary>
    /// 写PartInfo区域
    /// </summary>
    /// <param name="vParts"></param>
    /// <param name="pFileOut"></param>
    /// <returns></returns>
    pPart=FindPartInfoPartiton(vParts, false);
    pPart->SizeReal = CalcPartInfoPartitionRealSize(vParts);
    if (pPart == NULL) {
        _PrintErr("Can't Find PartInfo Partition\r\n");
        Ret = -1; goto __end;
    }
    _PrintLog("Write PartInfo   FileAddr:0x%08X, RealSize:0x%08X...\r\n", pPart->FileAddr, pPart->SizeReal);
    pFileOut->seek(pPart->FileAddr);//先切换到分区信息位置
    pFileOut->write((char*)&m_PartInfoHeader, sizeof(tPartInfoHeader));//写头部
    for (i = 0; i < m_vPartInfo.size(); ++i) {
        tPartInfoOnePart& OnePart = m_vPartInfo[i];
        pFileOut->write((char*)&OnePart, sizeof(tPartInfoOnePart));
    }
    /// <summary>
    /// 写PartInfoBK区域
    /// </summary>
    /// <param name="vParts"></param>
    /// <param name="pFileOut"></param>
    /// <returns></returns>
    pPart = FindPartInfoPartiton(vParts, true);
    pPart->SizeReal = CalcPartInfoPartitionRealSize(vParts);
    if (pPart == NULL) {
        _PrintErr("Can't Find PartInfoBK Partition\r\n");
        Ret = -1; goto __end;
    }
    _PrintLog("Write PartInfoBK FileAddr:0x%08X, RealSize:0x%08X...\r\n", pPart->FileAddr, pPart->SizeReal);
    pFileOut->seek(pPart->FileAddr);//先切换到分区信息位置
    pFileOut->write((char*)&m_PartInfoHeader, sizeof(tPartInfoHeader));//写头部
    for (i = 0; i < m_vPartInfo.size(); ++i) {//写分区信息
        tPartInfoOnePart& OnePart = m_vPartInfo[i];
        pFileOut->write((char*)&OnePart, sizeof(tPartInfoOnePart));
    }

__end:
    pFileOut->seek(FileCurOffset);//恢复到写之前的位置
    return Ret;
}


int32_t CACImg::WritePartInfoPartitionInUPImg(QVector<tPartition>& vParts, QFile* pFileOut)
{
    int32_t i;
    int32_t Ret = 0;
    tPartition* pPart = NULL;
    for (i = 0; i < vParts.size(); ++i) {
        tPartInfoOnePart OnePart;
        memset(&OnePart, 0, sizeof(tPartInfoOnePart));
        OnePart.CRC16 = vParts[i].CRC16;
        OnePart.FlashAddr = vParts[i].FlashAddr;
        OnePart.PartIndex = vParts[i].PartIdx;
        OnePart.SizeMax = vParts[i].SizeMax;
        OnePart.SizeReal = vParts[i].SizeReal;
        OnePart.FileOffset = vParts[i].FileAddr; //这个时候FileAddr还未固定
        calc_crc16sum((uint8_t*)&OnePart, sizeof(tPartInfoOnePart) - 2, &OnePart.CRC16Sum);
        m_vPartInfo.push_back(OnePart);
    }
    pPart = FindPartInfoPartiton(vParts, false);
    if (pPart == NULL) {
        _PrintErr("Can't Find PartInfo Partition\r\n");
        Ret = -1; goto __end;
    }
    _PrintLog("Write PartInfo   FileAddr:0x%08X,RealSize:0x%08X...\r\n",pPart->FileAddr,pPart->SizeReal);
    pFileOut->seek(pPart->FileAddr);//先切换到分区信息位置
    pFileOut->write((char*)&m_PartInfoHeader, sizeof(tPartInfoHeader));//写头部
    for (i = 0; i < m_vPartInfo.size(); ++i) {
        tPartInfoOnePart& OnePart = m_vPartInfo[i];
        pFileOut->write((char*)&OnePart, sizeof(tPartInfoOnePart));//写分区信息，但这里后面还要更新
    }

    pPart = FindPartInfoPartiton(vParts, true);
    if (pPart == NULL) {
        _PrintErr("Can't Find PartInfoBK Partition\r\n");
        Ret = -1; goto __end;
    }
    _PrintLog("Write PartInfoBK FileAddr:0x%08X,RealSize:0x%08X...\r\n", pPart->FileAddr, pPart->SizeReal);
    pFileOut->seek(pPart->FileAddr);//先切换到分区信息位置
    pFileOut->write((char*)&m_PartInfoHeader, sizeof(tPartInfoHeader));//写头部
    for (i = 0; i < m_vPartInfo.size(); ++i) {
        tPartInfoOnePart& OnePart = m_vPartInfo[i];
        pFileOut->write((char*)&OnePart, sizeof(tPartInfoOnePart));//写分区信息，但这里后面还要更新
    }


    //头部是需要放置的，这个地方由AG06访问
    _PrintLog("Update UPImg Header And Entrys...\r\n");
    pPart->FileAddr = 0; //PartInfo在UPImg里放在头部
    pFileOut->seek(pPart->FileAddr);//先切换到分区信息位置
    pFileOut->write((char*)&m_PartInfoHeader, sizeof(tPartInfoHeader));//写头部
    for (i = 0; i < m_vPartInfo.size(); ++i) {
        tPartInfoOnePart& OnePart = m_vPartInfo[i];
        pFileOut->write((char*)&OnePart, sizeof(tPartInfoOnePart));//写分区信息，但这里后面还要更新
    }

    //接下来写入实际的分区数据，这样驱动就不需要访问

__end:
    return Ret;
}

/*
* 创建量产的ACMPImg
*/
int32_t CACImg::CreateACMPImg(QVector<tPartition>&vParts,QString strFileOut)
{
    int32_t Ret = 0,i;
    bool RtnCall;
    int32_t FlashOffset = 0;
    uint32_t AppendSize = 0;
    tPartition* pLastPart;
    QFile FileOut(strFileOut);
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
    if (FileOut.open(QIODevice::WriteOnly | QIODevice::Truncate) == false) {
        QString strErrMsg = FileOut.errorString();
        _PrintErr("Create File Failed:%s\r\n", strFileOut.toUtf8().toStdString().c_str());
        _PrintErr("ErrMsg:%s\r\n", strErrMsg.toUtf8().toStdString().c_str());
        Ret = -1; goto __end;
    }
    for (i = 0; i < m_Parts.size(); ++i) {
        tPartition & OneParts = vParts[i];
        if (OneParts.FlashAddr > FlashOffset) {
            //发现中间有空隙则用空白值填充
            AppendSize = OneParts.FlashAddr - FlashOffset;
            RtnCall = AppendDefaultV(&FileOut, AppendSize, (uint8_t)m_DefaultV);
            if (RtnCall == false) {
                goto __end;
            }
        }

        //这里保存分区在文件中的起始位置，后面准备解析档案并开始写入
        OneParts.FileAddr = FileOut.pos();
        if (OneParts.strFileName != "") {//有档案需要载入
            Ret = ParsePartition(&OneParts, &FileOut);
            if (Ret != 0) {
                goto __end;
            }
        }
        FlashOffset = (int32_t)FileOut.pos();
    }

    pLastPart=&vParts[i-1];
    if (FlashOffset < pLastPart->FlashAddr + pLastPart->SizeMax) {//最后一个没有写满，需要补齐
        AppendSize = pLastPart->FlashAddr + pLastPart->SizeMax - FlashOffset;
        RtnCall = AppendDefaultV(&FileOut, AppendSize, (uint8_t)m_DefaultV);
        if (RtnCall == false) {
            goto __end;
        }
    }

    //需要开始补充PartInfo和PartInfoBackup
    Ret= WritePartInfoPartitionInMPImg(vParts, &FileOut);
    if (Ret != 0) {
        goto __end;
    }

    FileOut.close();

__end:
    return Ret;
}
int32_t CACImg::LoadConfig(QString strIniFile)
{
    int32_t Ret = 0;
    bool bOK;
    QSettings AppSetting(strIniFile, QSettings::IniFormat);
    if (AppSetting.status() != QSettings::NoError) {
        _PrintErr("Parse IniFile Failed\r\n");
        Ret = -2; goto __end;
    }

    m_DefaultV = AppSetting.value("Config/DefaultV", "0xFF").toString().toUInt(&bOK,16);
    m_MPImgExt = AppSetting.value("Config/MPImgExt", "mpimg").toString();
    m_UPImgExt = AppSetting.value("Config/UPImgExt", "mpimg").toString();
    m_bRemoveTmp = AppSetting.value("Config/RemoveTmp",1).toUInt();
    m_strVivadoPath = QProcessEnvironment::systemEnvironment().value("VIVADO_HOME");

    _PrintLog("--------------Config---------------\r\n");
    _PrintLog("DefautV   : 0x%02X\r\n", m_DefaultV);
    _PrintLog("MPImgExt  : %s\r\n", m_MPImgExt.toStdString().c_str());
    _PrintLog("Vivado Dir: %s\r\n", m_strVivadoPath.toStdString().c_str());
    _PrintLog("-----------------------------------\r\n");
__end:
    return Ret;
}

/*
* 创建ACUPImg文件，
*/
#define FileAlignSize (4096)
int32_t CACImg::CreateACUPImg(QVector<tPartition>& vParts, QString strFileOut)
{
    int32_t Ret = 0;
    int32_t i = 0;
    uint32_t PartInfoPartitionRealSize = 0;
    int32_t FlashOffset;
    QFile FileOut(strFileOut);
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
    if (FileOut.open(QIODevice::WriteOnly | QIODevice::Truncate) == false) {
        QString strErrMsg = FileOut.errorString();
        _PrintErr("Create File Failed:%s\r\n", strFileOut.toUtf8().toStdString().c_str());
        _PrintErr("ErrMsg:%s\r\n", strErrMsg.toUtf8().toStdString().c_str());
        Ret = -1; goto __end;
    }

    ReInitRes();
    ConstructPartInfoHeader(vParts);//构造头部

    PartInfoPartitionRealSize = sizeof(tPartInfoHeader) + m_Parts.size() * sizeof(tPartInfoOnePart);
    //先为头部和Entry保留
    AppendDefaultV(&FileOut, PartInfoPartitionRealSize, (uint8_t)m_DefaultV);

    for (i = 0; i < m_Parts.size(); ++i) {
        tPartition& OneParts = vParts[i];
        //这里保存分区在文件中的起始位置，后面准备解析档案并开始写入
        OneParts.FileAddr = FileOut.pos();
        if (OneParts.strName == "PartInfo" || OneParts.strName == "PartInfoBK") {//这里预先写入默认值形成占位
            OneParts.SizeReal = PartInfoPartitionRealSize;
            AppendDefaultV(&FileOut, PartInfoPartitionRealSize, (uint8_t)m_DefaultV);
            continue;
        }
        if (OneParts.strFileName != "") {//有档案需要载入,
            Ret = ParsePartition(&OneParts, &FileOut);
            if (Ret != 0) {
                goto __end;
            }
        }
    }

    //写入实际的头部和Entry信息
    Ret = WritePartInfoPartitionInUPImg(vParts, &FileOut);
    if (Ret != 0) {
        goto __end;
    }

    if (FileOut.size() % FileAlignSize) {//不是1K的整数倍则则进行补充
        uint32_t AppendSize = FileAlignSize - (FileOut.size() % FileAlignSize);
        FileOut.seek(FileOut.size());
        AppendDefaultV(&FileOut, AppendSize, (uint8_t)m_DefaultV);
        _PrintLog("Append %d Byte to align %d Size\r\n", AppendSize, FileAlignSize);
    }


    FileOut.close();
__end:
    return Ret;
}

int32_t CACImg::CheckDataStructSize()
{
    int32_t Ret = 0;
    if (sizeof(tPartInfoHeader) != PartInfoHeaderSize) {
        _PrintErr("tPartInfoHeader Size Must Be %d, Current:%d\r\n", PartInfoHeaderSize,sizeof(tPartInfoHeader));
        Ret = -1; goto __end;
    }


    if (sizeof(tPartInfoOnePart) != PartInfoOnePartSize) {
        _PrintErr("tPartInfoOnePart Size Must Be %d, Current:%d\r\n", PartInfoOnePartSize, sizeof(tPartInfoOnePart));
        Ret = -1; goto __end;
    }

__end:
    return Ret;
}

int32_t CACImg::ReInitRes()
{
    int32_t Ret = 0;
    memset(&m_PartInfoHeader, 0, sizeof(tPartInfoHeader));
    m_vPartInfo.clear();
    return Ret;
}

int32_t CACImg::ChangeEnvParaItemValueInAutoMode(tEnvPartItem& OneItem)
{
    int32_t Ret = 0;
    if (OneItem.Key.compare("FWVersion") == 0) {
        OneItem.Value = m_strVersion;
    }
    else {

    }

    return Ret;
}

int32_t CACImg::ParsePartJson(QString strJsonFile, QVector< tEnvPartItem>& vItems,const QString& rootNodeKey)
{
    int32_t Ret = 0;
    bool Rtn;
    QJsonParseError err_rpt;
    QByteArray ByteArray;
    QFile File(strJsonFile);
    vItems.clear();
    Rtn = File.open(QIODevice::ReadOnly);
    if (Rtn == false) {
        _PrintErr("Open File Failed:%s\r\n", strJsonFile.toUtf8().toStdString().c_str());
        Ret = -1; goto __end;
    }
    else {
        ByteArray = File.readAll();
        QJsonDocument RootDoc = QJsonDocument::fromJson(ByteArray, &err_rpt); // 字符串格式化为JSON
        if (RootDoc.isEmpty() == true) {
            _PrintErr("Parse Json  Failed : %s\r\n", err_rpt.errorString().toUtf8().data());
            Ret = -1; goto __end;
        }


        QJsonValue EnvItems = RootDoc[rootNodeKey];
        if (EnvItems.isArray()) {
            int32_t i;
            QJsonArray JsonArray = EnvItems.toArray();
            for (i = 0; i < JsonArray.count(); ++i) {
                tEnvPartItem OneItem;
                bool bOK;
                QJsonValue JsonOneItem = JsonArray.at(i);

                OneItem.Key= JsonOneItem["Key"].toString();
                OneItem.Value = JsonOneItem["Value"].toString();
                OneItem.Type = JsonOneItem["Type"].toString();
                
                if (OneItem.Type.compare("AUTO", Qt::CaseInsensitive) == 0) {//为AUTO模式
                    ChangeEnvParaItemValueInAutoMode(OneItem);
                }
                vItems.push_back(OneItem);
            }
        }
        else {
            _PrintErr("Parse Json  Failed : Can not find Key \"%s\"\r\n", rootNodeKey.toUtf8().data());
            Ret = -1; goto __end;
        }
    }

__end:
    return Ret;
}

int32_t CACImg::CreatePartBin(const QString& partName, QString strBinFile)
{
    int32_t Ret = 0,i;
    bool Rtn;
    tEnvPartHeader* pHeader = NULL;
    uint8_t* pPartData = NULL;
    QVector<tEnvPartItem> Items;
    QFile File;

    int32_t DataSize = 0;
    int partDefaultSize = 0;

    QString strData;
    QString strAppPath = QCoreApplication::applicationDirPath();
    QString strPartJsonPath = strAppPath + "/" + partName + ".json";
    QString strPartBinPath = strAppPath + "/tmpfile/" + partName + ".bin";
    QString logStr;

    QString partNameBk = partName;


    if (partName.compare("EnvPart") != 0 && partName.compare("ConfigPart") != 0) {
        return -1; goto __end;
    }

    logStr = "==========Create" + partName + " Binary File=========\r\n";
    _PrintLog(logStr.toStdString().c_str());
    if (partName.compare("EnvPart") == 0)
        partDefaultSize = ENVPART_DEFAULTSIZE;
    else if (partName.compare("ConfigPart") == 0)
        partDefaultSize = CONFIGPART_DEFAULTSIZE;


    DataSize = partDefaultSize - sizeof(tEnvPartHeader);

    pPartData = new uint8_t[partDefaultSize];
    if (!pPartData) {
        _PrintErr("Malloc Failed, Bytes:%d\r\n", partDefaultSize);
        Ret = -1; goto __end;
    }

    memset(pPartData, 0, partDefaultSize);
    pHeader = (tEnvPartHeader*)pPartData;


    Ret=ParsePartJson(strPartJsonPath, Items, partNameBk.replace("Part", "Info"));
    if (Ret != 0) {
        goto __end;
    }

    for (i = 0; i < Items.size(); ++i) {
        strData += QString::asprintf("%s:%s", Items[i].Key.toUtf8().data(),
            Items[i].Value.toUtf8().data());
        if (i != Items.size() - 1) {//不是最后一个，增加分割符号";"
            strData += ";";
        }
    }

    pHeader->EntryNum = Items.size();
    pHeader->EntrySizeMax = ENVPART_ENTRYSIZEMAX;
    pHeader->DataSize = strData.length();
    sprintf((char*)pHeader->Data, "%s", strData.toUtf8().data());
    calc_crc16sum(pHeader->Data, pHeader->DataSize, &pHeader->DataCRC);
    calc_crc16sum((uint8_t*)pHeader, sizeof(tEnvPartHeader) - 2, &pHeader->Checksum);

    _PrintLog("EntryNum     : %d\r\n", pHeader->EntryNum);
    _PrintLog("EntrySizeMax : %d\r\n", pHeader->EntrySizeMax);
    _PrintLog("DataSize     : 0x%X\r\n", pHeader->DataSize);
    _PrintLog("DataCRC      : 0x%04X\r\n", pHeader->DataCRC);
    _PrintLog("Checksum     : 0x%04X\r\n", pHeader->Checksum);
    _PrintLog("PartBin   : %s\r\n", strPartBinPath.toUtf8().data());

    File.setFileName(strPartBinPath);
    File.setPermissions(QFile::WriteGroup | QFile::WriteOther);
    Rtn=File.open(QIODevice::ReadWrite);
    if (Rtn == false) {
        _PrintErr("Open BinFile Failed:%s\r\n", strPartBinPath.toUtf8().data());
        _PrintErr("ErrMsg: %s\r\n", File.errorString().toUtf8().data());
        Ret = -1; goto __end;
    }

    File.write((char*)pPartData, partDefaultSize);
    File.flush();
    File.close();

__end:

    if (pPartData) {
        delete[] pPartData;
    }

    if (Ret == 0) {
        logStr = "====" + partName + ".bin Create PASS!\r\n";
        _PrintLog(logStr.toStdString().c_str());
    }
    else {
        logStr = "====" + partName + ".bin Create FAIL!\r\n";
        _PrintLog(logStr.toStdString().c_str());
    }

    return Ret;
}




int32_t CACImg::ConstructPartInfoHeader(QVector<tPartition>& vParts)
{
    int32_t Ret = 0;
    int32_t RtnCall;
    int32_t V1, V2, V3;
    uint32_t Year, Month, Day;
    tPartInfoHeader* pPartInfoHeader = &m_PartInfoHeader;
    QUuid guid = QUuid::createUuid();
    QDate CurDate = QDate::currentDate();
    memcpy(pPartInfoHeader->UID, guid.toRfc4122().constData(), 16);

    RtnCall =sscanf(m_strVersion.toStdString().c_str(), "%d.%d.%d", &V1, &V2, &V3);
    if (RtnCall != 3) {
        Ret = -1; goto __end;
    }
    
    pPartInfoHeader->PartNum = vParts.size();
    pPartInfoHeader->Version[0] = (uint8_t)V1;
    pPartInfoHeader->Version[1] = (uint8_t)V2;
    pPartInfoHeader->Version[2] = (uint8_t)V3;

    pPartInfoHeader->Date=(CurDate.year() << 16) | CurDate.month() << 8 | CurDate.day();
    pPartInfoHeader->PartInfoSize = sizeof(tPartInfoOnePart);


    calc_crc16sum((uint8_t*)pPartInfoHeader, sizeof(tPartInfoHeader) - 2, &pPartInfoHeader->CRC16Sum);

    _PrintLog("-------------------PartInfo Header-----------------\r\n");
    _PrintLog("UID       : %s\r\n", guid.toString().toUpper().toStdString().c_str());
    _PrintLog("Version   : %s \r\n", m_strVersion.toStdString().c_str());
    _PrintLog("Date      : %s\r\n", CurDate.toString(Qt::DateFormat::ISODate).toStdString().c_str());
    _PrintLog("---------------------------------------------------\r\n");
__end:
    return Ret;
}

int32_t CACImg::CreateACImg(QString strPartsJsonFile,QString strVersion, QString strSPIPara)
{
    int32_t Ret = 0;
    int32_t i;
    QFileInfo FileInfo = QFileInfo(strPartsJsonFile);
    QString strAppPath = QCoreApplication::applicationDirPath();
    QString strIniPath = strAppPath + "/Config.ini";
    QString strACMPImgPath;
    QString strACUPImgPath;
    m_pTextCodec = QTextCodec::codecForLocale();
    m_strAppPath = strAppPath;
    //printf("Sizeof(tPartInfoHeader):%d\r\n", sizeof(tPartInfoHeader));
    
    Ret = CheckDataStructSize();
    if (Ret != 0) {
        goto __end;
    }

    //先加载配置文件
    Ret=LoadConfig(strIniPath);
    if (Ret != 0) {
        goto __end;
    }
    m_PartsJsonFilePath=FileInfo.path();
    m_strVersion = strVersion;

    strACMPImgPath = m_PartsJsonFilePath + "/AP9900Firmware."+m_MPImgExt;
    strACUPImgPath = m_PartsJsonFilePath + "/AP9900Firmware." + m_UPImgExt;

    if (strSPIPara.isEmpty())
    {
        strSPIPara = "-all";
    }


    Ret=CreatePartBin("EnvPart", "");
    if (Ret != 0) {
        goto __end;
    }
    Ret = CreatePartBin("ConfigPart", "");
    if (Ret != 0) {
        goto __end;
    }

    //解析分区表
    _PrintLog("---------------------Paramters---------------------\r\n");
    _PrintLog("PartitonTable: %s\r\n", strPartsJsonFile.toStdString().c_str());
    _PrintLog("OutputFile   : %s\r\n", strACMPImgPath.toStdString().c_str());
    _PrintLog("Version      : %s\r\n", strVersion.toStdString().c_str());
    _PrintLog("---------------------------------------------------\r\n");
    Ret = ParsePartitionFile(strPartsJsonFile);
    if (Ret != 0){
        goto __end;
    }

    //创建量产文件ACMPIMG
    if (strSPIPara == "-bin" || strSPIPara == "-all") {
        _PrintLog("=====>Creating MPImg(.mpimg)...\r\n");
        Ret = CreateACMPImg(m_Parts, strACMPImgPath);
        if (Ret == 0) {
            DumpParts(m_Parts);
        }
    }
    
    _PrintLog("\r\n\r\n\r\n");
    if (strSPIPara == "-fwm" || strSPIPara == "-all") {
        _PrintLog("=====>Creating UPImg(.fwmx)...\r\n");
        Ret = CreateACUPImg(m_Parts, strACUPImgPath);
        if (Ret == 0) {
            DumpParts(m_Parts);
        }
    }

    //删除Vivado产生的临时文件 
    RemoveFilesWithSuffix(strAppPath,QString("jou"));
    RemoveFilesWithSuffix(strAppPath, QString("log"));

__end:
    if (Ret == 0) {
        _PrintLog("=====>Creating Images PASS\r\n");
    }
    else {
        _PrintLog("=====>Creating Images FAIL\r\n");
    }
    return Ret;
}
