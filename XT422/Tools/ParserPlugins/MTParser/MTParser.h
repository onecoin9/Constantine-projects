#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(MTParser_LIB)
#  define MTParser_EXPORT Q_DECL_EXPORT
#else
#  define MPSTYPEB_EXPORT Q_DECL_IMPORT
#endif
class MTParser_EXPORT MTParser : public AbstractParser
{

	Q_OBJECT
	Q_PLUGIN_METADATA(IID AbstractParser_IID)
	Q_INTERFACES(AbstractParser)
public:
	explicit MTParser();
	char* getVersion();
	char* getFormatName();
	bool ConfirmFormat(QString& filename);
	int TransferFile(QString& srcfn, QIODevice* dst);

};
