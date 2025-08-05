#pragma once

#include "..\parser_global.h"
#include <QtCore>
#include <QString>

#if defined(PMF_LIB)
#  define PMF_EXPORT Q_DECL_EXPORT
#else
#  define PMF_EXPORT Q_DECL_IMPORT
#endif
// Do not forget to add ABSTRACTPARSER_LIB in preprocessor definitions

#define PMFDATA_STATIC_LEN (256)

typedef struct tagPMFTable {
	QString strTblID;
	QString strConfig;
	unsigned char Key;
	unsigned short Offset;	///当前偏移
	unsigned short Length;	///实际长度
	unsigned short MaxSize; ///Data允许存放的最大字节数
	unsigned char Data[PMFDATA_STATIC_LEN];
	unsigned char* pData;
	tagPMFTable() {
		pData = NULL;
		Offset = 0;
		Length = 0;
		Key = 0;
		MaxSize = PMFDATA_STATIC_LEN;
		memset(Data, 0, PMFDATA_STATIC_LEN);
	}
}tPMFTable;

class PMF_EXPORT PmfParser : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit PmfParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);

private:

    int DataType; ///记录数据的类型
	QVector<tPMFTable> m_vTable;

	void ClearTable(QVector<tPMFTable>& vTable);
	int SaveTableInfoToBuffer(tPMFTable* pTable);
	int GetTableCRC(unsigned char* pData, int Size, unsigned short& CRCSum);
	
	int CheckTableDataReady(QVector<tPMFTable>& vTable);
	int WriteDataToPMFTable(tPMFTable* pTable, unsigned short* pAddr, unsigned short* pData);
	int ParserOneLine(QString& strLine, unsigned short* pAddr, unsigned short* pData);
	int GetTableCfg(QString& strLine, QVector<tPMFTable>& vTable);
	int GetTableKey(tPMFTable* pTable);
	tPMFTable* GetPMTTable(QString strLine, QVector<tPMFTable>& vTable);
	int OffsetTableInfo;
};

