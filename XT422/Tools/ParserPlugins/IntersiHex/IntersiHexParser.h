#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>


# if defined(INTERSIHEX_LIB)
#  define INTERSIHEX_EXPORT Q_DECL_EXPORT
# else
#  define INTERSIHEX_EXPORT Q_DECL_IMPORT
# endif

class INTERSIHEX_EXPORT IntersiHexParser : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit IntersiHexParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
    
};