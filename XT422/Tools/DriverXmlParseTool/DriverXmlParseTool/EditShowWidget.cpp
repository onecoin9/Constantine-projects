#include "EditShowWidget.h"
#include "ACheckBox.h"
#include "AComboBox.h"
#include "AEditText.h"
#include "AGroupBox.h";
#include "ALabel.h"
#include "ATabWidget.h"

bool operator <(QPoint a, QPoint b)
{
	if (a.x() < b.x())
	{
		return true;
	}
	return false;
}

EditShowWidget::EditShowWidget(QWidget *parent)
	: QWidget(parent)
	, m_curSelect(nullptr)
	, m_winSizeDlg(nullptr)
{
	setAcceptDrops(true);
	m_winSizeDlg = new SetWinSize(this);
}

EditShowWidget::~EditShowWidget()
{
	ClearAllVec();
}

int EditShowWidget::CheckCurWgtIndex(QString strCtrl)
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

void EditShowWidget::ClearAllVec()
{
	for (auto _ptr : m_vecGroup)
	{
		if(_ptr)
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

void EditShowWidget::DropCreateGroupBox(QPoint drop)
{
	AGroupBox* group = new AGroupBox(this);
	//自动命名
	int idx = XmlCreateGroupBox(group);

	//设置新属性
	group->setProperty("name", (strGroup.toStdString() + std::to_string(idx)).c_str());
	group->setProperty("addr", 123);	//从偏移地址0x0开始
	group->setProperty("len", 10);	//占用长度
	group->setProperty("default", 0);//默认偏移地址
	group->setProperty("ubuff", true);
	group->setProperty("wrect", QRect(drop.x(),drop.y(), group->width(), group->height()));
	group->setProperty("edited", true);	//是否可以编辑

	//Scroll非常坑，新加入的控件默认隐藏，需要设置拖动位置和show();
	group->setGeometry(drop.x(), drop.y(), group->width(), group->height());
	group->setTitle(strGroup + QString::number(idx));
	group->setName(strGroup + QString::number(idx));
	group->show();
}

void EditShowWidget::DropCreateLabel(QPoint drop)
{
	ALabel* label = new ALabel(this);
	//自动命名
	int idx = XmlCreateLabel(label);
	//设置新属性
	label->setProperty("name", (strLabel.toStdString() + std::to_string(idx)).c_str());
	label->setProperty("wrect", QRect(drop.x(), drop.y(), label->width(), label->height()));

	label->setText(strLabel + QString::number(idx));
	label->setGeometry(drop.x(), drop.y(), label->width(), label->height());
	label->show();
}

void EditShowWidget::DropCreateEditText(QPoint drop)
{
	AEditText* editText = new AEditText(this);
	//自动命名
	int idx = XmlCreateEditText(editText);
	//设置新属性
	editText->setProperty("name", (strTextEdit.toStdString() + std::to_string(idx)).c_str());
	editText->setProperty("addr", 0);	//从偏移地址0x0开始
	editText->setProperty("len", 0);	//占用长度
	editText->setProperty("default", 0);//默认偏移地址
	editText->setProperty("ubuff", true);
	editText->setProperty("wrect", QRect(drop.x(), drop.y(), editText->width(), editText->height()));
	editText->setProperty("passwd", true);		//密码显示
	editText->setProperty("valuetype", 1);		//输入限制类型 /DEC_0/HEX_1/STR_2
	editText->setProperty("shift", "R4");		//左移还是右移
	editText->setProperty("showzerohead", true);//移位之后是否需要补0位
	editText->setProperty("endian", 1);			//输入限制类型 /L_0 小端/B_1 大端
	editText->setProperty("edited", true);		//是否可以编辑

	editText->setText(strTextEdit + QString::number(idx));
	editText->setGeometry(drop.x(), drop.y(), editText->width(), editText->height());
	editText->show();
}

void EditShowWidget::DropCreateCheckBox(QPoint drop)
{
	ACheckBox* checkBox = new ACheckBox(this);
	//自动命名
	int idx = XmlCreateCheckBox(checkBox);
	//设置新属性
	checkBox->setProperty("name", (strCheckBox.toStdString() + std::to_string(idx)).c_str());
	checkBox->setProperty("addr", 0);	//从偏移地址0x0开始
	checkBox->setProperty("len", 0);	//占用长度
	checkBox->setProperty("default", 0);//默认偏移地址
	checkBox->setProperty("ubuff", true);
	checkBox->setProperty("wrect", QRect(drop.x(), drop.y(), checkBox->width(), checkBox->height()));
	checkBox->setProperty("value", 1);		//16进制显示value进制数
	checkBox->setProperty("mask", 1);		//需要遮挡的位数
	checkBox->setProperty("edited", true);	//是否可以编辑

	checkBox->setText(strCheckBox + QString::number(idx));
	checkBox->setGeometry(drop.x(), drop.y(), checkBox->width(), checkBox->height());
	checkBox->show();
}

void EditShowWidget::DropCreateComboBox(QPoint drop)
{
	AComboBox* comboBox = new AComboBox(this);
	//自动命名
	int idx = XmlCreateComboBox(comboBox);
	//设置新属性
	comboBox->setProperty("name", (strComboBox.toStdString() + std::to_string(idx)).c_str());
	comboBox->setProperty("addr", 0);	//从偏移地址0x0开始
	comboBox->setProperty("len", 0);	//占用长度
	comboBox->setProperty("default", 0);//默认偏移地址
	comboBox->setProperty("ubuff", true);
	comboBox->setProperty("wrect", QRect(drop.x(), drop.y(), comboBox->width(), comboBox->height()));
	comboBox->setProperty("mask", 1);		//需要遮挡的位数
	comboBox->setProperty("edited", true);	//是否可以编辑

	comboBox->setGeometry(drop.x(), drop.y(), comboBox->width(), comboBox->height());
	comboBox->show();
}

void EditShowWidget::DropCreateTabWidget(QPoint drop)
{
	ATabWidget* tabWgt = new ATabWidget(this);
	//自动命名
	int idx = XmlCreateTabWidget(tabWgt);
	//设置新属性
	tabWgt->setProperty("name", (strTabWidget.toStdString() + std::to_string(idx)).c_str());
	tabWgt->setProperty("wrect", QRect(drop.x(), drop.y(), tabWgt->width(), tabWgt->height()));

	tabWgt->setGeometry(drop.x(), drop.y(), tabWgt->width(), tabWgt->height());
	tabWgt->show();
}

void EditShowWidget::CheckCover(QPoint ePos)
{
	bool hasParent = false;
	//多个group一起需要考虑
	QMap<QPoint,int> saveGroup;
	for (int i = 0; i < m_vecGroup.size(); ++i)
	{
		if (strTabWidget == m_curSelect->property("type"))
			break;

		if(m_vecGroup[i] == nullptr)
			continue;

		//鼠标相对于整个显示器的位置
		QPoint mouseRelativePos = cursor().pos() - mapToGlobal(this->pos());
		//Group控件相对于整个显示器的位置
		QPoint ctrlRelativePos = m_vecGroup[i]->mapToGlobal(this->pos()) - this->mapToGlobal(this->pos());

		if ((ePos.x() < ctrlRelativePos.x() + m_vecGroup[i]->width()
			&& ePos.x() >= ctrlRelativePos.x()
			&& ePos.y() < ctrlRelativePos.y() + m_vecGroup[i]->height()
			&& ePos.y() >= ctrlRelativePos.y()))
		{
			int ctlX = m_curPos.x() + ePos.x() - m_startPos.x() - m_vecGroup[i]->pos().x();
			int ctlY = m_curPos.y() + ePos.y() - m_startPos.y() - m_vecGroup[i]->pos().y();

			if(m_curSelect == m_vecGroup[i])
				continue;

			saveGroup[ctrlRelativePos - ePos] = i;
		}
	}
	//设置Group父类
	{
		QPoint minPos;
		QMap<QPoint, int>::iterator iter = saveGroup.begin();
		minPos = iter.key();
		int tureLength = INT_MAX;
		while (iter != saveGroup.end())
		{
			int temp = iter.key().manhattanLength();
			if (temp < tureLength)
			{
				tureLength = qMin(temp, tureLength);
				minPos = iter.key();
			}

			++iter;
		}

		if (saveGroup.size() != 0)
		{
			int idx = saveGroup[minPos];

			if (m_vecGroup.size() > idx)
			{
				QPoint newPoint = m_curPos + ePos - m_startPos - (m_vecGroup[idx]->mapToGlobal(this->pos()) - this->mapToGlobal(this->pos()));
				SetCtrlNewGroupParent(idx, newPoint.x(), newPoint.y());
				hasParent = true;
			}
		}
	}

	if (hasParent)
		return;

	QMap<QPoint, int> saveTab;
	for (int i = 0; i < m_vecTabWidget.size(); ++i)
	{
		if (m_vecTabWidget[i] == nullptr)
			continue;

		//鼠标相对于整个显示器的位置
		QPoint mouseRelativePos = cursor().pos();
		//Group控件相对于整个显示器的位置
		QPoint ctrlRelativePos = m_vecTabWidget[i]->mapToGlobal(this->pos()) - mapToGlobal(this->pos()) - m_vecTabWidget[i]->tabBar()->pos();

		if ((ePos.x() < ctrlRelativePos.x() + m_vecTabWidget[i]->width()
			&& ePos.x() >= ctrlRelativePos.x()
			&& ePos.y() < ctrlRelativePos.y() + m_vecTabWidget[i]->height()
			&& ePos.y() >= ctrlRelativePos.y()))
		{
			int ctlX = m_curPos.x() + ePos.x() - m_startPos.x() - m_vecTabWidget[i]->pos().x();
			int ctlY = m_curPos.y() + ePos.y() - m_startPos.y() - m_vecTabWidget[i]->pos().y() - m_vecTabWidget[i]->tabBar()->height();

			if(nullptr == m_vecTabWidget[i]->currentWidget())
				break;

			if (m_curSelect == m_vecTabWidget[i])
				continue;

			saveTab[ctrlRelativePos - ePos] = i;
		}
	}

	//设置Tab父类
	{
		QPoint minPos;
		QMap<QPoint, int>::iterator iter = saveTab.begin();
		minPos = iter.key();
		int tureLength = INT_MAX;
		while (iter != saveTab.end())
		{
			int temp = iter.key().manhattanLength();
			if (temp < tureLength)
			{
				tureLength = qMin(temp, tureLength);
				minPos = iter.key();
			}

			++iter;
		}

		if (saveTab.size() != 0)
		{
			int idx = saveTab[minPos];

			if (m_vecTabWidget.size() > idx)
			{
				QPoint newPoint = m_curPos + ePos - m_startPos - (m_vecTabWidget[idx]->mapToGlobal(this->pos()) - this->mapToGlobal(this->pos()));
				SetCtrlNewTabWidgetParent(idx, newPoint.x(), newPoint.y() - m_vecTabWidget[idx]->tabBar()->height());
			}
		}
	}

}

void EditShowWidget::SetCtrlNewGroupParent(int idx, int posX, int posY)
{
	if (strGroup == m_curSelect->property("type"))
	{
		AGroupBox* agb = qobject_cast<AGroupBox*>(m_curSelect);
		if (agb == m_vecGroup[idx])
			return;

		agb->setParent(m_vecGroup[idx]);
		agb->disconnect(agb, &AGroupBox::sgnGroupBox, this, &EditShowWidget::OnSlotGroupBox);
		agb->setGeometry(posX, posY, agb->width(), agb->height());
		agb->setProperty("wrect", QRect(posX, posY, agb->width(), agb->height()));
		agb->show();
		return;
	}
	else if (strLabel == m_curSelect->property("type"))
	{
		ALabel* aLabel = qobject_cast<ALabel*>(m_curSelect);
		aLabel->setParent(m_vecGroup[idx]);
		aLabel->disconnect(aLabel, &ALabel::sgnLabel, this, &EditShowWidget::OnSlotLabel);
		aLabel->setGeometry(posX, posY, aLabel->width(), aLabel->height());
		aLabel->setProperty("wrect", QRect(posX, posY, aLabel->width(), aLabel->height()));
		aLabel->show();
		return;
	}
	else if (strTextEdit == m_curSelect->property("type"))
	{
		AEditText* editText = qobject_cast<AEditText*>(m_curSelect);
		editText->setParent(m_vecGroup[idx]);
		editText->disconnect(editText, &AEditText::sgnEditText, this, &EditShowWidget::OnSlotEditText);
		editText->setGeometry(posX, posY, editText->width(), editText->height());
		editText->setProperty("wrect", QRect(posX, posY, editText->width(), editText->height()));
		editText->show();
		return;
	}
	else if (strCheckBox == m_curSelect->property("type"))
	{
		ACheckBox* checkBox = qobject_cast<ACheckBox*>(m_curSelect);
		checkBox->setParent(m_vecGroup[idx]);
		checkBox->disconnect(checkBox, &ACheckBox::sgnCheckBox, this, &EditShowWidget::OnSlotCheckBox);
		checkBox->setGeometry(posX, posY, checkBox->width(), checkBox->height());
		checkBox->setProperty("wrect", QRect(posX, posY, checkBox->width(), checkBox->height()));
		checkBox->show();
		checkBox->setEnabled(true);
		return;
	}
	else if (strComboBox == m_curSelect->property("type"))
	{
		AComboBox* comboBox = qobject_cast<AComboBox*>(m_curSelect);
		comboBox->setParent(m_vecGroup[idx]);
		comboBox->disconnect(comboBox, &AComboBox::sgnComboBox, this, &EditShowWidget::OnSlotComboBox);
		comboBox->setGeometry(posX, posY, comboBox->width(), comboBox->height());
		comboBox->setProperty("wrect", QRect(posX, posY, comboBox->width(), comboBox->height()));
		comboBox->show();
		return;
	}
}

void EditShowWidget::SetCtrlNewTabWidgetParent(int idx, int posX, int posY)
{
	if (strGroup == m_curSelect->property("type"))
	{
		AGroupBox* agb = qobject_cast<AGroupBox*>(m_curSelect);
		agb->setParent(m_vecTabWidget[idx]->currentWidget());
		agb->disconnect(agb, &AGroupBox::sgnGroupBox, this, &EditShowWidget::OnSlotGroupBox);
		agb->setGeometry(posX, posY, agb->width(), agb->height());
		agb->setProperty("wrect", QRect(posX, posY, agb->width(), agb->height()));
		agb->show();
		return;
	}
	else if (strLabel == m_curSelect->property("type"))
	{
		ALabel* aLabel = qobject_cast<ALabel*>(m_curSelect);
		aLabel->setParent(m_vecTabWidget[idx]->currentWidget());
		aLabel->disconnect(aLabel, &ALabel::sgnLabel, this, &EditShowWidget::OnSlotLabel);
		aLabel->setGeometry(posX, posY, aLabel->width(), aLabel->height());
		aLabel->setProperty("wrect", QRect(posX, posY, aLabel->width(), aLabel->height()));
		aLabel->show();
		return;
	}
	else if (strTextEdit == m_curSelect->property("type"))
	{
		AEditText* editText = qobject_cast<AEditText*>(m_curSelect);
		editText->setParent(m_vecTabWidget[idx]->currentWidget());
		editText->disconnect(editText, &AEditText::sgnEditText, this, &EditShowWidget::OnSlotEditText);
		editText->setGeometry(posX, posY, editText->width(), editText->height());
		editText->setProperty("wrect", QRect(posX, posY, editText->width(), editText->height()));
		editText->show();
		return;
	}
	else if (strCheckBox == m_curSelect->property("type"))
	{
		ACheckBox* checkBox = qobject_cast<ACheckBox*>(m_curSelect);
		checkBox->setParent(m_vecTabWidget[idx]->currentWidget());
		checkBox->disconnect(checkBox, &ACheckBox::sgnCheckBox, this, &EditShowWidget::OnSlotCheckBox);
		checkBox->setGeometry(posX, posY, checkBox->width(), checkBox->height());
		checkBox->setProperty("wrect", QRect(posX, posY, checkBox->width(), checkBox->height()));
		checkBox->show();
		return;
	}
	else if (strComboBox == m_curSelect->property("type"))
	{
		AComboBox* comboBox = qobject_cast<AComboBox*>(m_curSelect);
		comboBox->setParent(m_vecTabWidget[idx]->currentWidget());
		comboBox->disconnect(comboBox, &AComboBox::sgnComboBox, this, &EditShowWidget::OnSlotComboBox);
		comboBox->setGeometry(posX, posY, comboBox->width(), comboBox->height());
		comboBox->setProperty("wrect", QRect(posX, posY, comboBox->width(), comboBox->height()));
		comboBox->show();
		return;
	}
	else if (strTabWidget == m_curSelect->property("type"))
	{
		ATabWidget* aTabWgt = qobject_cast<ATabWidget*>(m_curSelect);
		if (aTabWgt == m_vecTabWidget[idx])
			return;

		aTabWgt->setParent(m_vecTabWidget[idx]->currentWidget());
		aTabWgt->disconnect(aTabWgt, &ATabWidget::sgnTabWidget, this, &EditShowWidget::OnSlotComboBox);
		aTabWgt->setGeometry(posX, posY, aTabWgt->width(), aTabWgt->height());
		aTabWgt->setProperty("wrect", QRect(posX, posY, aTabWgt->width(), aTabWgt->height()));
		aTabWgt->show();
		return;
	}
}

void EditShowWidget::ClearChildList(QObjectList _objList)
{
	for (auto obj : _objList)
	{
		if (strGroup == obj->property("type"))
		{
			std::vector<AGroupBox*>::iterator p = std::find(m_vecGroup.begin(), m_vecGroup.end(), qobject_cast<AGroupBox*>(obj));
			if (p != m_vecGroup.end())
			{
				//删除自己
				qobject_cast<AGroupBox*>(obj)->close();
				qobject_cast<AGroupBox*>(obj)->deleteLater();
				*p = nullptr;
				m_vecGroup.erase(p);
			}
		}
		else if (strLabel == obj->property("type"))
		{
			std::vector<ALabel*>::iterator p = std::find(m_vecLabel.begin(), m_vecLabel.end(), qobject_cast<ALabel*>(obj));
			if (p != m_vecLabel.end())
			{
				//删除自己
				qobject_cast<ALabel*>(obj)->close();
				qobject_cast<ALabel*>(obj)->deleteLater();
				*p = nullptr;
				m_vecLabel.erase(p);
			}
		}
		else if (strTextEdit == obj->property("type"))
		{
			std::vector<AEditText*>::iterator p = std::find(m_vecEditText.begin(), m_vecEditText.end(), qobject_cast<AEditText*>(obj));
			if (p != m_vecEditText.end())
			{
				//删除自己
				qobject_cast<AEditText*>(obj)->close();
				qobject_cast<AEditText*>(obj)->deleteLater();
				*p = nullptr;
				m_vecEditText.erase(p);
			}
		}
		else if (strCheckBox == obj->property("type"))
		{
			std::vector<ACheckBox*>::iterator p = std::find(m_vecCheckBox.begin(), m_vecCheckBox.end(), qobject_cast<ACheckBox*>(obj));
			if (p != m_vecCheckBox.end())
			{
				//删除自己
				qobject_cast<ACheckBox*>(obj)->close();
				qobject_cast<ACheckBox*>(obj)->deleteLater();
				*p = nullptr;
				m_vecCheckBox.erase(p);
			}
		}
		else if (strComboBox == obj->property("type"))
		{
			std::vector<AComboBox*>::iterator p = std::find(m_vecComboBox.begin(), m_vecComboBox.end(), qobject_cast<AComboBox*>(obj));
			if (p != m_vecComboBox.end())
			{
				//删除自己
				qobject_cast<AComboBox*>(obj)->close();
				qobject_cast<AComboBox*>(obj)->deleteLater();
				*p = nullptr;
				m_vecComboBox.erase(p);
			}
		}
		else if (strTabWidget == obj->property("type"))
		{
			std::vector<ATabWidget*>::iterator p = std::find(m_vecTabWidget.begin(), m_vecTabWidget.end(), qobject_cast<ATabWidget*>(obj));
			if (p != m_vecTabWidget.end())
			{
				//删除自己
				qobject_cast<ATabWidget*>(obj)->close();
				qobject_cast<ATabWidget*>(obj)->deleteLater();
				*p = nullptr;
				m_vecTabWidget.erase(p);
			}
		}
	}
	//ClearAllVec();
}

int EditShowWidget::XmlCreateGroupBox(AGroupBox* agb)
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

	connect(agb, &AGroupBox::sgnGroupBox, this, &EditShowWidget::OnSlotGroupBox);
	connect(agb, &AGroupBox::sgnShowPropetry, this, &EditShowWidget::sgnShowPropetry);
	connect(agb, &AGroupBox::sgnRClickGroupBox, this, &EditShowWidget::OnSlotRClick);

	agb->setProperty("index", idx);
	agb->setProperty("type", strGroup);

	return idx;
}

int EditShowWidget::XmlCreateLabel(ALabel* aLabel)
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

	connect(aLabel, &ALabel::sgnLabel, this, &EditShowWidget::OnSlotLabel);
	connect(aLabel, &ALabel::sgnShowPropetry, this, &EditShowWidget::sgnShowPropetry);
	connect(aLabel, &ALabel::sgnRClickLabel, this, &EditShowWidget::OnSlotRClick);

	aLabel->setProperty("index", idx);
	aLabel->setProperty("type", strLabel);

	return idx;
}

