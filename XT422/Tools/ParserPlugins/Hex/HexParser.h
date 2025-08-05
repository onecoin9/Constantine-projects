#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>


# if defined(HEX_LIB)
#  define HEX_EXPORT Q_DECL_EXPORT
# else
#  define HEX_EXPORT Q_DECL_IMPORT
# endif

class HEX_EXPORT HexParser : public AbstractParser
{
    Q_OBJECT
        Q_PLUGIN_METADATA(IID AbstractParser_IID)
        Q_INTERFACES(AbstractParser)
public:
    explicit HexParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
    int CheckSumCompare(unsigned char* pData, int Size);


    long m_StartDataTblAdrr;
    unsigned int m_SumLen;
    unsigned long m_FlashTableOff;

    long m_PreDataTblAddr; //最后一次写入的地址，
    //bool m_bExistFlashTable;
    unsigned long m_FlashTableStartAddr;
};
