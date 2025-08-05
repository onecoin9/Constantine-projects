#include "AngKProjectChipConfigWidget.h"
#include "AngkLogger.h"
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QFileDialog>

AngKProjectChipConfigWidget::AngKProjectChipConfigWidget(QWidget* parent)
	: QWidget(parent)
{
	m_pLayout = new QVBoxLayout(this);
	this->setLayout(m_pLayout);
}

AngKProjectChipConfigWidget::~AngKProjectChipConfigWidget()
{
}

int AngKProjectChipConfigWidget::CheckCurWgtIndex(QString strCtrl)
{
	int idx = -1;
	if (strCtrl == strGroup)
	{
		for (int i = 0; i < m_vecGroup.size(); ++i)
		{
			if (m_vecGroup[i] == nullptr)
			{
				idx = i + 1;
				break;
			}
		}
	}
	else if (strCtrl == strLabel)
	{
		for (int i = 0; i < m_vecLabel.size(); ++i)
		{
			if (m_vecLabel[i] == nullptr)
			{
				idx = i + 1;
				break;
			}
		}
	}
	else if (strCtrl == strTextEdit)
	{
		for (int i = 0; i < m_vecEditText.size(); ++i)
		{
			if (m_vecEditText[i] == nullptr)
			{
				idx = i + 1;
				break;
			}
		}
	}
	else if (strCtrl == strCheckBox)
	{
		for (int i = 0; i < m_vecCheckBox.size(); ++i)
		{
			if (m_vecCheckBox[i] == nullptr)
			{
				idx = i + 1;
				break;
			}
		}
	}
	else if (strCtrl == strComboBox)
	{
		for (int i = 0; i < m_vecComboBox.size(); ++i)
		{
			if (m_vecComboBox[i] == nullptr)
			{
				idx = i + 1;
				break;
			}
		}
	}
	else if (strCtrl == strTabWidget)
	{
		for (int i = 0; i < m_vecTabWidget.size(); ++i)
		{
			if (m_vecTabWidget[i] == nullptr)
			{
				idx = i + 1;
				break;
			}
		}
	}

	return idx;
}

void AngKProjectChipConfigWidget::ClearAllVec()
{
	for (auto _ptr : m_vecGroup)
	{
		if (_ptr)
			_ptr->close();
		_ptr = nullptr;
		delete _ptr;
	}
	for (auto _ptr : m_vecLabel)
	{
		if (_ptr)
			_ptr->close();
		_ptr = nullptr;
		delete _ptr;
	}
	for (auto _ptr : m_vecEditText)
	{
		if (_ptr)
			_ptr->close();
		_ptr = nullptr;
		delete _ptr;
	}
	for (auto _ptr : m_vecCheckBox)
	{
		if (_ptr)
			_ptr->close();
		_ptr = nullptr;
		delete _ptr;
	}
	for (auto _ptr : m_vecComboBox)
	{
		if (_ptr)
			_ptr->close();
		_ptr = nullptr;
		delete _ptr;
	}
	for (auto _ptr : m_vecTabWidget)
	{
		if (_ptr)
			_ptr->close();
		_ptr = nullptr;
		delete _ptr;
	}
	m_vecGroup.clear();
	m_vecLabel.clear();
	m_vecEditText.clear();
	m_vecCheckBox.clear();
	m_vecComboBox.clear();
	m_vecTabWidget.clear();
}

QTabWidget* AngKProjectChipConfigWidget::IsSubTabWgt(QString strName)
{
	bool bFind = false;

	for (auto item : m_vecTabWidget)
	{
		if (item != nullptr && strName == item->property("name").toString())
		{
			bFind = true;
			return item;
		}
	}

	return nullptr;
}

int AngKProjectChipConfigWidget::XmlCreateGroupBox(QGroupBox* agb)
{
	int idx = CheckCurWgtIndex(strGroup);
	if (idx == -1)
	{
		m_vecGroup.push_back(agb);
		idx = m_vecGroup.size();
	}
	else
	{
		m_vecGroup[idx - 1] = agb;
	}

	agb->setProperty("index", idx);
	agb->setProperty("type", strGroup);

	return idx;
}

int AngKProjectChipConfigWidget::XmlCreateLabel(QLabel* aLabel)
{
	int idx = CheckCurWgtIndex(strLabel);
	if (idx == -1)
	{
		m_vecLabel.push_back(aLabel);
		idx = m_vecLabel.size();
	}
	else
	{
		m_vecLabel[idx - 1] = aLabel;
	}

	aLabel->setProperty("index", idx);
	aLabel->setProperty("type", strLabel);

	return idx;
}

int AngKProjectChipConfigWidget::XmlCreateEditText(QTextEdit* aEditText)
{
	int idx = CheckCurWgtIndex(strTextEdit);
	if (idx == -1)
	{
		m_vecEditText.push_back(aEditText);
		idx = m_vecEditText.size();
	}
	else
	{
		m_vecEditText[idx - 1] = aEditText;
	}

	aEditText->setProperty("index", idx);
	aEditText->setProperty("type", strTextEdit);

	return idx;
}

