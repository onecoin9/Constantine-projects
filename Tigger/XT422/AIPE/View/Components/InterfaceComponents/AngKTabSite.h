#pragma once

#include <QtWidgets/QWidget>
#include "qteditorfactory.h"
#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
namespace Ui { class AngKTabSite; };

class AngKTabSite : public QWidget
{
	Q_OBJECT

public:
	AngKTabSite(QWidget *parent = Q_NULLPTR);
	~AngKTabSite();

	void InitLabelText();

	void InitEditState();

	void InitProgPropetry();
protected:
	virtual bool eventFilter(QObject* watched, QEvent* event);

private:
	Ui::AngKTabSite *ui;
	QtStringPropertyManager* m_pStrManager;
	QtLineEditFactory* m_pLineFactory;
	QtVariantPropertyManager* m_pVarManager;
	QtVariantEditorFactory* m_pVarFactory;
};