int EditShowWidget::XmlCreateEditText(AEditText* aEditText)
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

	connect(aEditText, &AEditText::sgnEditText, this, &EditShowWidget::OnSlotEditText);
	connect(aEditText, &AEditText::sgnShowPropetry, this, &EditShowWidget::sgnShowPropetry);
	connect(aEditText, &AEditText::sgnRClickEdit, this, &EditShowWidget::OnSlotRClick);

	aEditText->setProperty("index", idx);
	aEditText->setProperty("type", strTextEdit);

	return idx;
}

int EditShowWidget::XmlCreateCheckBox(ACheckBox* ackBox)
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

	connect(ackBox, &ACheckBox::sgnCheckBox, this, &EditShowWidget::OnSlotCheckBox);
	connect(ackBox, &ACheckBox::sgnShowPropetry, this, &EditShowWidget::sgnShowPropetry);
	connect(ackBox, &ACheckBox::sgnRClickCheck, this, &EditShowWidget::OnSlotRClick);

	ackBox->setProperty("index", idx);
	ackBox->setProperty("type", strCheckBox);

	return idx;
}

int EditShowWidget::XmlCreateComboBox(AComboBox* acbBox)
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

	connect(acbBox, &AComboBox::sgnComboBox, this, &EditShowWidget::OnSlotComboBox);
	connect(acbBox, &AComboBox::sgnShowPropetry, this, &EditShowWidget::sgnShowPropetry);
	connect(acbBox, &AComboBox::sgnRClickComboBox, this, &EditShowWidget::OnSlotRClick);

	acbBox->setProperty("index", idx);
	acbBox->setProperty("type", strComboBox);
	return idx;
}

