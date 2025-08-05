#include "DriverParseWidget.h"
#include "ui_DriverParseWidget.h"
#include "ACheckBox.h"
#include "AComboBox.h"
#include "AEditText.h"
#include "AGroupBox.h"
#include "ALabel.h"
#include "ATabWidget.h"

DriverParseWidget::DriverParseWidget(QWidget *parent)
	: QWidget(parent)
	, m_curShowPropetry(nullptr)
	, m_curSelItemCol(0)
	, m_curOperFile("")
	, m_curXmlDoc(nullptr)
{
	ui = new Ui::DriverParseWidget();
	ui->setupUi(this);

	InitEditWgt();
	InitPropetryWgt();
	InitChipTreeWgt();
}

DriverParseWidget::~DriverParseWidget()
{
	delete ui;
}

void DriverParseWidget::InitEditWgt()
{
	ui->dragListWidget->addItem("Group");
	ui->dragListWidget->addItem("Label");
	ui->dragListWidget->addItem("TextEdit");
	ui->dragListWidget->addItem("CheckBox");
	ui->dragListWidget->addItem("ComboBox");
	ui->dragListWidget->addItem("TabPage");
}

void DriverParseWidget::InitPropetryWgt()
{
	connect(ui->scrollAreaWidget, &EditShowWidget::sgnShowPropetry, this, &DriverParseWidget::OnSlotShowPropetry);
	connect(ui->scrollAreaWidget, &EditShowWidget::sgnClearPropetry, this, &DriverParseWidget::OnSlotClearPropetry);

	connect(ui->saveButton, &QPushButton::clicked, this, &DriverParseWidget::OnSlotSaveUI2XML);
	connect(ui->viewButton, &QPushButton::clicked, this, &DriverParseWidget::OnSlotOpenViewWgt);
}

void DriverParseWidget::InitChipTreeWgt()
{
	m_curXmlDoc = new tinyxml2::XMLDocument();

	ui->treeWidget->setHeaderHidden(true);
	ui->treeWidget->expandAll();
	ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, &DriverParseWidget::OnSlotItemDoubleClicked);
	connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &DriverParseWidget::OnSlotItemClicked);

	//默认顶层结点为chipNode
	QTreeWidgetItem* topItem = new QTreeWidgetItem(ui->treeWidget);
	topItem->setText(0, "Algorithm");
	ui->treeWidget->addTopLevelItem(topItem);

	//添加菜单
	QMenu* tMenu = new QMenu(ui->treeWidget);

	QAction* addAction = new QAction("add", ui->treeWidget);
	QAction* delAction = new QAction("del", ui->treeWidget);

	connect(addAction, &QAction::triggered, this, &DriverParseWidget::OnSlotAddTreeItem);
	connect(delAction, &QAction::triggered, this, &DriverParseWidget::OnSlotDelTreeItem);

	tMenu->addAction(addAction);
	tMenu->addAction(delAction);

	connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, [=](QPoint p)
		{
			tMenu->exec(ui->treeWidget->mapToGlobal(p));
		});
}

int DriverParseWidget::CreateXML(QString strPath)
{
	if (strPath.isEmpty())
	{
		qDebug() << "Create XML is Failed";
		return -1;
	}

	//遍历找到Chip节点
	XMLElement* chipList = m_curXmlDoc->RootElement()->FirstChildElement();
	while (chipList != nullptr && !XMLUtil::StringEqual(chipList->Name(), "ChipList"))
	{
		chipList = chipList->FirstChildElement();
	}
	
	std::map<QTreeWidgetItem*, XMLElement*>::iterator iter = m_mapAlgoElement.begin();
	while (iter != m_mapAlgoElement.end())
	{
		if (iter->first == ui->treeWidget->currentItem())
		{
			QStringList strList = ui->treeWidget->currentItem()->text(m_curSelItemCol).split(":");
			iter->second->SetAttribute("Algo", strList[strList.count() - 1].toStdString().c_str());
			chipList->InsertEndChild(iter->second);

			//属性表单
			iter->second->DeleteChildren();
			XMLElement* proSheet = m_curXmlDoc->NewElement("PropertySheet");
			iter->second->InsertEndChild(proSheet);

			CreateChildElement(ui->scrollAreaWidget->children(), proSheet, m_curXmlDoc);
		}
		else
		{
			if(iter->second != nullptr)
				chipList->InsertEndChild(iter->second);
		}
		iter++;
	}
	return m_curXmlDoc->SaveFile(strPath.toStdString().c_str());
}

