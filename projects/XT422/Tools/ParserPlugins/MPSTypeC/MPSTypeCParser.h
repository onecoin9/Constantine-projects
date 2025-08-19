#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(MPSTYPEC_LIB)
#  define MPSTYPEC_EXPORT Q_DECL_EXPORT
#else
#  define MPSTYPEC_EXPORT Q_DECL_IMPORT
#endif

class MPSTYPEC_EXPORT MPSTypeCParser : public AbstractParser
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID AbstractParser_IID)
	Q_INTERFACES(AbstractParser)
public:
	explicit MPSTypeCParser();
	char* getVersion();
	char* getFormatName();
	bool ConfirmFormat(QString& filename);
	int TransferFile(QString& srcfn, QIODevice* dst);
};