int EditShowWidget::XmlCreateTabWidget(ATabWidget* tabWgt)
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
	connect(tabWgt, &ATabWidget::sgnTabWidget, this, &EditShowWidget::OnSlotTabWidget);
	connect(tabWgt, &ATabWidget::sgnShowPropetry, this, &EditShowWidget::sgnShowPropetry);
	connect(tabWgt, &ATabWidget::sgnRClickTabWidget, this, &EditShowWidget::OnSlotRClick);

	tabWgt->setProperty("index", idx);
	tabWgt->setProperty("type", strTabWidget);

	return idx;
}

ATabWidget* EditShowWidget::IsSubTabWgt(QString strName)
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

int EditShowWidget::EditWgtChildren()
{
	return m_vecGroup.size() || m_vecLabel.size() || m_vecEditText.size() 
		|| m_vecCheckBox.size() || m_vecComboBox.size() || m_vecTabWidget.size();
}

void EditShowWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(strGroup)
		|| event->mimeData()->hasFormat(strLabel)
		|| event->mimeData()->hasFormat(strTextEdit)
		|| event->mimeData()->hasFormat(strCheckBox)
		|| event->mimeData()->hasFormat(strComboBox)
		|| event->mimeData()->hasFormat(strTabWidget))
	{
		qDebug() << QString::fromLocal8Bit("成功完成拖拽到显示区");
		event->acceptProposedAction();
	}

	QWidget::dragEnterEvent(event);
}