void DriverParseWidget::CreateChildElement(QObjectList oList, XMLElement* element, XMLDocument* doc)
{
	if (oList.isEmpty())
		return;

	for (int i = 0;i < oList.count(); ++i)
	{
		if (oList[i]->property("type") == strTabWidget)
		{
			ATabWidget* tabWgt = qobject_cast<ATabWidget*>(oList[i]);
			for (int i = 0; i < tabWgt->count(); ++i)
			{
				XMLElement* tabXml = doc->NewElement(strTabWidget.toStdString().c_str());
				tabXml->SetAttribute("ObjectName", tabWgt->property("name").toString().toStdString().c_str());
				tabXml->SetAttribute("Caption", tabWgt->widget(i)->property("caption").toString().toStdString().c_str());
				tabXml->SetAttribute("wrect", ComTool::Rect2String(tabWgt->property("wrect").value<QRect>()).c_str());
				element->InsertEndChild(tabXml);
				CreateChildElement(tabWgt->widget(i)->children(), tabXml, doc);
			}
		}
		else if (oList[i]->property("type") == strGroup)
		{
			AGroupBox* agb = qobject_cast<AGroupBox*>(oList[i]);
			XMLElement* agbXml = doc->NewElement(strGroup.toStdString().c_str());
			agbXml->SetAttribute("ObjectName", agb->property("name").toString().toStdString().c_str());
			agbXml->SetAttribute("addr", QString("0x" + QString::number(agb->property("addr").toInt(), 16)).toStdString().c_str());
			agbXml->SetAttribute("len", QString("0x" + QString::number(agb->property("len").toInt(), 16)).toStdString().c_str());
			agbXml->SetAttribute("default", QString("0x" + QString::number(agb->property("default").toInt(), 16)).toStdString().c_str());
			agbXml->SetAttribute("ubuff", agb->property("ubuff").toBool());
			agbXml->SetAttribute("wrect", ComTool::Rect2String(agb->property("wrect").value<QRect>()).c_str());
			agbXml->SetAttribute("edited", agb->property("edited").toBool());
			element->InsertEndChild(agbXml);
			CreateChildElement(agb->children(), agbXml, doc);
		}
		else if (oList[i]->property("type") == strTextEdit)
		{
			AEditText* aEdit = qobject_cast<AEditText*>(oList[i]);
			XMLElement* aEditXml = doc->NewElement(strTextEdit.toStdString().c_str());
			aEditXml->SetAttribute("ObjectName", aEdit->property("name").toString().toStdString().c_str());
			aEditXml->SetAttribute("addr", QString("0x" + QString::number(aEdit->property("addr").toInt(), 16)).toStdString().c_str());
			aEditXml->SetAttribute("len", QString("0x" + QString::number(aEdit->property("len").toInt(), 16)).toStdString().c_str());
			aEditXml->SetAttribute("default", QString("0x" + QString::number(aEdit->property("default").toInt(), 16)).toStdString().c_str());
			aEditXml->SetAttribute("ubuff", aEdit->property("ubuff").toBool());
			aEditXml->SetAttribute("wrect", ComTool::Rect2String(aEdit->property("wrect").value<QRect>()).c_str());
			aEditXml->SetAttribute("passwd", aEdit->property("passwd").toBool());
			aEditXml->SetAttribute("valuetype", ComTool::EditVaule_Int2CString(aEdit->property("valuetype").toInt()));
			aEditXml->SetAttribute("shift", aEdit->property("shift").toString().toStdString().c_str());
			aEditXml->SetAttribute("showzerohead", aEdit->property("showzerohead").toBool());
			aEditXml->SetAttribute("edian", ComTool::EditEndian_Int2CString(aEdit->property("endian").toInt()));
			aEditXml->SetAttribute("edited", aEdit->property("edited").toBool());
			aEditXml->SetText(aEdit->toPlainText().toStdString().c_str());
			element->InsertEndChild(aEditXml);
		}
		else if (oList[i]->property("type") == strCheckBox)
		{
			ACheckBox* ackBox = qobject_cast<ACheckBox*>(oList[i]);
			XMLElement* ackBoxXml = doc->NewElement(strCheckBox.toStdString().c_str());
			ackBoxXml->SetAttribute("ObjectName", ackBox->property("name").toString().toStdString().c_str());
			ackBoxXml->SetAttribute("addr", QString("0x" + QString::number(ackBox->property("addr").toInt(), 16)).toStdString().c_str());
			ackBoxXml->SetAttribute("len", QString("0x" + QString::number(ackBox->property("len").toInt(), 16)).toStdString().c_str());
			ackBoxXml->SetAttribute("default", QString("0x" + QString::number(ackBox->property("default").toInt(), 16)).toStdString().c_str());
			ackBoxXml->SetAttribute("ubuff", ackBox->property("ubuff").toBool());
			ackBoxXml->SetAttribute("wrect", ComTool::Rect2String(ackBox->property("wrect").value<QRect>()).c_str());
			ackBoxXml->SetAttribute("value", QString("0x" + QString::number(ackBox->property("value").toInt(), 16)).toStdString().c_str());
			ackBoxXml->SetAttribute("mask", ackBox->property("mask").toInt());
			ackBoxXml->SetAttribute("edited", ackBox->property("edited").toBool());
			ackBoxXml->SetText(QString::number(ackBox->checkState()).toStdString().c_str());
			element->InsertEndChild(ackBoxXml);
		}
		else if (oList[i]->property("type") == strComboBox)
		{
			AComboBox* acbBox = qobject_cast<AComboBox*>(oList[i]);
			XMLElement* acbBoxXml = doc->NewElement(strComboBox.toStdString().c_str());
			acbBoxXml->SetAttribute("ObjectName", acbBox->property("name").toString().toStdString().c_str());
			acbBoxXml->SetAttribute("addr", QString("0x" + QString::number(acbBox->property("addr").toInt(), 16)).toStdString().c_str());
			acbBoxXml->SetAttribute("len", QString("0x" + QString::number(acbBox->property("len").toInt(), 16)).toStdString().c_str());
			acbBoxXml->SetAttribute("default", QString("0x" + QString::number(acbBox->property("default").toInt(), 16)).toStdString().c_str());
			acbBoxXml->SetAttribute("ubuff", acbBox->property("ubuff").toBool());
			acbBoxXml->SetAttribute("wrect", ComTool::Rect2String(acbBox->property("wrect").value<QRect>()).c_str());
			acbBoxXml->SetAttribute("mask", acbBox->property("mask").toInt());
			acbBoxXml->SetAttribute("edited", acbBox->property("edited").toBool());
			
			//判断是否有子项，插入comboItem
			for (int i = 0; i < acbBox->count(); ++i)
			{
				XMLElement* boxItemXml = doc->NewElement(QString("ComboItem").toStdString().c_str());
				QString itemValue = acbBox->property(acbBox->itemText(i).toStdString().c_str()).toString();
				boxItemXml->SetAttribute("ObjectName", acbBox->itemText(i).toStdString().c_str());
				boxItemXml->SetAttribute("value", QString("0x" + QString::number(itemValue.toInt(), 16)).toStdString().c_str());
				acbBoxXml->InsertEndChild(boxItemXml);
			}
			acbBoxXml->SetText(QString::number(acbBox->currentIndex()).toStdString().c_str());
			element->InsertEndChild(acbBoxXml);
		}
		else if (oList[i]->property("type") == strLabel)
		{
			ALabel* alab = qobject_cast<ALabel*>(oList[i]);
			XMLElement* alabXml = doc->NewElement(strLabel.toStdString().c_str());
			alabXml->SetAttribute("ObjectName", alab->property("name").toString().toStdString().c_str());
			alabXml->SetAttribute("wrect", ComTool::Rect2String(alab->property("wrect").value<QRect>()).c_str());
			alabXml->SetText(alab->text().toStdString().c_str());
			element->InsertEndChild(alabXml);
		}
	}
}

