#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include "Depend/tinyxml2/tinyxml2.h"
using namespace tinyxml2;
class ATabWidget;
namespace Ui { class ViewWidget; };

class ViewWidget : public QDialog
{
	Q_OBJECT

public:
	ViewWidget(QWidget *parent = Q_NULLPTR);
	~ViewWidget();

	QWidget* GetWidget();

	ATabWidget* IsSubTabWgt(QString strName);

	int SaveTabWgt(ATabWidget* tabWgt);

	void InitButton();

	void InitState();

	void SetDefaultXML(XMLDocument* x);
protected:
	void ChangeEditState(bool bState, QObject* obj);

	void GenerateDefault(XMLElement* element);

	QString ChangeElementValue(XMLElement* element);
public slots:
	void OnSlotCheckBox(int);
	void OnSlotSaveBtn();
	void OnSlotLoadBtn();
	void OnSlotDefaultBtn();

private:
	Ui::ViewWidget *ui;
	std::vector<ATabWidget*>	m_vecTab;
	XMLDocument*				m_defaultXML;
};
