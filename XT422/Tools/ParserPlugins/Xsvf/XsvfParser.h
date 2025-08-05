#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>


# if defined(XSVF_LIB)
#  define XSVF_EXPORT Q_DECL_EXPORT
# else
#  define XSVF_EXPORT Q_DECL_IMPORT
# endif

class XSVF_EXPORT XsvfParser : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit XsvfParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
};
