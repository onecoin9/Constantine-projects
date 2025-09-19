#pragma once

#include "AngKDialog.h"
namespace Ui { class ACAdapterCountInfomation; };

class QTextStream;
class ACAdapterCountInfomation : public AngKDialog
{
	Q_OBJECT

public:
	ACAdapterCountInfomation(QWidget *parent = Q_NULLPTR);
	~ACAdapterCountInfomation();

	void InitText();

	void InitButton();

private:
	void SetAdaUseTab();
	void SetAdaInfoTab();
	int GetUseBPUEn();
	void TextFormat(QTextStream& textStream, QString strBPUID, QString strUID, int InstCnt, int nFailedCnt, int nLifeCycle);
	void TextDetailFormat(QTextStream& textStream, QString strBPUID, int InstCnt, int nFailedCnt, int nLifeCycle);
signals:
	void sgnQueryAdapterInfo(int, int, std::string, int);
public slots:
	void onSlotSwitchTabBar(int);
	void onSlotSetBtnState(bool);
	void onSlotQueryAdapterInfo();
	void onSlotShowSktInfo(std::string, std::string);
	void onSlotShowSktInfoSimple(std::string, std::string);
private:
	Ui::ACAdapterCountInfomation *ui;
};
