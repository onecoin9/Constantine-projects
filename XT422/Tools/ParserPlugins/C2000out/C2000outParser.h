#pragma once


#include "..\parser_global.h"
#include <QtCore>
#include <QString>


# if defined(C2000OUT_LIB)
#  define C2000OUT_EXPORT Q_DECL_EXPORT
# else
#  define C2000OUT_EXPORT Q_DECL_IMPORT
# endif

class C2000OUT_EXPORT C2000outParser : public AbstractParser
{
    Q_OBJECT
        Q_PLUGIN_METADATA(IID AbstractParser_IID)
        Q_INTERFACES(AbstractParser)
public:
    explicit C2000outParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);


    static bool ExecHexC2000Tool(const QString& strCurFile, const QString& OutFile, QString& errStr);
private:
    int isC2000OutButSumError;
    LineData line;
};
