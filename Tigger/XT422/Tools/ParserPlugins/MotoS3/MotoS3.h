#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(MOTOS3_LIB)
#  define MOTOS3_EXPORT Q_DECL_EXPORT
#else
#  define MOTOS3_EXPORT Q_DECL_IMPORT
#endif
// Do not forget to add ABSTRACTPARSER_LIB in preprocessor definitions

class MOTOS3_EXPORT MotoS3 : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit MotoS3();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
private:
    int isMotorolaSButSumError;
    bool isMotorolaSRecord(uint8_t* rec, int len);
    LineData line;
};

