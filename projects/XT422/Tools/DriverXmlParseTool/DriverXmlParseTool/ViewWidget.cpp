#include "ViewWidget.h"
#include "ui_ViewWidget.h"
#include "ACheckBox.h"
#include "AComboBox.h"
#include "AEditText.h"
#include "AGroupBox.h";
#include "ALabel.h"
#include "ATabWidget.h"
ViewWidget::ViewWidget(QWidget *parent)
	: QDialog(parent)
	, m_defaultXML(nullptr)
{
	ui = new Ui::ViewWidget();
	ui->setupUi(this);
	InitButton();
}

ViewWidget::~ViewWidget()
{
	delete ui;
}

QWidget* ViewWidget::GetWidget()
{
	return ui->scrollAreaWidgetContents;
}

ATabWidget* ViewWidget::IsSubTabWgt(QString strName)
{
	bool bFind = false;

	for (auto item : m_vecTab)
	{
		if (item != nullptr && strName == item->property("name").toString())
		{
			bFind = true;
			return item;
		}
	}

	return nullptr;
}

int ViewWidget::SaveTabWgt(ATabWidget* tabWgt)
{
	m_vecTab.push_back(tabWgt);
	return 0;
}

void ViewWidget::InitButton()
{
	connect(ui->editEnable, &QCheckBox::stateChanged, this, &ViewWidget::OnSlotCheckBox);
	connect(ui->saveConfig, &QPushButton::clicked, this, &ViewWidget::OnSlotSaveBtn);
	connect(ui->loadConfig, &QPushButton::clicked, this, &ViewWidget::OnSlotLoadBtn);
	connect(ui->defaultButton, &QPushButton::clicked, this, &ViewWidget::OnSlotDefaultBtn);
}

void ViewWidget::InitState()
{
	OnSlotCheckBox(false);
}

void ViewWidget::SetDefaultXML(XMLDocument* x)
{
	m_defaultXML = x;
}

void ViewWidget::ChangeEditState(bool bState, QObject* obj)
{
	if (strGroup == obj->property("type"))
	{
		AGroupBox* agb = qobject_cast<AGroupBox*>(obj);
		agb->setEnabled(bState);
	}
	else if (strLabel == obj->property("type"))
	{
		ALabel* aLabel = qobject_cast<ALabel*>(obj);
		aLabel->setEnabled(bState);
	}
	else if (strTextEdit == obj->property("type"))
	{
		AEditText* editText = qobject_cast<AEditText*>(obj);
		editText->setEnabled(bState);
	}
	else if (strCheckBox == obj->property("type"))
	{
		ACheckBox* checkBox = qobject_cast<ACheckBox*>(obj);
		checkBox->setEnabled(bState);
	}
	else if (strComboBox == obj->property("type"))
	{
		AComboBox* comboBox = qobject_cast<AComboBox*>(obj);
		comboBox->setEnabled(bState);
	}
	else if (strTabWidget == obj->property("type"))
	{
		ATabWidget* tabWgt = qobject_cast<ATabWidget*>(obj);
		tabWgt->setEnabled(bState);
	}
}

void ViewWidget::GenerateDefault(XMLElement* element)
{
	if (element == nullptr)
		return;

	ChangeElementValue(element);

	element = element->NextSiblingElement();
	GenerateDefault(element);
}

QString ViewWidget::ChangeElementValue(XMLElement* element)
{
	QString strName = element->Attribute("ObjectName");
	QString changeValue;
	QObjectList objList = ui->scrollAreaWidgetContents->children();
	for (auto obj : objList)
	{
		if (obj->property("name").toString() == strName)
		{
			if (obj->property("type").toString() == strTextEdit)
			{
				AEditText* edText = qobject_cast<AEditText*>(obj);

				if(edText->toPlainText() != QString(element->GetText()))
					changeValue += (strName + " : " + QString::fromUtf8(element->GetText())) + "\r\n";

				edText->setText(element->GetText());
			}
			else if (obj->property("type").toString() == strCheckBox)
			{
				ACheckBox* ackBox = qobject_cast<ACheckBox*>(obj);

				qDebug() << ackBox->checkState() << "----" << (Qt::CheckState)element->IntText();
				if (ackBox->checkState() != (Qt::CheckState)element->IntText())
				{
					if(ackBox->checkState() == Qt::CheckState::Unchecked)
						changeValue += "[ ] " + strName + "\r\n";
					else if(ackBox->checkState() == Qt::CheckState::Checked)
						changeValue += "[*] " + strName + "\r\n";
				}

				ackBox->setCheckState((Qt::CheckState)element->IntText());
			}
			else if (obj->property("type").toString() == strComboBox)
			{
				AComboBox* acbBox = qobject_cast<AComboBox*>(obj);

				if(acbBox->currentIndex() != element->IntText())
					changeValue += (strName + " : " + acbBox->itemText(element->IntText())) + "\r\n";

				acbBox->setCurrentIndex(element->IntText());
			}
			else if (obj->property("type").toString() == strLabel)
			{
				ALabel* alab = qobject_cast<ALabel*>(obj);

				if(alab->text() != element->GetText())
					changeValue += (strName + " : " + QString::fromUtf8(element->GetText())) + "\r\n";

				alab->setText(element->GetText());
			}

			break;
		}
	}

	return changeValue;
}

