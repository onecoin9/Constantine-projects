#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>


# if defined(MPS_HR12XX_TXT_LIB)
#  define MPS_HR12XX_TXT_EXPORT Q_DECL_EXPORT
# else
#  define MPS_HR12XX_TXT_EXPORT Q_DECL_IMPORT
# endif

class MPS_HR12XX_TXT_EXPORT MPS_HR12xx_txtParser : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit MPS_HR12xx_txtParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
};
