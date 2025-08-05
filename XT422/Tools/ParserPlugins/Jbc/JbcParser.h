#pragma once

#include "../parser_global.h"

# if defined(JBC_LIB)
#  define JBC_EXPORT Q_DECL_EXPORT
# else
#  define JBC_EXPORT Q_DECL_IMPORT
# endif

typedef struct tagFielInfo {
    unsigned char MAGIC[4];	///"ACAP"
    unsigned int FileType;		///1 为JBC文件
    unsigned int Offset;		///文件偏移位置
    unsigned int Length;		///文件长度
}JBCFILEINFO;

class JBC_EXPORT JbcParser : public AbstractParser
{
    Q_OBJECT
        Q_PLUGIN_METADATA(IID AbstractParser_IID)
        Q_INTERFACES(AbstractParser)
public:
    JbcParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
};
