#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(SIGNETICSHEX_LIB)
#  define SIGNETICSHEX_EXPORT Q_DECL_EXPORT
#else
#  define SIGNETICSHEX_EXPORT Q_DECL_IMPORT
#endif

class SIGNETICSHEX_EXPORT SigneticsHex : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit SigneticsHex();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
private:
    int isSigneticsHexButSumError;
    bool isSigneticsHexRecord(uint8_t* rec, int len);
    uint32_t XorRotateSum(uint8_t* str, int len);
    LineData line;
};
