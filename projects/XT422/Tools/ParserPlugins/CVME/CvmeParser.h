#pragma once

#include "../parser_global.h"

# if defined(CVME_LIB)
#  define CVME_EXPORT Q_DECL_EXPORT
# else
#  define CVME_EXPORT Q_DECL_IMPORT
# endif

class CVME_EXPORT CvmeParser : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit CvmeParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
};
