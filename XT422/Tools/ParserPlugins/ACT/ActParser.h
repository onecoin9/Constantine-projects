#pragma once

#include "../parser_global.h"
# if defined(ACT_LIB)
#  define ACT_EXPORT Q_DECL_EXPORT
# else
#  define ACT_EXPORT Q_DECL_IMPORT
# endif


typedef struct tagHeader {
	unsigned char Magic[4];		///鍥哄畾涓篈CAT
	unsigned int TagCnt;		///Tag涓暟
	unsigned char Version[2];	///楂樺瓧鑺備负涓荤増鏈彿锛屼綆瀛楄妭涓烘鐗堟湰鍙锋瘮濡?x01 0x00锛岃〃绀?.0鐗堟湰
	unsigned char Reserved[6];	///淇濈暀
}tHeader;

typedef struct tagTag {
	unsigned char TagID;			///Tag鐨勬爣璇嗭紝0涓烘棤鏁堟爣璇?
	unsigned char Reserved[3];	///淇濈暀
	unsigned int Offset;		///Tag鏁版嵁鍦ㄦ枃浠朵腑鐨勫亸绉?
	unsigned int Size;			///Tag鏁版嵁鐨勫ぇ灏?
	char TagName[16];	///Tag鍚嶇О
	unsigned int CRC32;			///CRC32鍊?
}tTag;

class ACT_EXPORT ActParser : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit ActParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);

private:
	int ReadUserDataFromTag(QFile& File, tTag& Tag);
	int ReadFileFromTag(QFile& File, tTag& Tag);
};
