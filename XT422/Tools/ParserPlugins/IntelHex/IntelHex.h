#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(INTELHEX_LIB)
#  define INTELHEX_EXPORT Q_DECL_EXPORT
#else
#  define INTELHEX_EXPORT Q_DECL_IMPORT
#endif

class INTELHEX_EXPORT IntelHex : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit IntelHex();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
private:
    int isIntelHexButSumError;
    bool isIntelHexRecord(uint8_t* rec, int len);
    LineData line;
};