int AngKProjectChipConfigWidget::XmlCreateCheckBox(QCheckBox* ackBox)
{
	int idx = CheckCurWgtIndex(strCheckBox);
	if (idx == -1)
	{
		m_vecCheckBox.push_back(ackBox);
		idx = m_vecCheckBox.size();
	}
	else
	{
		m_vecCheckBox[idx - 1] = ackBox;
	}

	ackBox->setProperty("index", idx);
	ackBox->setProperty("type", strCheckBox);

	return idx;
}

int AngKProjectChipConfigWidget::XmlCreateComboBox(QComboBox* acbBox)
{
	int idx = CheckCurWgtIndex(strComboBox);
	if (idx == -1)
	{
		m_vecComboBox.push_back(acbBox);
		idx = m_vecComboBox.size();
	}
	else
	{
		m_vecComboBox[idx - 1] = acbBox;
	}

	acbBox->setProperty("index", idx);
	acbBox->setProperty("type", strComboBox);
	return idx;
}

int AngKProjectChipConfigWidget::XmlCreateTabWidget(QTabWidget* tabWgt)
{
	int idx = CheckCurWgtIndex(strTabWidget);
	if (idx == -1)
	{
		m_vecTabWidget.push_back(tabWgt);
		idx = m_vecTabWidget.size();
	}
	else
	{
		m_vecTabWidget[idx - 1] = tabWgt;
	}

	tabWgt->setProperty("index", idx);
	tabWgt->setProperty("type", strTabWidget);

	return idx;
}

void AngKProjectChipConfigWidget::AddSubWidget(QWidget* _subWgt)
{
	m_pLayout->addWidget(_subWgt);
}

void AngKProjectChipConfigWidget::SetCurrentDevAlgo(QString strDev, uint32_t uAlgo)
{
	m_strDevName = strDev;
	m_nAlgo = uAlgo;
}

void AngKProjectChipConfigWidget::GetNodeValue(pugi::xml_node sheetXmlNode)
{
	if (sheetXmlNode == nullptr)
		return;

	if (sheetXmlNode.name() == strTabWidget || sheetXmlNode.name() == strGroup) {
		pugi::xml_node propertyNode = sheetXmlNode.first_child();
		GetNodeValue(propertyNode);
	}
	else {
		std::string strObjName = sheetXmlNode.attribute("ObjectName").as_string();
		if (!strObjName.empty())
			m_mapNodeValue[strObjName] = sheetXmlNode.text().as_string();
	}

	sheetXmlNode = sheetXmlNode.next_sibling();
	GetNodeValue(sheetXmlNode);
}

void AngKProjectChipConfigWidget::SetNodeValue()
{
	for (auto textVar : m_vecEditText) {
		std::string strNodeValue = textVar->property("name").toString().toStdString();
		if (m_mapNodeValue.find(strNodeValue) != m_mapNodeValue.end()) {
			textVar->setText(QString::fromStdString(m_mapNodeValue[strNodeValue]));
		}
	}

	for (auto checkVar : m_vecCheckBox) {
		std::string strNodeValue = checkVar->property("name").toString().toStdString();
		if (m_mapNodeValue.find(strNodeValue) != m_mapNodeValue.end()) {
			bool bCheck = m_mapNodeValue[strNodeValue] == "0" ? false : true;
			checkVar->setChecked(bCheck);
		}
	}

	for (auto labelVar : m_vecLabel) {
		std::string strNodeValue = labelVar->property("name").toString().toStdString();
		if (m_mapNodeValue.find(strNodeValue) != m_mapNodeValue.end()) {
			labelVar->setText(QString::fromStdString(m_mapNodeValue[strNodeValue]));
		}
	}

	for (auto comboVar : m_vecComboBox) {
		std::string strNodeValue = comboVar->property("name").toString().toStdString();
		if (m_mapNodeValue.find(strNodeValue) != m_mapNodeValue.end()) {
			QString comboNum = QString::fromStdString(m_mapNodeValue[strNodeValue]);
			if(comboNum.toInt() < comboVar->count())
				comboVar->setCurrentIndex(comboNum.toInt());
		}
	}
}

int AngKProjectChipConfigWidget::SaveNodeValue(QString saveFile)
{
	pugi::xml_document doc;

	pugi::xml_node root = doc.append_child("ChipConfig");
	pugi::xml_node deviceNode = root.append_child("Device");
	deviceNode.append_attribute("DrvName").set_value(m_strDevName.toStdString().c_str());
	pugi::xml_node algoNode = deviceNode.append_child("Chip");
	algoNode.append_attribute("Algo").set_value(QString::number(m_nAlgo).toStdString().c_str());
	pugi::xml_node sheetNode = algoNode.append_child("PropertySheet");

	
	ReadNodeValue(this->children(), sheetNode);

	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(saveFile.utf16());
	if (!doc.save_file(encodedName, "\t", pugi::format_default, pugi::encoding_utf8)) {
		ALOG_ERROR("%s", "CU", "--", "Failed to save XML document to file.");
		return XMLMESSAGE_LOAD_FAILED;
	}

	return XMLMESSAGE_SUCCESS;
}

