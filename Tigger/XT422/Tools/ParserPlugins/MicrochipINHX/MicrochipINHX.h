#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(MICROCHIPINHX_LIB)
#  define MICROCHIPINHX_EXPORT Q_DECL_EXPORT
#else
#  define MICROCHIPINHX_EXPORT Q_DECL_IMPORT
#endif

class MICROCHIPINHX_EXPORT MicrochipINHX : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit MicrochipINHX();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
private:
    int isMicrochipINHXButSumError;
    bool isMicrochipINHXRecord(uint8_t* rec, int len);
    LineData line;
};

