#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(TEKTRONIXHEX_LIB)
#  define TEKTRONIXHEX_EXPORT Q_DECL_EXPORT
#else
#  define TEKTRONIXHEX_EXPORT Q_DECL_IMPORT
#endif

class TEKTRONIXHEX_EXPORT TektronixHex : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit TektronixHex();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
private:
    int isTektronixHexButSumError;
    uint32_t NibbleSum(uint8_t* str, int len);
    bool isTektronixHexRecord(uint8_t* rec, int len);
    LineData line;
};
