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
    uint32_t FlashAddr; //������Flash�е�λ��
    uint32_t FileAddr;  //�������ļ��е�λ��
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

//�����СΪ256���ֽ�
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

//�����С32���ֽ�
#define PartInfoOnePartSize (32)
typedef struct _tPartInfoOnePart
{
    uint8_t  PartIndex;
    uint8_t  Reserved0;
    uint32_t FileOffset;  //�������ļ��е�ƫ��λ��
    uint32_t FlashAddr;   //������Flash�е�λ��
    uint32_t SizeReal;    //������ʵ���ֽ���
    uint32_t SizeMax;     //����������ֽ���
    uint16_t CRC16;       //����ʵ���ֽ������ݵ�CRC16ֵ
    uint8_t Reserved[10];
    uint16_t CRC16Sum;
}tPartInfoOnePart;



#define ENVPART_DEFAULTSIZE  (2*1024)
#define ENVPART_ENTRYSIZEMAX  (256)
typedef struct _tEnvPartHeader {
    uint16_t EntryNum;          //��Ŀ����
    uint16_t EntrySizeMax;      //�����������������ռ���ַ�����Ŀǰ�����256
    uint16_t DataSize;          //���������ܹ�ռ�ַ���
    uint16_t DataCRC;           //�������ݵ� CRC16 У��ֵ
    uint8_t Reserved[22];       //Ԥ��
    uint16_t Checksum;          //ǰ�������ֽ� CRC16 У��ֵ
    uint8_t Data[0];            //���ݲ���
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

    //����ACImg
    //strPartsJsonFile�� �������ļ���������ļ�֧�����·�������·�������·����������뱾�������ļ����Եģ����鶼�ŵ�ͬ���ļ�����
    //�ɹ����� 0�� ʧ�ܷ��ظ���
	int32_t CreateACImg(QString strPartsJsonFile,QString strVersion, QString strSPIPara);

protected:
    //���ض�Ӧ��ini�����ļ�
    //strIniFile�� ��Ӧ��INI�ļ�·��
    //�ɹ����� 0�� ʧ�ܷ��ظ���
    int32_t LoadConfig(QString strIniFile);

    //���������������ļ�
    //strCfgFile : ��Ӧ�ķ����������ļ�
    //�ɹ����� 0�� ʧ�ܷ��ظ���
	int32_t ParsePartitionFile(QString strCfgFile);

    //����ACMPImg�ļ�
    //vParts: ������Ϣ����
    //strFileOut ��������ļ�·��
    //�ɹ����� 0�� ʧ�ܷ��ظ���
    int32_t CreateACMPImg(QVector<tPartition>& vParts, QString strFileOut);

    //����������Ϣ
    //pOneParts: һ��������Ϣ
    //pFileOut ����Ϣ������ļ�
    //�ɹ����� 0�� ʧ�ܷ��ظ���
    int32_t ParsePartition(tPartition* pOneParts, QFile* pFileOut);

    //��Bit�ļ�תΪBin�ļ�����ȡ�����¼����
    //pOneParts�� һ��������Ϣ
    //strBitFile�� ��ת����bit�ļ���ȫ·��
    //pFileOut:    ���������Ϣ��д����ļ�
    //�ɹ����� 0�� ʧ�ܷ��ظ���
    //��ע�������ڲ����޸�pOneParts��SizeReal����ʾ�÷�����ʵ��������
    int32_t Bit2BinParser(tPartition* pOneParts,QString strBitFile,QFile* pFileOut);

    //��ȡBin�����������
    //pOneParts�� һ��������Ϣ
    //strFile�� ��ת����bit�ļ���ȫ·��
    //pFileOut:    ���������Ϣ��д����ļ�
    //�ɹ����� 0�� ʧ�ܷ��ظ���
    //��ע�������ڲ����޸�pOneParts��SizeReal����ʾ�÷�����ʵ��������
    int32_t BinParser(tPartition* pOneParts, QString strFile, QFile* pFileOut);

    //�Զ�����Tcl�ű����ýű��������ݸ�Vivado���������ļ���������bitתΪbin����
    //strTmpTclFile: Tcl�ļ��ı���·��
    //BitFile : ������Bit�ļ���·��
    //BinFile : �����Bin�ļ���·��
    //�ɹ����� 0�� ʧ�ܷ��ظ���
    int32_t CreateTmpTcl(QString strTmpTclFile, QString BitFile, QString BinFile);

    //��ӡ���з��������Ϣ
    void DumpParts(QVector<tPartition>& vParts);
    //ȷ�Ϸ�����ķ����Ƿ����໥���ǣ�����з���true�����򷵻�false����Ҫ�Ƚ��������Flash��ַ���մ�С��������
    bool IsPartOverlap(QVector<tPartition>& vParts);

    //�Զ������������Flash��ַ�����Ҫ�ڷ�����Ϣ�е�IsAddrAdjoin����Ϊ1������²Ż��Զ����뵽ǰ��һ��������
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

    //��������������Ҫ��ʵ���ֽ���
    uint32_t CalcPartInfoPartitionRealSize(QVector<tPartition>& vParts);
private:
    ILog* m_pILog;
    QVector<tPartition> m_Parts;   //��Json�ļ�����ȡ���ķ��������Ϣ
    QString m_PartsJsonFilePath;   //Json�����ļ�����·��
    QString m_strVersion;          //�̼��汾��   
    QString m_strAppPath;          //Ӧ�ó�������·��

    ///������Ϣ���е���Ϣ
    tPartInfoHeader m_PartInfoHeader;
    QVector<tPartInfoOnePart>m_vPartInfo;
    /// <summary>
    /// ������������
    /// </summary>
    uint32_t m_DefaultV;           //ini�ļ��е�Ĭ��ֵ
    QString m_MPImgExt;            //����ļ�MPImg�ĺ�׺��
    QString m_UPImgExt;            //����ļ�UPImg�ĺ�׺��
    bool m_bRemoveTmp;             //�Ƿ�ɾ����ʱ�ļ�
    QString m_strVivadoPath;       //ϵͳ���õ�Vivado���ԣ�����Windows��ϵͳ��������
    QTextCodec* m_pTextCodec;
};

