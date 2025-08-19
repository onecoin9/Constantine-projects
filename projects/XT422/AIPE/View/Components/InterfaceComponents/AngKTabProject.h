#pragma once

#include <QtWidgets/QWidget>
#include "DataJsonSerial.hpp"
#include "ACProjManager.h"
#include "qteditorfactory.h"
#include "qtpropertymanager.h"
#include "qtvariantproperty.h"

namespace Ui { class AngKTabProject; };

class AngKTabProject : public QWidget
{
	Q_OBJECT

public:
	AngKTabProject(QWidget *parent = Q_NULLPTR);
	~AngKTabProject();

	void InitProjPropetry();

	void SetProjPariInfo(QMap<QString, QPair<QString, ACProjManager*>> _projPairInfo);

	void UpdateProjUI();
private:

	void ChangeProjInfo(QString propText, int nIndex);
signals:
	void sgnAcceptChipDlg();
	void sgnProperty2Mainframe(QString, int);
public slots:
	void onSlotSelectProjFile();
	void onSlotSelectChip();
	void onSlotSelectChipDataJson(ChipDataJsonSerial);
	void onSlotSelectFile();

	void onSlotSaveProj();
	void onSlotVariantPropertyValueChanged(QtProperty*, const QVariant&);
	void onSlotCurrentItemChanged(QtBrowserItem*);
	void onSlotProjIndexValueChanged(QtProperty*, int);
private:
	Ui::AngKTabProject *ui;
	AngKProjDataset*	m_projDataset;
	QtStringPropertyManager*	m_pStrManager;
	QtVariantPropertyManager*	m_pVarManager;
	QtLineEditFactory*			m_pLineFactory;
	QtVariantEditorFactory*		m_pVarFactory;
	QtEnumPropertyManager*		m_pTaskPropertyManager;
	QtEnumEditorFactory*		m_pCcomboBoxFactory;
	QtStringPropertyManager*	m_pStrProjFileManager;
	QMap<QString, QPair<QString, ACProjManager*>> m_mapProjPair;
};