void EditShowWidget::dropEvent(QDropEvent* event)
{
	qDebug() << QString::fromLocal8Bit("接收到了拖拽事件 %1，创建控件...").arg(event->mimeData()->text());

	QString dropText = event->mimeData()->text();

	if (dropText == strGroup)
	{
		DropCreateGroupBox(event->pos());
	}
	else if (dropText == strLabel)
	{
		DropCreateLabel(event->pos());
	}
	else if (dropText == strTextEdit)
	{
		DropCreateEditText(event->pos());
	}
	else if (dropText == strCheckBox)
	{
		DropCreateCheckBox(event->pos());
	}
	else if (dropText == strComboBox)
	{
		DropCreateComboBox(event->pos());
	}
	else if (dropText == strTabWidget)
	{
		DropCreateTabWidget(event->pos());
	}

	QWidget::dropEvent(event);
}

void EditShowWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() == Qt::LeftButton)
	{
		if (m_curSelect != nullptr)
		{
			int moveX = m_curPos.x() + event->x() - m_startPos.x();
			int moveY = m_curPos.y() + event->y() - m_startPos.y();
			//qDebug() << QString::fromLocal8Bit("mouseMoveEvent -- moveX = %1, moveY = %2").arg(moveX).arg(moveY);
			if (strGroup == m_curSelect->property("type"))
			{
				AGroupBox* agb = qobject_cast<AGroupBox*>(m_curSelect);

				if (agb->GetCursorPos() != CENTER)
					return;

				agb->move(moveX, moveY);
			}
			else if (strLabel == m_curSelect->property("type"))
			{
				ALabel* aLabel = qobject_cast<ALabel*>(m_curSelect);

				if (aLabel->GetCursorPos() != CENTER)
					return;

				aLabel->move(moveX, moveY);
			}
			else if (strTextEdit == m_curSelect->property("type"))
			{
				AEditText* editText = qobject_cast<AEditText*>(m_curSelect);

				if (editText->GetCursorPos() != CENTER)
					return;

				editText->move(moveX, moveY);
			}
			else if (strCheckBox == m_curSelect->property("type"))
			{
				ACheckBox* checkBox = qobject_cast<ACheckBox*>(m_curSelect);

				if (checkBox->GetCursorPos() != CENTER)
					return;

				checkBox->move(moveX, moveY);
			}
			else if (strComboBox == m_curSelect->property("type"))
			{
				AComboBox* comboBox = qobject_cast<AComboBox*>(m_curSelect);

				if (comboBox->GetCursorPos() != CENTER)
					return;

				comboBox->move(moveX, moveY);
			}
			else if (strTabWidget == m_curSelect->property("type"))
			{
				ATabWidget* tabWgt = qobject_cast<ATabWidget*>(m_curSelect);

				if (tabWgt->GetCursorPos() != CENTER)
					return;

				tabWgt->move(moveX, moveY);
			}

			QRect tempRect = m_curSelect->property("wrect").toRect();
			m_curSelect->setProperty("wrect", QRect(moveX, moveY, tempRect.width(), tempRect.height()));
		}

		//update();
	}

	QWidget::mouseMoveEvent(event);
}

void EditShowWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (m_curSelect != nullptr)
	{
		QRect tempRect = m_curSelect->property("wrect").toRect();
		int moveX = tempRect.x();
		int moveY = tempRect.y();
		if (strGroup == m_curSelect->property("type"))
		{
			AGroupBox* agb = qobject_cast<AGroupBox*>(m_curSelect);
			
			agb->setProperty("wrect", QRect(moveX, moveY, agb->width(), agb->height()));
			agb->activateWindow();
			agb->raise();
		}
		else if (strLabel == m_curSelect->property("type"))
		{
			ALabel* aLabel = qobject_cast<ALabel*>(m_curSelect);
			aLabel->setProperty("wrect", QRect(moveX, moveY, aLabel->width(), aLabel->height()));
			aLabel->activateWindow();
			aLabel->raise();
		}
		else if (strTextEdit == m_curSelect->property("type"))
		{
			AEditText* editLabel = qobject_cast<AEditText*>(m_curSelect);
			editLabel->setProperty("wrect", QRect(moveX, moveY, editLabel->width(), editLabel->height()));
			editLabel->activateWindow();
			editLabel->raise();
		}
		else if (strCheckBox == m_curSelect->property("type"))
		{
			ACheckBox* checkBox = qobject_cast<ACheckBox*>(m_curSelect);
			checkBox->setProperty("wrect", QRect(moveX, moveY, checkBox->width(), checkBox->height()));
			checkBox->activateWindow();
			checkBox->raise();
		}
		else if (strComboBox == m_curSelect->property("type"))
		{
			AComboBox* comboBox = qobject_cast<AComboBox*>(m_curSelect);
			comboBox->setProperty("wrect", QRect(moveX, moveY, comboBox->width(), comboBox->height()));
			comboBox->activateWindow();
			comboBox->raise();
		}
		else if (strTabWidget == m_curSelect->property("type"))
		{
			ATabWidget* tabWgt = qobject_cast<ATabWidget*>(m_curSelect);
			tabWgt->setProperty("wrect", QRect(moveX, moveY, tabWgt->width(), tabWgt->height()));
			tabWgt->activateWindow();
			tabWgt->raise();
		}

		CheckCover(event->pos());
		emit sgnShowPropetry(m_curSelect);
	}

	m_curSelect = nullptr;

	QWidget::mouseReleaseEvent(event);
}