void DriverParseWidget::CreateXML2UI(XMLElement* element, QWidget* objParent, ViewWidget& vWgt, bool bView)
{
	if (element == nullptr)
		return;

	if (element != nullptr && XMLUtil::StringEqual(element->Name(), strTabWidget.toStdString().c_str()))
	{
		ATabWidget* tabWgt;
		if (!bView)
		{
			tabWgt = ui->scrollAreaWidget->IsSubTabWgt(element->Attribute("ObjectName"));

			if (nullptr == tabWgt)
			{
				tabWgt = new ATabWidget(objParent);
				ui->scrollAreaWidget->XmlCreateTabWidget(tabWgt);
			}

			if (objParent->property("type") == strTabWidget)
			{
				tabWgt->disconnect(tabWgt, &ATabWidget::sgnTabWidget, ui->scrollAreaWidget, &EditShowWidget::OnSlotTabWidget);
			}
			
		}
		else
		{
			tabWgt = vWgt.IsSubTabWgt(element->Attribute("ObjectName"));
			if (tabWgt == nullptr)
			{
				tabWgt = new ATabWidget(objParent);
				vWgt.SaveTabWgt(tabWgt);
			}
			tabWgt->SetView(bView);
		}

		tabWgt->setProperty("type", strTabWidget);
		tabWgt->setProperty("name", element->Attribute("ObjectName"));
		//添加子窗口
		QWidget* tab = new QWidget();
		tab->setObjectName(QString::fromUtf8("tab") + QString::fromUtf8(element->Attribute("Caption")));
		tabWgt->addTab(tab, QString());
		tabWgt->setTabText(tabWgt->count() - 1, element->Attribute("Caption"));
		tab->setProperty("caption", element->Attribute("Caption"));
		tab->setProperty("type", strTabWidget);
		tabWgt->setProperty("wrect", ComTool::String2Rect(element->Attribute("wrect")));
		QRect showPos = ComTool::String2Rect(element->Attribute("wrect"));
		tabWgt->setGeometry(showPos);
		tabWgt->show();

		XMLElement* childElement = element->FirstChildElement();
		CreateXML2UI(childElement, tab, vWgt, bView);
	}
	else if (element != nullptr && XMLUtil::StringEqual(element->Name(), strTextEdit.toStdString().c_str()))
	{
		AEditText* aeditText = new AEditText(objParent);
		aeditText->SetView(bView);
		if (!bView)
			ui->scrollAreaWidget->XmlCreateEditText(aeditText);
		if (objParent->property("type") == strTextEdit || objParent->property("type") == strGroup || objParent->property("type") == strTabWidget)
		{
			aeditText->disconnect(aeditText, &AEditText::sgnEditText, ui->scrollAreaWidget, &EditShowWidget::OnSlotEditText);
		}

		aeditText->setProperty("type", strTextEdit);
		aeditText->setProperty("name", element->Attribute("ObjectName"));
		aeditText->setProperty("addr", ComTool::Hex_String2Int(element->Attribute("addr")));
		aeditText->setProperty("len", ComTool::Hex_String2Int(element->Attribute("len")));
		aeditText->setProperty("default", ComTool::Hex_String2Int(element->Attribute("default")));
		aeditText->setProperty("ubuff", element->BoolAttribute("ubuff"));
		aeditText->setProperty("wrect", ComTool::String2Rect(element->Attribute("wrect")));
		aeditText->setProperty("passwd", element->BoolAttribute("passwd"));
		aeditText->setProperty("valuetype", ComTool::EditVaule_CString2Int(element->Attribute("valuetype")));
		aeditText->setProperty("shift", element->Attribute("shift"));
		aeditText->setProperty("showzerohead", element->BoolAttribute("showzerohead"));
		aeditText->setProperty("edian", ComTool::EditEndian_CString2Int(element->Attribute("edian")));
		aeditText->setProperty("edited", element->BoolAttribute("edited"));
		QRect showPos = ComTool::String2Rect(element->Attribute("wrect"));
		aeditText->setText(element->GetText());
		aeditText->setGeometry(showPos);
		aeditText->show();
	}
	else if (element != nullptr && XMLUtil::StringEqual(element->Name(), strGroup.toStdString().c_str()))
	{
		AGroupBox* agb = new AGroupBox(objParent);
		agb->SetView(bView);
		if (!bView)
			ui->scrollAreaWidget->XmlCreateGroupBox(agb);
		if (objParent->property("type") == strGroup || objParent->property("type") == strTabWidget)
		{
			agb->disconnect(agb, &AGroupBox::sgnGroupBox, ui->scrollAreaWidget, &EditShowWidget::OnSlotGroupBox);
		}
		agb->setProperty("type", strGroup);
		agb->setProperty("name", element->Attribute("ObjectName"));
		agb->setProperty("addr", ComTool::Hex_String2Int(element->Attribute("addr")));
		agb->setProperty("len", ComTool::Hex_String2Int(element->Attribute("len")));
		agb->setProperty("default", ComTool::Hex_String2Int(element->Attribute("default")));
		agb->setProperty("ubuff", element->BoolAttribute("ubuff"));
		agb->setProperty("wrect", ComTool::String2Rect(element->Attribute("wrect")));
		agb->setProperty("edited", element->BoolAttribute("edited"));
		QRect showPos = ComTool::String2Rect(element->Attribute("wrect"));
		agb->setTitle(element->Attribute("ObjectName"));
		agb->setName(element->Attribute("ObjectName"));
		agb->setGeometry(showPos);
		agb->show();

		XMLElement* childElement = element->FirstChildElement();
		CreateXML2UI(childElement, agb, vWgt, bView);
	}
	else if (element != nullptr && XMLUtil::StringEqual(element->Name(), strCheckBox.toStdString().c_str()))
	{
		ACheckBox* ackBox = new ACheckBox(objParent);
		ackBox->SetView(bView);
		if (!bView)
			ui->scrollAreaWidget->XmlCreateCheckBox(ackBox);
		if (objParent->property("type") == strCheckBox || objParent->property("type") == strGroup || objParent->property("type") == strTabWidget)
		{
			ackBox->disconnect(ackBox, &ACheckBox::sgnCheckBox, ui->scrollAreaWidget, &EditShowWidget::OnSlotCheckBox);
		}
		ackBox->setProperty("type", strCheckBox);
		ackBox->setProperty("name", element->Attribute("ObjectName"));
		ackBox->setProperty("addr", ComTool::Hex_String2Int(element->Attribute("addr")));
		ackBox->setProperty("len", ComTool::Hex_String2Int(element->Attribute("len")));
		ackBox->setProperty("default", ComTool::Hex_String2Int(element->Attribute("default")));
		ackBox->setProperty("ubuff", element->BoolAttribute("ubuff"));
		ackBox->setProperty("wrect", ComTool::String2Rect(element->Attribute("wrect")));
		ackBox->setProperty("value", ComTool::Hex_String2Int(element->Attribute("value")));
		ackBox->setProperty("mask", element->IntAttribute("mask"));
		ackBox->setProperty("edited", element->BoolAttribute("edited"));
		QRect showPos = ComTool::String2Rect(element->Attribute("wrect"));
		ackBox->setText(element->Attribute("ObjectName"));
		ackBox->setGeometry(showPos);
		ackBox->show();
	}
	else if (element != nullptr && XMLUtil::StringEqual(element->Name(), strComboBox.toStdString().c_str()))
	{
		AComboBox* acbBox = new AComboBox(objParent);
		acbBox->SetView(bView);
		if (!bView)
			ui->scrollAreaWidget->XmlCreateComboBox(acbBox);
		if (objParent->property("type") == strComboBox || objParent->property("type") == strGroup || objParent->property("type") == strTabWidget)
		{
			acbBox->disconnect(acbBox, &AComboBox::sgnComboBox, ui->scrollAreaWidget, &EditShowWidget::OnSlotComboBox);
		}
		acbBox->setProperty("type", strComboBox);
		acbBox->setProperty("name", element->Attribute("ObjectName"));
		acbBox->setProperty("addr", ComTool::Hex_String2Int(element->Attribute("addr")));
		acbBox->setProperty("len", ComTool::Hex_String2Int(element->Attribute("len")));
		acbBox->setProperty("default", ComTool::Hex_String2Int(element->Attribute("default")));
		acbBox->setProperty("ubuff", element->BoolAttribute("ubuff"));
		acbBox->setProperty("wrect", ComTool::String2Rect(element->Attribute("wrect")));
		acbBox->setProperty("mask", element->IntAttribute("mask"));
		acbBox->setProperty("edited", element->BoolAttribute("edited"));
		QRect showPos = ComTool::String2Rect(element->Attribute("wrect"));
		acbBox->setGeometry(showPos);
		acbBox->show();

		//comboBox比较特殊有子元素
		XMLElement* childElement = element->FirstChildElement();
		CreateXML2UI(childElement, acbBox, vWgt, bView);
		acbBox->setCurrentIndex(element->IntText());
	}
	else if (element != nullptr && XMLUtil::StringEqual(element->Name(), strLabel.toStdString().c_str()))
	{
		ALabel* alabel = new ALabel(objParent);
		alabel->SetView(bView);
		if (!bView)
		{
			ui->scrollAreaWidget->XmlCreateLabel(alabel);
		}
		if (objParent->property("type") == strLabel || objParent->property("type") == strGroup || objParent->property("type") == strTabWidget)
		{
			alabel->disconnect(alabel, &ALabel::sgnLabel, ui->scrollAreaWidget, &EditShowWidget::OnSlotLabel);
		}
		alabel->setProperty("type", strLabel);
		alabel->setProperty("name", element->Attribute("ObjectName"));
		alabel->setProperty("wrect", ComTool::String2Rect(element->Attribute("wrect")));
		QRect showPos = ComTool::String2Rect(element->Attribute("wrect"));
		alabel->setText(element->Attribute("ObjectName"));
		alabel->setGeometry(showPos);
		alabel->show();
	}
	else if (element != nullptr && XMLUtil::StringEqual(element->Name(), "ComboItem"))
	{
		AComboBox* acbBox = qobject_cast<AComboBox*>(objParent);
		QString name = element->Attribute("name");
		int nname = ComTool::Hex_String2Int(element->Attribute("value"));
		acbBox->addItem(element->Attribute("name"));
		acbBox->setProperty(element->Attribute("name"), ComTool::Hex_String2Int(element->Attribute("value")));
	}
	element = element->NextSiblingElement();
	CreateXML2UI(element, objParent, vWgt, bView);
}

