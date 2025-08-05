#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(BINARY_LIB)
#  define BINARY_EXPORT Q_DECL_EXPORT
#else
#  define BINARY_EXPORT Q_DECL_IMPORT
#endif

class BINARY_EXPORT Binary : public AbstractParser
{
    Q_OBJECT
        Q_PLUGIN_METADATA(IID AbstractParser_IID)
        Q_INTERFACES(AbstractParser)
public:
    explicit Binary();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
private:
    int isBinaryButSumError;
    //bool isBinaryRecord(uint8_t* rec, int len);
    LineData line;
};
