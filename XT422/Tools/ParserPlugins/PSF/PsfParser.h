#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>


# if defined(PSF_LIB)
#  define PSF_EXPORT Q_DECL_EXPORT
# else
#  define PSF_EXPORT Q_DECL_IMPORT
# endif

class PSF_EXPORT PsfParser : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit PsfParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
    int ParserOneLine(QString& strLine, unsigned short* pAddr, unsigned short* pData);
};