void DriverParseWidget::ModifyLocationPropetry(const QVariant& vari)
{
	QRect eRect = vari.value<QRect>();
	if (eRect.isEmpty())
		return;

	if (m_curShowPropetry->property("type") == strTabWidget)
	{
		ATabWidget* tabWgt = qobject_cast<ATabWidget*>(m_curShowPropetry);
		tabWgt->setGeometry(eRect);
	}
	else if (m_curShowPropetry->property("type") == strGroup)
	{
		AGroupBox* agb = qobject_cast<AGroupBox*>(m_curShowPropetry);
		agb->setGeometry(eRect);
	}
	else if (m_curShowPropetry->property("type") == strTextEdit)
	{
		AEditText* aEdit = qobject_cast<AEditText*>(m_curShowPropetry);
		aEdit->setGeometry(eRect);
	}
	else if (m_curShowPropetry->property("type") == strCheckBox)
	{
		ACheckBox* ackBox = qobject_cast<ACheckBox*>(m_curShowPropetry);
		ackBox->setGeometry(eRect);
	}
	else if (m_curShowPropetry->property("type") == strComboBox)
	{
		AComboBox* acbBox = qobject_cast<AComboBox*>(m_curShowPropetry);
		acbBox->setGeometry(eRect);
	}
	else if (m_curShowPropetry->property("type") == strLabel)
	{
		ALabel* alab = qobject_cast<ALabel*>(m_curShowPropetry);
		alab->setGeometry(eRect);
	}
}

