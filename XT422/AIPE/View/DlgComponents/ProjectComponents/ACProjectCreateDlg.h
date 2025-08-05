#pragma once

#include "AngKDialog.h"
namespace Ui { class ACProjectCreateDlg; };

class QToolButton;
class AngKDataBuffer;
class ACProjectCreateDlg : public AngKDialog
{
	Q_OBJECT

public:
	ACProjectCreateDlg(QWidget *parent, std::shared_ptr<AngKDataBuffer> _pDataBuf);
	~ACProjectCreateDlg();

	void InitText();

	void InitButton();

	void AddProjTab(AngKDataBuffer* pDataBuf);

	void FTabCreateEmptyEapr();
public slots:
	void onSlotAddProjTab();

	void onSlotRemoveProjTab(int);

	void onSlotTabBarDoubleClicked();

	void onSlotImportProjName(QString);
private:
	Ui::ACProjectCreateDlg *ui;
	//std::unique_ptr<QToolButton> m_pAddButton;
};