void ViewWidget::OnSlotCheckBox(int nState)
{
	bool bEdit = false;
	if (nState == Qt::Checked)
	{
		bEdit = true;
	}
	else if (nState == Qt::Unchecked)
	{
		bEdit = false;
	}

	ui->defaultButton->setEnabled(bEdit);

	QObjectList objList = ui->scrollAreaWidgetContents->children();
	for (auto obj : objList)
	{
		ChangeEditState(bEdit, obj);
	}
}

void ViewWidget::OnSlotSaveBtn()
{
	qDebug() << "OnSlotSaveBtn";
	QString filePath = QFileDialog::getSaveFileName(this, "New save File...", QDir::currentPath(), tr("*.xml"));

	if (!filePath.isEmpty())
	{
		m_defaultXML->SaveFile(filePath.toStdString().c_str());
	}
}

void ViewWidget::OnSlotLoadBtn()
{
	qDebug() << "OnSlotLoadBtn";
	QString filePath = QFileDialog::getOpenFileName(this, "Open Config File...", QDir::currentPath(), tr("*.xml"));

	XMLDocument doc;
	if (doc.LoadFile(filePath.toStdString().c_str()) != XMLError::XML_SUCCESS) {
		qDebug() << "open file failed";
		return;
	}

	//先找到算法节点和当前节点比较
	{
		XMLElement* algoElement = doc.FirstChildElement();
		while (algoElement != nullptr && !XMLUtil::StringEqual(algoElement->Name(), "Chip"))
		{
			algoElement = algoElement->FirstChildElement();
		}
		QString algoName;
		if(algoElement != nullptr)
			algoName = algoElement->Attribute("Algo");

		XMLElement* algoDefault = m_defaultXML->FirstChildElement();
		while (algoDefault != nullptr && !XMLUtil::StringEqual(algoDefault->Name(), "Chip"))
		{
			algoDefault = algoDefault->FirstChildElement();
		}
		QString algoDefaultName;
		if (algoDefault != nullptr)
			algoDefaultName = algoDefault->Attribute("Algo");

		if (algoName != algoDefaultName)
		{
			QMessageBox::warning(this, tr("Warning"), tr("Algorithm matching error"),
				QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
	}

	//找到样式节点
	XMLElement* proDefaultSheet = m_defaultXML->FirstChildElement();
	while (proDefaultSheet != nullptr && !XMLUtil::StringEqual(proDefaultSheet->Name(), "PropertySheet"))
	{
		qDebug() << proDefaultSheet->Name();
		proDefaultSheet = proDefaultSheet->FirstChildElement();
	}

	XMLElement* proElement = doc.FirstChildElement();
	while (proElement != nullptr && !XMLUtil::StringEqual(proElement->Name(), "PropertySheet"))
	{
		proElement = proElement->FirstChildElement();
	}

	if (proDefaultSheet != nullptr && proElement != nullptr)
	{
		XMLElement* subDefaultNode = proDefaultSheet->FirstChildElement();
		XMLElement* subSheetNode = proElement->FirstChildElement();

		XMLElement* subDefaultNodeTemp = subDefaultNode;
		XMLElement* subSheetNodeTemp = subSheetNode;

		QString strPrint;
		while (subDefaultNodeTemp != nullptr)
		{

			while (subSheetNodeTemp != nullptr)
			{
				const char* testName;
				if (subDefaultNodeTemp->QueryAttribute("ObjectName", &testName) == XML_SUCCESS)
				{
					if (XMLUtil::StringEqual(subSheetNodeTemp->Attribute("ObjectName"), testName))
					{
						strPrint += ChangeElementValue(subSheetNodeTemp);
					}
					else
					{
						qDebug() << "error:" << subSheetNodeTemp->Attribute("ObjectName") << " - " << testName;
					}
				}
				subSheetNodeTemp = subSheetNodeTemp->NextSiblingElement();
			}
			subSheetNodeTemp = subSheetNode;
			subDefaultNodeTemp = subDefaultNodeTemp->NextSiblingElement();
		}

		if (!strPrint.isEmpty())
		{
			qDebug().noquote() << strPrint;
			QMessageBox::warning(this, tr("Warning"), "Config Changed: \n" + strPrint, QMessageBox::Ok, QMessageBox::Ok);
		}
	}
}

void ViewWidget::OnSlotDefaultBtn()
{
	qDebug() << "OnSlotDefaultBtn";
	if (m_defaultXML != nullptr)
	{
		XMLElement* proSheet = m_defaultXML->FirstChildElement();
		while (proSheet != nullptr && !XMLUtil::StringEqual(proSheet->Name(), "PropertySheet"))
		{
			proSheet = proSheet->FirstChildElement();
		}

		if (proSheet != nullptr)
		{
			XMLElement* subSheetNode = proSheet->FirstChildElement();

			GenerateDefault(subSheetNode);
		}
	}
}
