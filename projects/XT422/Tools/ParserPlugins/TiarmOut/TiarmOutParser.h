#pragma once
#include "../parser_global.h"

# if defined(TIARMOUT_LIB)
#  define TIARMOUT_EXPORT Q_DECL_EXPORT
# else
#  define TIARMOUT_EXPORT Q_DECL_IMPORT
# endif


class TIARMOUT_EXPORT TiarmOutParser : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
	TiarmOutParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
    bool ExecHexTIARMTool(QString strCurFile, QString& OutFile);
};