void AngKProjectChipConfigWidget::ReadNodeValue(QObjectList varObjs, pugi::xml_node saveNode)
{
	if (varObjs.isEmpty() || saveNode == nullptr)
		return;

	for (auto varObj : varObjs) {
		QString varName = varObj->property("type").toString();
		if (varName == strTabWidget) {
			QTabWidget* tabWgt = qobject_cast<QTabWidget*>(varObj);
			for (int i = 0; i < tabWgt->count(); ++i) {
				auto tabSubWgtList = tabWgt->widget(i)->children();
				pugi::xml_node tabPageNode = saveNode.append_child("TabPage");
				tabPageNode.append_attribute("ObjectName").set_value(tabWgt->property("name").toString().toStdString().c_str());
				ReadNodeValue(tabSubWgtList, tabPageNode);
			}
		}
		else if (varName == strGroup) {
			QGroupBox* groupWgt = qobject_cast<QGroupBox*>(varObj);
			auto groupSubWgtList = groupWgt->children();
			pugi::xml_node groupNode = saveNode.append_child("Group");
			groupNode.append_attribute("ObjectName").set_value(groupWgt->property("name").toString().toStdString().c_str());
			ReadNodeValue(groupSubWgtList, groupNode);
		}
		else if (varName == strTextEdit) {
			QTextEdit* textWgt = qobject_cast<QTextEdit*>(varObj);
			pugi::xml_node subPageNode = saveNode.append_child(varName.toStdString().c_str());
			subPageNode.append_attribute("ObjectName").set_value(varObj->property("name").toString().toStdString().c_str());
			subPageNode.text().set(textWgt->toPlainText().toStdString().c_str());
		}
		else if (varName == strLabel) {
			QLabel* labelWgt = qobject_cast<QLabel*>(varObj);
			pugi::xml_node subPageNode = saveNode.append_child(varName.toStdString().c_str());
			subPageNode.append_attribute("ObjectName").set_value(varObj->property("name").toString().toStdString().c_str());
			subPageNode.text().set(labelWgt->text().toStdString().c_str());
		}
		else if (varName == strCheckBox) {
			QCheckBox* checkWgt = qobject_cast<QCheckBox*>(varObj);
			int nCheck = checkWgt->isChecked() ? 1 : 0;
			pugi::xml_node subPageNode = saveNode.append_child(varName.toStdString().c_str());
			subPageNode.append_attribute("ObjectName").set_value(varObj->property("name").toString().toStdString().c_str());
			subPageNode.text().set(std::to_string(nCheck).c_str());
		}
		else if (varName == strComboBox) {
			QComboBox* comboWgt = qobject_cast<QComboBox*>(varObj);
			pugi::xml_node subPageNode = saveNode.append_child(varName.toStdString().c_str());
			subPageNode.append_attribute("ObjectName").set_value(varObj->property("name").toString().toStdString().c_str());
			subPageNode.text().set(std::to_string(comboWgt->currentIndex()).c_str());
		}
	}
}

int AngKProjectChipConfigWidget::LoadNodeValue(QString loadFile)
{
	m_mapNodeValue.clear();

	pugi::xml_document doc;
	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(loadFile.utf16());
	pugi::xml_parse_result result = doc.load_file(encodedName);
	pugi::xml_node root_node = doc.child(XML_NODE_CHIPCONFIG);
	if (!result)
		return XMLMESSAGE_LOAD_FAILED;

	pugi::xml_node deviceNode = root_node.child(XML_NODE_CHIPCONFIG_DEVICE);
	if (m_strDevName != deviceNode.attribute("DrvName").as_string()) {
		ALOG_ERROR("%s", "CU", "--","The device name of the loaded file does not match.");
		return XMLMESSAGE_NODE_COMPARE_FAILED;
	}

	pugi::xml_node chipNode = deviceNode.child(XML_NODE_CHIPCONFIG_CHIP);
	if (!Utils::AngKCommonXMLParser::CheckAlgoRange(m_nAlgo, chipNode.attribute("Algo").as_string())) {
		ALOG_ERROR("%s", "CU", "--", "The algorithm ID value of the loaded file does not match.");
		return XMLMESSAGE_NODE_COMPARE_FAILED;
	}

	pugi::xml_node propertysheetNode = chipNode.child(XML_NODE_CHIPCONFIG_PROPERTYSHEET);
	pugi::xml_node propertyNode = propertysheetNode.first_child();

	GetNodeValue(propertyNode);

	return XMLMESSAGE_SUCCESS;
}

void AngKProjectChipConfigWidget::onSlotLoadConfigXML()
{
	QString xmlFilePath = QFileDialog::getOpenFileName(this, "Select File...", QCoreApplication::applicationDirPath(), tr(" bin Files(*.xml);; All Files(*.*)"));

	LoadNodeValue(xmlFilePath);

	SetNodeValue();
}

void AngKProjectChipConfigWidget::onSlotSaveConfigXML()
{
	QString xmlFilePath = QFileDialog::getSaveFileName(this, "Select File...", QCoreApplication::applicationDirPath(), tr("All Files(*.*)"));

	SaveNodeValue(xmlFilePath);


}
