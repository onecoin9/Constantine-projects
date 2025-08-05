#pragma once

#include "AngKDialog.h"
#include "GlobalDefine.h"
namespace Ui { class AngKAddeMMCFile; };

class QCheckBox;
class AngKAddeMMCFile : public AngKDialog
{
	Q_OBJECT

public:
	AngKAddeMMCFile(QWidget *parent);
	~AngKAddeMMCFile();

	void InitText();

	void InitButton();

	void seteMMCParams(eMMCFileInfo eInfo);

	void setFileRecord(QVector<eMMCFileRecord> vecFileReocrd);
signals:
	void sgnAddeMMCFile(eMMCFileInfo);

public slots:
	void onSlotAddeMMCFile();

	void onAutoCalPartitionPos(QString);
private:
	Ui::AngKAddeMMCFile *ui;
	eMMCFileInfo stuEFileInfo;
	QVector<eMMCFileRecord> m_TempVecRecord;
};
