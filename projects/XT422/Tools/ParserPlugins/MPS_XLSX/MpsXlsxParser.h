#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(MPS_XLSX_LIB)
#  define MPS_XLSX_EXPORT Q_DECL_EXPORT
#else
#  define MPS_XLSX_EXPORT Q_DECL_IMPORT
#endif
typedef struct tagItemData
{
	unsigned char Device_Addr;
	unsigned char Command_Code;
	unsigned char Len;
	unsigned short Register_Value;
	void ReInit() {
		Device_Addr = 0;
		Command_Code = 0;
		Len = 0;
		Register_Value = 0;
	}
	tagItemData() {
		ReInit();
	}
}tItemData;
class MPS_XLSX_EXPORT MpsXlsxParser : public AbstractParser
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID AbstractParser_IID)
	Q_INTERFACES(AbstractParser)
public:
    explicit MpsXlsxParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
};