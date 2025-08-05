#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(MPSTYPEB_LIB)
#  define MPSTYPEB_EXPORT Q_DECL_EXPORT
#else
#  define MPSTYPEB_EXPORT Q_DECL_IMPORT
#endif
class MPSTYPEB_EXPORT MPSTypeBParser : public AbstractParser
{

	Q_OBJECT
	Q_PLUGIN_METADATA(IID AbstractParser_IID)
	Q_INTERFACES(AbstractParser)
public:
	explicit MPSTypeBParser();
	char* getVersion();
	char* getFormatName();
	bool ConfirmFormat(QString& filename);
	int TransferFile(QString& srcfn, QIODevice* dst);

};
