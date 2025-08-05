#pragma once


#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(MPSTYPEE_LIB)
#  define MPSTYPEE_EXPORT Q_DECL_EXPORT
#else
#  define MPSTYPEE_EXPORT Q_DECL_IMPORT
#endif

class MPSTYPEE_EXPORT MPSTypeEParser : public AbstractParser
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID AbstractParser_IID)
	Q_INTERFACES(AbstractParser)
public:
	explicit MPSTypeEParser();
	char* getVersion();
	char* getFormatName();
	bool ConfirmFormat(QString& filename);
	int TransferFile(QString& srcfn, QIODevice* dst);
};
