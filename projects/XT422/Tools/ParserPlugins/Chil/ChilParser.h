#pragma once
#include "../parser_global.h"
# if defined(CHIL_LIB)
#  define CHIL_EXPORT Q_DECL_EXPORT
# else
#  define CHIL_EXPORT Q_DECL_IMPORT
# endif


class Q_DECL_EXPORT ChilParser : public AbstractParser
{
    Q_OBJECT
        Q_PLUGIN_METADATA(IID AbstractParser_IID)
        Q_INTERFACES(AbstractParser)
public:
    explicit ChilParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);

private:
    //int GetOneLine(unsigned char* pLineBuf, int LineBufSize, int& Offset, unsigned char* pData, int TotalSize);
    //static bool CheckLineFormat(unsigned char* pLineBuf, int Size);
    int WriteStrLine(QString& strLine, unsigned int& Offset);
    int SetCRC8(QString& strLine);

    bool ParserOneLine(QString& strLine, unsigned int& Addr, unsigned int& Data, unsigned int& Mask);
};
