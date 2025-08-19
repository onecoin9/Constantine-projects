#pragma once

#include "AngKDialog.h"
#include "GlobalDefine.h"
#include <QCryptographicHash>
namespace Ui { class AngKFilesImport; };

class AbstractParser;
class QButtonGroup;
class QFile;
class AngKFilesImport : public AngKDialog
{
	Q_OBJECT

public:
	AngKFilesImport(QWidget *parent = Q_NULLPTR);
	~AngKFilesImport();

	void InitText();

	void InitGroup();

	void InitDetectInfo();

	void InitDlg();

	void InitParsers();

	void clearDlg();

	QString fileChecksum(const QString& fileName, QCryptographicHash::Algorithm hashAlgorithm);

	void DetectFileFormat(QString strFile);

	void TransferFileFormat(QString srcFile, QFile& dstFile);

	void SetFileTagHide(bool bHide);

	bool GetFileTagHide();

	void SetFileDataInfo(FileDataInfo& dataInfo);
signals:
	void sgnSelectFileInfo(FileDataInfo, bool);

public slots:
	void onSlotSelectFile();

	void onSlotAddFile();
private:
	Ui::AngKFilesImport *ui;
	QButtonGroup* m_swapGroup;
	bool isDoubleClicked;
};
