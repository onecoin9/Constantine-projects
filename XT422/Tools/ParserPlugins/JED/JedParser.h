#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>


# if defined(JED_LIB)
#  define JED_EXPORT Q_DECL_EXPORT
# else
#  define JED_EXPORT Q_DECL_IMPORT
# endif

class JED_EXPORT JedParser : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit JedParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);

};
