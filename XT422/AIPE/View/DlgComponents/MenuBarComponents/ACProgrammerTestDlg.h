#pragma once

#include "AngKDialog.h"
namespace Ui { class ACProgrammerTestDlg; };

/// <summary>
/// 0-3 : DDR、SPIFlash、SSD、DPS
/// </summary>
class ACProgrammerTestDlg : public AngKDialog
{
	Q_OBJECT

public:
	ACProgrammerTestDlg(QWidget *parent = Q_NULLPTR);
	~ACProgrammerTestDlg();

	void InitText();

signals:
	void sgnProgrammgerTest(int);

public slots:
	void onSlotProgrammgerTest();

	void onSlotProgramSelfTestResult(QString, QString, int);

	void onSlotTestTimeout();
private:
	Ui::ACProgrammerTestDlg *ui;
	int	m_nAllDevNums;
	bool bStartTest;
};
