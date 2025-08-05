#pragma once
#include "../parser_global.h"

# if defined(DAT_LIB)
#  define DAT_EXPORT Q_DECL_EXPORT
# else
#  define DAT_EXPORT Q_DECL_IMPORT
# endif

class DAT_EXPORT DatParser : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit DatParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);

protected:
    bool CheckIsDat(const QString& filename);

    int ParserOneLine(const unsigned char* pData, int Size, unsigned char* pDataGet, int* SizeGet, int LineNum);
};