QString  DriverParseWidget::CreateNewFile()
{
	QString filePath = QFileDialog::getSaveFileName(this, "New Build File...", QDir::currentPath(), tr("*.xml"));

	if (!ComTool::CreateFile(filePath))
	{
		qDebug() << filePath << " save failed";
		return "";
	}

	m_curOperFile = filePath;

	emit sgnWindowTitle(filePath);

	return filePath;
}

void DriverParseWidget::ClearWgt()
{
	ui->scrollAreaWidget->ClearChildList(ui->scrollAreaWidget->children());
	ui->scrollAreaWidget->ClearAllVec();
	m_curShowPropetry = nullptr;
	ui->propetryTable->clear();

	ui->treeWidget->collapseAll();
	int itemChildrenCounts = ui->treeWidget->topLevelItem(0)->childCount();
	while (itemChildrenCounts--)
	{
		QTreeWidgetItem* child = ui->treeWidget->topLevelItem(0)->child(itemChildrenCounts); //index从大到小区做删除处理
		ui->treeWidget->topLevelItem(0)->removeChild(child);
		delete child;
		child = nullptr;
	}

	m_curXmlDoc = nullptr;
	m_curXmlDoc = new tinyxml2::XMLDocument();
	m_mapAlgoElement.clear();
}

bool DriverParseWidget::CheckStrHex(QString& strHex)
{
	bool isHex = true;

	//判断是否是16进制
	int mulPos = strHex.lastIndexOf("-");
	if (mulPos != -1)
	{
		//避免出现1-1这种输入,直接规避掉输出为1
		QStringList inputList = strHex.split("-");
		if (inputList[0] == inputList[1])
		{
			strHex = inputList[0];

			strHex.toInt(&isHex, 16);
			if (!isHex)
			{
				qDebug() << QString::fromLocal8Bit("输入不是16进制");
				return isHex;
			}
		}

	}
	else
	{
		strHex.toInt(&isHex, 16);
		if (!isHex)
		{
			qDebug() << QString::fromLocal8Bit("输入不是16进制");
			return isHex;
		}
	}


	//先判断是否存在相同节点
	QTreeWidgetItemIterator it(ui->treeWidget);
	while (*it) {
		if ((*it) == ui->treeWidget->topLevelItem(0))
		{
			it++;
			continue;
		}


		if ((*it)->text(0) == ("Algo:" + strHex))
		{
			qDebug() << QString::fromLocal8Bit("已存在相同节点");
			return !isHex;
		}

		//去除Algo标志
		QString curNode = (*it)->text(0);
		int idxPos = curNode.lastIndexOf("Algo:");
		curNode = curNode.mid(idxPos + QString("Algo:").size(), curNode.length());

		int mulPos = strHex.lastIndexOf("-");
		if(mulPos == -1)
		{
			//判断是否在单节点的算法范围内
			QStringList idxList = curNode.split("-");
			if (idxList.count() > 1)
			{
				bool isOk;
				int leftBorder = idxList[0].toInt(&isOk, 16);
				int rightBorder = idxList[1].toInt(&isOk, 16);
				int compareNum = strHex.toInt(&isOk, 16);

				if(isOk)

				if (compareNum <= rightBorder && compareNum >= leftBorder)
				{
					qDebug() << QString::fromLocal8Bit("已存在相同节点");
					return !isHex;
				}
			}
		}
		else
		{
			QStringList inputList = strHex.split("-");
			QStringList idxList = curNode.split("-");
			if (inputList.count() > 1)
			{
				bool isInputLeftOk, isInputRightOk;
				int comLeftNum = inputList[0].toInt(&isInputLeftOk, 16);
				int comRightNum = inputList[1].toInt(&isInputRightOk, 16);
				if (idxList.count() > 1)
				{
					if (!isInputLeftOk || !isInputRightOk)
					{
						qDebug() << QString::fromLocal8Bit("输入不是16进制");
						return false;
					}

					bool isLeftBorder, isRightBorder;
					int leftBorder = idxList[0].toInt(&isLeftBorder, 16);
					int rightBorder = idxList[1].toInt(&isRightBorder, 16);

					if (!isLeftBorder || !isRightBorder)
					{
						qDebug() << QString::fromLocal8Bit("输入不是16进制");
						return false;
					}

					if ((comLeftNum <= rightBorder && comLeftNum >= leftBorder)
						|| (comRightNum <= rightBorder && comRightNum >= leftBorder))
					{
						qDebug() << QString::fromLocal8Bit("已存在相同节点");
						return !isHex;
					}
				}
				else
				{
					bool isOk;
					int borderNum = idxList[0].toInt(&isOk, 16);
					if (!isOk)
					{
						qDebug() << QString::fromLocal8Bit("输入不是16进制");
						return false;
					}

					if (borderNum <= comRightNum && borderNum >= comLeftNum)
					{
						qDebug() << QString::fromLocal8Bit("已存在相同节点");
						return !isHex;
					}
				}
			}
		}

		++it;
	}
	return isHex;
}

