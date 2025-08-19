#pragma once

#include <QtWidgets/QWidget>
#include "qteditorfactory.h"
#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include <QStyledItemDelegate>
#include <QToolTip>
#include <QAbstractItemView>
#include <QHelpEvent>

namespace Ui { class AngKTabProgram; };






class AngKTabProgram : public QWidget
{
	Q_OBJECT

public:
	AngKTabProgram(QWidget *parent = Q_NULLPTR);
	~AngKTabProgram();

	void InitLabelText();

	void InitEditState();

	void InitProgPropetry();
private:
	Ui::AngKTabProgram *ui;
	QtStringPropertyManager* m_pStrManager;
	QtLineEditFactory* m_pLineFactory;
	QtVariantPropertyManager* m_pVarManager;
	QtVariantEditorFactory* m_pVarFactory;

};
