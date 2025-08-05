#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(MPSTYPED_LIB)
#  define MPSTYPED_EXPORT Q_DECL_EXPORT
#else
#  define MPSTYPED_EXPORT Q_DECL_IMPORT
#endif
class MPSTYPED_EXPORT MPSTypeDParser : public AbstractParser
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID AbstractParser_IID)
	Q_INTERFACES(AbstractParser)
public:
	explicit MPSTypeDParser();
	char* getVersion();
	char* getFormatName();
	bool ConfirmFormat(QString& filename);
	int TransferFile(QString& srcfn, QIODevice* dst);
};