void EditShowWidget::OnSlotGroupBox(QObject* group)
{
	m_curSelect = nullptr;

	qDebug() << QString::fromLocal8Bit("窗口group获取");

	//鼠标相对于展示区的位置
	m_startPos = cursor().pos() - mapToGlobal(this->pos());

	AGroupBox* agb = qobject_cast<AGroupBox*>(group);
	m_curPos = agb->pos();

	m_curSelect = group;
}

void EditShowWidget::OnSlotLabel(QObject* label)
{
	m_curSelect = nullptr;

	qDebug() << QString::fromLocal8Bit("窗口label获取");

	//鼠标相对于展示区的位置
	m_startPos = cursor().pos() - mapToGlobal(this->pos());

	ALabel* aLabel = qobject_cast<ALabel*>(label);
	m_curPos = aLabel->pos();

	m_curSelect = label;
}

void EditShowWidget::OnSlotEditText(QObject* edit)
{
	m_curSelect = nullptr;

	qDebug() << QString::fromLocal8Bit("窗口editText获取");

	//鼠标相对于展示区的位置
	m_startPos = cursor().pos() - mapToGlobal(this->pos());

	AEditText* editText = qobject_cast<AEditText*>(edit);
	m_curPos = editText->pos();

	m_curSelect = edit;
}

void EditShowWidget::OnSlotCheckBox(QObject* chbox)
{
	m_curSelect = nullptr;

	qDebug() << QString::fromLocal8Bit("窗口checkBox获取");

	//鼠标相对于展示区的位置
	m_startPos = cursor().pos() - mapToGlobal(this->pos());

	ACheckBox* checkBox = qobject_cast<ACheckBox*>(chbox);
	checkBox->show();
	m_curPos = checkBox->pos();

	m_curSelect = chbox;
}

void EditShowWidget::OnSlotComboBox(QObject* cbBox)
{
	m_curSelect = nullptr;

	qDebug() << QString::fromLocal8Bit("窗口comboBox获取");

	//鼠标相对于展示区的位置
	m_startPos = cursor().pos() - mapToGlobal(this->pos());

	AComboBox* comboBox = qobject_cast<AComboBox*>(cbBox);
	m_curPos = comboBox->pos();

	m_curSelect = cbBox;
}

void EditShowWidget::OnSlotTabWidget(QObject* tab)
{
	m_curSelect = nullptr;

	qDebug() << QString::fromLocal8Bit("窗口TabWidget获取");

	//鼠标相对于展示区的位置
	m_startPos = cursor().pos() - mapToGlobal(this->pos());

	ATabWidget* comboBox = qobject_cast<ATabWidget*>(tab);
	m_curPos = comboBox->pos();

	m_curSelect = tab;
}

