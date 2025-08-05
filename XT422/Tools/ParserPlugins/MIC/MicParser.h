#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>


# if defined(MIC_LIB)
#  define MIC_EXPORT Q_DECL_EXPORT
# else
#  define MIC_EXPORT Q_DECL_IMPORT
# endif

class MIC_EXPORT MicParser : public AbstractParser
{
    Q_OBJECT
        Q_PLUGIN_METADATA(IID AbstractParser_IID)
        Q_INTERFACES(AbstractParser)
public:
    explicit MicParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
private:
    int WriteStrLine(QString& strLine, unsigned int& Offset);
    int WriteDataLine(QString& strLine, unsigned int& BufferOffset);
    unsigned int m_CNFGOffset;
    unsigned int m_ImgStrOffset;
    unsigned int m_ImgDataOffset;
    unsigned int m_MaskOffset;
    unsigned int m_NoteOffset;
};