void DriverParseWidget::CreateXMLDefaultHead()
{
	XMLDeclaration* declaration = m_curXmlDoc->NewDeclaration();
	m_curXmlDoc->InsertFirstChild(declaration);

	XMLElement* root = m_curXmlDoc->NewElement("ChipConfig");
	m_curXmlDoc->InsertEndChild(root);

	QString name = QInputDialog::getText(this, tr("Drv Rename"), tr("Set a Device Name"), QLineEdit::Normal);

	XMLElement* device = m_curXmlDoc->NewElement("Device");
	device->SetAttribute("DrvName", name.toStdString().c_str());//获取设备需要单独提供接口
	device->SetAttribute("Version", "1.0");
	device->SetAttribute("Date", QDateTime::currentDateTime().toString("yyyyMMdd").toStdString().c_str());
	root->InsertEndChild(device);

	XMLElement* chipList = m_curXmlDoc->NewElement("ChipList");
	device->InsertEndChild(chipList);

	if(!m_curOperFile.isEmpty())
		m_curXmlDoc->SaveFile(m_curOperFile.toStdString().c_str());
}

void DriverParseWidget::OnSlotShowPropetry(QObject* obj)
{
	m_curShowPropetry = nullptr;
	ui->propetryTable->clear();
	QtVariantPropertyManager* pVarManager = new QtVariantPropertyManager(ui->propetryTable);
	QtVariantEditorFactory* pVarFactory = new QtVariantEditorFactory(ui->propetryTable);

	//通用属性
	QtVariantProperty* item = pVarManager->addProperty(QVariant::String, tr("name"));
	item->setValue(obj->property("name").toString());
	ui->propetryTable->addProperty(item);

	item = pVarManager->addProperty(QVariant::Rect, tr("wrect"));
	item->setValue(obj->property("wrect").toRect());
	qDebug() << "OnSlotShowPropetry: " << obj->property("wrect").toRect();
	ui->propetryTable->addProperty(item);

	if (strGroup == obj->property("type") || strTextEdit == obj->property("type")
		|| strCheckBox == obj->property("type") || strComboBox == obj->property("type"))
	{
		item = pVarManager->addProperty(QVariant::Int, tr("addr"));
		item->setValue(obj->property("addr"));
		ui->propetryTable->addProperty(item);

		item = pVarManager->addProperty(QVariant::Int, tr("len"));
		item->setValue(obj->property("len"));
		ui->propetryTable->addProperty(item);

		item = pVarManager->addProperty(QVariant::Int, tr("default"));
		item->setValue(obj->property("default"));
		ui->propetryTable->addProperty(item);

		item = pVarManager->addProperty(QVariant::Bool, tr("ubuff"));
		item->setValue(obj->property("ubuff"));
		ui->propetryTable->addProperty(item);

		item = pVarManager->addProperty(QVariant::Bool, tr("edited"));
		item->setValue(obj->property("edited"));
		ui->propetryTable->addProperty(item);
	}

	//控件特殊属性
	if (strTextEdit == obj->property("type"))
	{
		item = pVarManager->addProperty(QVariant::Bool, tr("passwd"));
		item->setValue(obj->property("passwd"));
		ui->propetryTable->addProperty(item);

		item = pVarManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("valuetype"));
		QStringList valueTypes;
		valueTypes	<< "DEC" << "HEX" << "STR";
		item->setAttribute(QLatin1String("enumNames"), valueTypes);

		item->setValue(obj->property("valuetype"));
		ui->propetryTable->addProperty(item);

		item = pVarManager->addProperty(QVariant::String, tr("shift"));
		item->setValue(obj->property("shift"));
		ui->propetryTable->addProperty(item);

		item = pVarManager->addProperty(QVariant::Bool, tr("showzerohead"));
		item->setValue(obj->property("showzerohead"));
		ui->propetryTable->addProperty(item);

		item = pVarManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("endian"));
		QStringList endianTypes;
		endianTypes << "L" << "R";
		item->setAttribute(QLatin1String("enumNames"), endianTypes);

		item->setValue(obj->property("showzerohead"));
		ui->propetryTable->addProperty(item);
	}
	else if (strComboBox == obj->property("type"))
	{
		item = pVarManager->addProperty(QVariant::Int, tr("mask"));
		item->setValue(obj->property("mask"));
		ui->propetryTable->addProperty(item);

		QtProperty* groupItem = pVarManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("ComboItem"));

		AComboBox* acbBox = qobject_cast<AComboBox*>(obj);
		for (int i = 0; i < acbBox->count(); ++i)
		{
			item = pVarManager->addProperty(QVariant::String, acbBox->itemText(i).toStdString().c_str());
			item->setValue(acbBox->property(acbBox->itemText(i).toStdString().c_str()));
			groupItem->addSubProperty(item);
		}

		ui->propetryTable->addProperty(groupItem);
	}
	else if (strCheckBox == obj->property("type"))
	{
		item = pVarManager->addProperty(QVariant::Int, tr("mask"));
		item->setValue(obj->property("mask"));
		ui->propetryTable->addProperty(item);

		item = pVarManager->addProperty(QVariant::Int, tr("value"));
		item->setValue(obj->property("value"));
		ui->propetryTable->addProperty(item);
	}

	connect(pVarManager, SIGNAL(valueChanged(QtProperty*, const QVariant&)), this, SLOT(variantPropertyValueChanged(QtProperty*, const QVariant&)));
	m_curShowPropetry = obj;
	ui->propetryTable->setFactoryForManager(pVarManager, pVarFactory);
}