template<class T>
void EditShowWidget::SetConnectSize(T* t, QAction* adjAction)
{
	connect(adjAction, &QAction::triggered, [=]()
		{
			connect(m_winSizeDlg, &SetWinSize::sgnWHSize, t, [&](int w, int h)
				{
					t->resize(QSize(w, h));
					QRect wrect = t->property("wrect").toRect();
					t->setProperty("wrect", QRect(wrect.x(), wrect.y(), t->width(), t->height()));
				});
			m_winSizeDlg->SetWHSize(t->width(), t->height());
			m_winSizeDlg->exec();
			m_winSizeDlg->disconnect();
		});

	//std::vector<T*>& temp = GetSubWinVec(t);
}

template<class T>
std::vector<T*>& EditShowWidget::GetSubWinVec(T* tt)
{
	if (std::is_same_v<decltype(tt), AGroupBox*>)
	{
		return m_vecGroup;
	}
	else if (std::is_same_v<decltype(tt), ALabel*>)
	{
		return m_vecLabel;
	}
	else if (std::is_same_v<decltype(tt), AEditText*>)
	{
		return m_vecEditText;
	}
	else if (std::is_same_v<decltype(tt), ACheckBox*>)
	{
		return m_vecCheckBox;
	}
	else if (std::is_same_v<decltype(tt), AComboBox*>)
	{
		return m_vecComboBox;
	}
	else if (std::is_same_v<decltype(tt), ATabWidget*>)
	{
		return m_vecTabWidget;
	}
	return nullptr;
}

void EditShowWidget::GroupRClick(AGroupBox* agb, QMenu* qmenu, QAction* adj, QAction* del)
{
	SetConnectSize(agb, adj);

	connect(del, &QAction::triggered, [=]()
		{
			std::vector<AGroupBox*>::iterator p = std::find(m_vecGroup.begin(), m_vecGroup.end(), agb);
			if (p != m_vecGroup.end())
			{
				//删除子窗口
				QObjectList objList = agb->children();
				ClearChildList(objList);
				//删除自己
				agb->close();
				agb->deleteLater();
				*p = nullptr;
				m_vecGroup.erase(p);
			}

		});
}

void EditShowWidget::LabelRClick(ALabel* alabel, QMenu* qmenu, QAction* adj, QAction* del)
{
	SetConnectSize(alabel, adj);

	connect(del, &QAction::triggered, [=]()
		{
			std::vector<ALabel*>::iterator p = std::find(m_vecLabel.begin(), m_vecLabel.end(), alabel);
			if (p != m_vecLabel.end())
			{
				//删除子窗口
				QObjectList objList = alabel->children();
				ClearChildList(objList);
				//删除自己
				alabel->close();
				alabel->deleteLater();
				*p = nullptr;
				m_vecLabel.erase(p);
			}
		});
}

void EditShowWidget::EditTextRClick(AEditText* aEdit, QMenu* qmenu, QAction* adj, QAction* del)
{
	SetConnectSize(aEdit, adj);

	connect(del, &QAction::triggered, [=]()
		{
			std::vector<AEditText*>::iterator p = std::find(m_vecEditText.begin(), m_vecEditText.end(), aEdit);
			if (p != m_vecEditText.end())
			{
				//删除子窗口
				QObjectList objList = aEdit->children();
				ClearChildList(objList);
				//删除自己
				aEdit->close();
				aEdit->deleteLater();
				*p = nullptr;
				m_vecEditText.erase(p);
			}
		});
}

void EditShowWidget::CheckBoxRClick(ACheckBox* aChkBox, QMenu* qmenu, QAction* adj, QAction* del)
{
	SetConnectSize(aChkBox, adj);

	connect(del, &QAction::triggered, [=]()
		{
			std::vector<ACheckBox*>::iterator p = std::find(m_vecCheckBox.begin(), m_vecCheckBox.end(), aChkBox);
			if (p != m_vecCheckBox.end())
			{
				//删除子窗口
				QObjectList objList = aChkBox->children();
				ClearChildList(objList);
				//删除自己
				aChkBox->close();
				aChkBox->deleteLater();
				*p = nullptr;
				m_vecCheckBox.erase(p);
			}
		});
}

void EditShowWidget::ComboBoxRClick(AComboBox* aCbBox, QMenu* qmenu, QAction* adj, QAction* del, QAction* addEle, QAction* delEle)
{
	SetConnectSize(aCbBox, adj);

	connect(del, &QAction::triggered, [=]()
		{
			std::vector<AComboBox*>::iterator p = std::find(m_vecComboBox.begin(), m_vecComboBox.end(), aCbBox);
			if (p != m_vecComboBox.end())
			{
				//删除子窗口
				QObjectList objList = aCbBox->children();
				ClearChildList(objList);
				//删除自己
				aCbBox->close();
				aCbBox->deleteLater();
				*p = nullptr;
				m_vecComboBox.erase(p);
			}
		});

	connect(addEle, &QAction::triggered, [=]()
		{
			QString name = QInputDialog::getText(this, tr("Insert"), tr("Enter your new item name"), QLineEdit::Normal);

			if (name.isEmpty())
				return;

			aCbBox->addItem(name);
			aCbBox->setProperty(name.toStdString().c_str(), "c1");
		});

	connect(delEle, &QAction::triggered, [=]()
		{
			if(aCbBox->count() > 0)
				aCbBox->removeItem(aCbBox->count() - 1);
		});

	qmenu->addAction(addEle);
	qmenu->addAction(delEle);
}

