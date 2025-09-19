#pragma once
#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(MPS_LIB)
#  define MPS_EXPORT Q_DECL_EXPORT
#else
#  define MPS_EXPORT Q_DECL_IMPORT
#endif


typedef struct DestDataFmt {
	unsigned char Addr;
	unsigned char Cmd;
	unsigned char NumCnt;
	unsigned char Data[2];
	void ReInit() {
		Addr = 0x20;
		Cmd = 0x00;
		NumCnt = 0x00;
		memset(Data, 0, 2);
	};
	DestDataFmt() {
		ReInit();
	};
}tDestDataFmt;


class MPS_EXPORT MpsParser : public AbstractParser
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID AbstractParser_IID)
	Q_INTERFACES(AbstractParser)
public:
	explicit MpsParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
};