void DriverParseWidget::OnSlotClearPropetry(QObject*)
{
	m_curShowPropetry = nullptr;
	ui->propetryTable->clear();
}

void DriverParseWidget::variantPropertyValueChanged(QtProperty* property, const QVariant& value)
{
	qDebug() << property->propertyName() << ":" << property->valueText();
	m_curShowPropetry->setProperty(property->propertyName().toStdString().c_str(), value);
	ModifyLocationPropetry(value);

	//修改文本
	if (m_curShowPropetry->property("type") == strGroup)
	{
		AGroupBox* agb = qobject_cast<AGroupBox*>(m_curShowPropetry);
		if(property->propertyName() == "name")
			agb->setTitle(value.toString());
	}
	else if (m_curShowPropetry->property("type") == strTextEdit)
	{
		AEditText* aEdit = qobject_cast<AEditText*>(m_curShowPropetry);
		if (property->propertyName() == "name")
			aEdit->setText(value.toString());
	}
	else if (m_curShowPropetry->property("type") == strCheckBox)
	{
		ACheckBox* ackBox = qobject_cast<ACheckBox*>(m_curShowPropetry);
		if (property->propertyName() == "name")
			ackBox->setText(value.toString());
	}
	else if (m_curShowPropetry->property("type") == strLabel)
	{
		ALabel* alab = qobject_cast<ALabel*>(m_curShowPropetry);
		if (property->propertyName() == "name")
			alab->setText(value.toString());
	}
}

void DriverParseWidget::OnSlotAddTreeItem()
{
	QTreeWidgetItem* topItem = ui->treeWidget->topLevelItem(0);

	QString name = QInputDialog::getText(this, tr("Rename"), tr("Enter your new Algo name, must Hex"), QLineEdit::Normal);

	if (!name.isEmpty())
	{
		//判断是否输入为16进制
		bool isHex = CheckStrHex(name);

		if (isHex)
		{
			QTreeWidgetItem* addItem = new QTreeWidgetItem(topItem);
			addItem->setText(topItem->columnCount() - 1, "Algo:" + name.toUpper());
			topItem->addChild(addItem);
			ui->treeWidget->setCurrentItem(addItem);

			m_mapAlgoElement[addItem] = m_curXmlDoc->NewElement("Chip");
			m_mapAlgoElement[addItem]->SetAttribute("Algo", name.toStdString().c_str());
		}
	}
}

void DriverParseWidget::OnSlotDelTreeItem()
{
	QTreeWidgetItem* delItem = ui->treeWidget->currentItem();
	ui->treeWidget->topLevelItem(0)->removeChild(delItem);
}

void DriverParseWidget::OnSlotItemClicked(QTreeWidgetItem* clickItem, int col)
{
	m_curSelItemCol = col;
}

void DriverParseWidget::OnSlotItemDoubleClicked(QTreeWidgetItem* clickItem, int col)
{
	//创建新的之前添加判断之前的是否需要保存
	if (ui->scrollAreaWidget->EditWgtChildren() > 0)
	{
		int ret = QMessageBox::warning(this, tr("Warning"), tr("There are modifications in the editing area. Do you want to save them first ?"),
			QMessageBox::Ok, QMessageBox::Cancel);

		if (ret == QMessageBox::Ok)
		{
			ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0)->child(m_curSelItemCol));
			return;
		}
	}

	m_curSelItemCol = col;
	//根据item的text获取算法STR，在xml去找对应UI。没有则直接进行编辑
	std::map<QTreeWidgetItem*, XMLElement*>::iterator iter = m_mapAlgoElement.begin();
	while (iter != m_mapAlgoElement.end())
	{
		if (clickItem == iter->first)
		{
			ui->scrollAreaWidget->ClearChildList(ui->scrollAreaWidget->children());
			ui->scrollAreaWidget->ClearAllVec();
			m_curShowPropetry = nullptr;
			ui->propetryTable->clear();

			XMLElement* proSheet = iter->second->FirstChildElement();
			while (proSheet != nullptr && !XMLUtil::StringEqual(proSheet->Name(), "PropertySheet"))
			{
				proSheet = proSheet->FirstChildElement();
			}

			//样式节点 依次获取
			if (proSheet != nullptr)
			{
				XMLElement* subSheetNode = proSheet->FirstChildElement();
				ViewWidget vWgt;
				CreateXML2UI(subSheetNode, ui->scrollAreaWidget, vWgt);
				break;
			}
		}
		iter++;
	}
}