void EditShowWidget::TabWidgetRClick(ATabWidget* aTabWgt, QMenu* qmenu, QAction* adj, QAction* del, QAction* InsertAf, QAction* InsertBe, QAction* modifyAct)
{
	SetConnectSize(aTabWgt, adj);

	QMenu* subMenu = qmenu->addMenu(QStringLiteral("插入页"));
	subMenu->addAction(InsertBe);
	subMenu->addAction(InsertAf);

	if(aTabWgt->count() > 0)
		qmenu->addAction(modifyAct);

	connect(del, &QAction::triggered, [=]()
		{
			std::vector<ATabWidget*>::iterator p = std::find(m_vecTabWidget.begin(), m_vecTabWidget.end(), aTabWgt);
			if (p != m_vecTabWidget.end())
			{
				//删除子窗口
				QObjectList objList = aTabWgt->currentWidget()->children();
				ClearChildList(objList);
				//删除自己
				aTabWgt->close();
				aTabWgt->deleteLater();
				*p = nullptr;
				m_vecTabWidget.erase(p);
			}
		});

	connect(InsertBe, &QAction::triggered, [=]()
		{
			//输入tab页框
			QString name = QInputDialog::getText(this, tr("Rename"), tr("Enter your new tab name"), QLineEdit::Normal);

			if (name.isEmpty())
				return;

			QWidget* tab = new QWidget();
			tab->setObjectName(QString::fromUtf8("tab") + name);

			int idx = 0;
			if (aTabWgt->currentIndex() == 0 || aTabWgt->currentIndex() == -1)
			{
				idx = 0;
			}
			else if (aTabWgt->currentIndex() == 1)
			{
				idx = 1;
			}
			else
			{
				idx = aTabWgt->currentIndex() - 1;
			}

			aTabWgt->insertTab(idx, tab, name);
			aTabWgt->widget(idx)->setProperty("caption", name);
			aTabWgt->widget(idx)->setProperty("type", strTabWidget);
		});

	connect(InsertAf, &QAction::triggered, [=]()
		{
			//输入tab页框
			QString name = QInputDialog::getText(this, tr("Rename"), tr("Enter your new tab name"), QLineEdit::Normal);

			if (name.isEmpty())
				return;

			QWidget* tab = new QWidget();
			tab->setObjectName(QString::fromUtf8("tab") + name);

			int idx = aTabWgt->currentIndex() + 1;

			aTabWgt->insertTab(idx, tab, name);
			aTabWgt->widget(idx)->setProperty("caption", name);
			aTabWgt->widget(idx)->setProperty("type", strTabWidget);
		});

	connect(modifyAct, &QAction::triggered, [=]()
		{
			//输入tab页框
			QString name = QInputDialog::getText(this, tr("Rename"), tr("Enter your new tab name"), QLineEdit::Normal);

			if (name.isEmpty())
				return;

			QWidget* tab = new QWidget();
			tab->setObjectName(QString::fromUtf8("tab") + name);

			int idx = aTabWgt->currentIndex() + 1;

			aTabWgt->setTabText(aTabWgt->currentIndex(), name);
			aTabWgt->widget(aTabWgt->currentIndex())->setProperty("caption", name);
		});
}

void EditShowWidget::OnSlotRClick(QObject* agbObj)
{
	QMenu* menu;
	QAction* adjAction = new QAction(QStringLiteral("宽高调整"), agbObj);
	QAction* delAction = new QAction(QStringLiteral("删除"), agbObj);
	QAction* InsertBeforeAction = new QAction(QStringLiteral("当前页之前"));
	QAction* InsertAfterAction = new QAction(QStringLiteral("当前页之后"));
	QAction* ModifyTabAction = new QAction(QStringLiteral("修改tab页名称"));
	QAction* addElement = new QAction(QStringLiteral("添加元素"));
	QAction* delElement = new QAction(QStringLiteral("删除元素"));

	if (strGroup == agbObj->property("type"))
	{
		AGroupBox* agb = qobject_cast<AGroupBox*>(agbObj);
		menu = new QMenu(agb);
		GroupRClick(agb, menu, adjAction, delAction);
	}
	else if (strLabel == agbObj->property("type"))
	{
		ALabel* alabel = qobject_cast<ALabel*>(agbObj);
		menu = new QMenu(alabel);
		LabelRClick(alabel, menu, adjAction, delAction);
	}
	else if (strTextEdit == agbObj->property("type"))
	{
		AEditText* aEditText = qobject_cast<AEditText*>(agbObj);
		menu = new QMenu(aEditText);
		EditTextRClick(aEditText, menu, adjAction, delAction);
	}
	else if (strCheckBox == agbObj->property("type"))
	{
		ACheckBox* aChkBox = qobject_cast<ACheckBox*>(agbObj);
		menu = new QMenu(aChkBox);
		CheckBoxRClick(aChkBox, menu, adjAction, delAction);
	}
	else if (strComboBox == agbObj->property("type"))
	{
		AComboBox* aCbBox = qobject_cast<AComboBox*>(agbObj);
		menu = new QMenu(aCbBox);
		ComboBoxRClick(aCbBox, menu, adjAction, delAction, addElement, delElement);
	}
	else if (strTabWidget == agbObj->property("type"))
	{
		ATabWidget* aTabWgt = qobject_cast<ATabWidget*>(agbObj);
		menu = new QMenu(aTabWgt);
		TabWidgetRClick(aTabWgt, menu, adjAction, delAction, InsertAfterAction, InsertBeforeAction, ModifyTabAction);
	}

	menu->addAction(adjAction);
	menu->addAction(delAction);
	if(menu != nullptr)
		menu->exec(QCursor::pos());

	emit sgnClearPropetry(agbObj);
}
