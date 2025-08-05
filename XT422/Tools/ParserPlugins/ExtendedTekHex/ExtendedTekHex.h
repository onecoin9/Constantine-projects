#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(EXTENDEDTEKHEX_LIB)
#  define EXTENDEDTEKHEX_EXPORT Q_DECL_EXPORT
#else
#  define EXTENDEDTEKHEX_EXPORT Q_DECL_IMPORT
#endif

class EXTENDEDTEKHEX_EXPORT ExtendedTekHex : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit ExtendedTekHex();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
private:
    int isExtendedTekHexButSumError;
    uint32_t SumExtTek(uint8_t* str, int len);
    bool isExtendedTekHexRecord(uint8_t* rec, int len);
    LineData line;
};