void DriverParseWidget::OnSlotSaveUI2XML()
{
	//对EditWgt界面窗口的子类进行解析成xml
	if (ui->treeWidget->currentItem() == nullptr || (ui->treeWidget->topLevelItem(0) == ui->treeWidget->currentItem()))
	{
		int ret = QMessageBox::warning(this, tr("Warning"), tr("Please select an algorithm node"),
			QMessageBox::Ok , QMessageBox::Ok);

		return;
	}

	if (m_curOperFile.isEmpty())
	{
		m_curOperFile = CreateNewFile();
		CreateXMLDefaultHead();
	}

	int ret = CreateXML(m_curOperFile);
}

void DriverParseWidget::OnSlotOpenViewWgt()
{
	ViewWidget view;

	//先将编辑区保存至XML
	XMLDocument* doc = new XMLDocument();
	XMLElement* algoXml = nullptr;
	if (ui->treeWidget->currentItem() != nullptr)
	{
		QStringList strList = ui->treeWidget->currentItem()->text(m_curSelItemCol).split(":");
		if (strList.count() > 0)
		{
			algoXml = doc->NewElement("Chip");
			algoXml->SetAttribute("Algo", strList[strList.count() - 1].toStdString().c_str());
			doc->InsertEndChild(algoXml);
		}
	}

	XMLElement* proSheet = doc->NewElement("PropertySheet");
	if (algoXml)
		algoXml->InsertEndChild(proSheet);
	else
		doc->InsertEndChild(proSheet);
	CreateChildElement(ui->scrollAreaWidget->children(), proSheet, doc);

	//读XML还原至预览弹窗
	XMLElement* subSheetNode = proSheet->FirstChildElement();
	CreateXML2UI(subSheetNode, view.GetWidget(), view, true);

	view.InitState();
	view.SetDefaultXML(doc);

	view.exec();
}

void DriverParseWidget::OnSlotNewFile()
{
	//创建新的之前添加判断之前的是否需要保存
	if (ui->scrollAreaWidget->EditWgtChildren() > 0)
	{
		int ret = QMessageBox::warning(this, tr("Warning"), tr("There are modifications in the editing area. Do you want to save them first ?"),
			QMessageBox::Ok, QMessageBox::Cancel);

		if (ret == QMessageBox::Ok)
		{
			return;
		}
	}

	if (!CreateNewFile().isEmpty())
	{
		ClearWgt();

		CreateXMLDefaultHead();
	}
}

void DriverParseWidget::OnSlotLoadFile()
{
	//创建新的之前添加判断之前的是否需要保存
	if (ui->scrollAreaWidget->EditWgtChildren() > 0)
	{
		int ret = QMessageBox::warning(this, tr("Warning"), tr("There are modifications in the editing area. Do you want to save them first ?"),
			QMessageBox::Ok, QMessageBox::Cancel);

		if (ret == QMessageBox::Ok)
		{
			return;
		}

		ClearWgt();
	}

	QString filePath = QFileDialog::getOpenFileName(this, "Open New File...", QDir::currentPath(), tr("*.xml"));

	m_curOperFile = filePath;

	if (m_curXmlDoc->LoadFile(filePath.toStdString().c_str()) != 0) {
		qDebug() << "load xml file failed";
		return;
	}

	XMLElement* algoShowElement;
	//遍历所有算法节点,<Chip> 兄弟节点只会是<Chip>
	{
		XMLElement* algoElement = m_curXmlDoc->RootElement()->FirstChildElement();
		while (algoElement != nullptr && !XMLUtil::StringEqual(algoElement->Name(), "Chip"))
		{
			algoElement = algoElement->FirstChildElement();
		}
		algoShowElement = algoElement;
		while (algoElement != nullptr)
		{
			QTreeWidgetItem* addItem = new QTreeWidgetItem(ui->treeWidget->topLevelItem(0));
			QString algoName = algoElement->Attribute("Algo");
			addItem->setText(ui->treeWidget->topLevelItem(0)->columnCount() - 1, "Algo:" + algoName);
			ui->treeWidget->topLevelItem(0)->addChild(addItem);
			ui->treeWidget->setCurrentItem(addItem);
			m_mapAlgoElement[addItem] = algoElement;

			algoElement = algoElement->NextSiblingElement();
		}
		ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0)->child(0));
	}

	//遍历找到样式节点
	if (algoShowElement != nullptr)
	{
		XMLElement* proSheet = algoShowElement->FirstChildElement();
		while (proSheet != nullptr && !XMLUtil::StringEqual(proSheet->Name(), "PropertySheet"))
		{
			proSheet = proSheet->FirstChildElement();
		}

		//样式节点 依次获取
		if (proSheet != nullptr)
		{
			XMLElement* subSheetNode = proSheet->FirstChildElement();
			ViewWidget vWgt;
			CreateXML2UI(subSheetNode, ui->scrollAreaWidget, vWgt);
		}
	}

	emit sgnWindowTitle(filePath);
}

void DriverParseWidget::OnSlotClearCurrentEditWgt()
{
	ClearWgt();
	m_curOperFile = "";
	emit sgnWindowTitle("untitle");
}
