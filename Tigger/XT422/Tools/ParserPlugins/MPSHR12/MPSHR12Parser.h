#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(MPSHR12_LIB)
#  define MPSHR12_EXPORT Q_DECL_EXPORT
#else
#  define MPSHR12_EXPORT Q_DECL_IMPORT
#endif

class MPSHR12_EXPORT MPSHR12Parser : public AbstractParser
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID AbstractParser_IID)
	Q_INTERFACES(AbstractParser)
public:
	explicit MPSHR12Parser();
	char* getVersion();
	char* getFormatName();
	bool ConfirmFormat(QString& filename);
	int TransferFile(QString& srcfn, QIODevice* dst);
};
