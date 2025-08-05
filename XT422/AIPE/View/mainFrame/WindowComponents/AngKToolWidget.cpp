#include "AngKToolWidget.h"
#include "ui_AngKToolWidget.h"
#include "../View/GlobalInit/StyleInit.h"
#include "AngKPathResolve.h"
#include "AngKGlobalInstance.h"
#include <QButtonGroup>
#include <QMouseEvent>
AngKToolWidget::AngKToolWidget(QWidget *parent)
	: QWidget(parent)
	, m_bDark(false)
	, m_buttonGroup(nullptr)
{
	ui = new Ui::AngKToolWidget();
	ui->setupUi(this);

	this->setAttribute(Qt::WA_Hover, true);//开启悬停事件

	InitButton();

	this->setObjectName("AngKToolWidget");
	QT_SET_STYLE_SHEET(objectName());
}

AngKToolWidget::~AngKToolWidget()
{
	delete ui;
}

void AngKToolWidget::InitButton()
{
	m_buttonGroup = new QButtonGroup(this);

	m_nLastId = -1;
	connect(m_buttonGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [&](int index) {
		m_buttonGroup->button(index);
		QList<QAbstractButton*> buttonLists = m_buttonGroup->buttons();

		if (index == m_nLastId)
		{
			// exclusive：此属性保存按钮组是否是互斥的，如果此属性为真，则在任何给定时间只能选中按钮组中的一个按钮。
			// 用户可以单击任何按钮来选中它，该按钮将替换现有的按钮作为选中的按钮。
			// 在一个排他组中，用户不能通过点击取消当前选中的按钮；相反，必须单击该组中的另一个按钮来设置该组的新选中按钮。默认情况下，此属性为true。
			m_buttonGroup->setExclusive(false); // 先取消互斥
			m_buttonGroup->button(index)->setChecked(false);
			m_buttonGroup->setExclusive(true); // 再设置互斥

			m_nLastId = -1; // 取消之后，重置标志位
		}
		else
		{
			m_nLastId = index; // 保存最后一次点击的标志位，用于下次判断是否点击同一个按钮
		}

		for (int i = 0; i < buttonLists.size(); ++i)
		{
			ButtonRecoverState(qobject_cast<QToolButton*>(buttonLists[i]));
		}
	});

	m_buttonGroup->addButton(ui->defineToolButton, (int)OperationTagType::Self);
	m_buttonGroup->addButton(ui->eraseToolButton, (int)OperationTagType::Erase);
	m_buttonGroup->addButton(ui->blankToolButton, (int)OperationTagType::Blank);
	m_buttonGroup->addButton(ui->programToolButton, (int)OperationTagType::Program);
	m_buttonGroup->addButton(ui->verifyToolButton, (int)OperationTagType::Verify);
	m_buttonGroup->addButton(ui->readToolButton, (int)OperationTagType::Read);
	m_buttonGroup->addButton(ui->secureToolButton, (int)OperationTagType::Secure);
	m_buttonGroup->addButton(ui->illegalToolButton, 8);
	m_buttonGroup->addButton(ui->protectToolButton, 9);
	m_buttonGroup->addButton(ui->bufferToolButton, SP_BufferToolButton);
	m_buttonGroup->addButton(ui->chipSelectToolButton, SP_ChipSelectToolButton);

	ui->defineToolButton->setText(tr("Self\nDefine"));
	ui->eraseToolButton->setText(tr("Erase"));
	ui->blankToolButton->setText(tr("Blank"));
	ui->programToolButton->setText(tr("Program"));
	ui->verifyToolButton->setText(tr("Verify"));
	ui->readToolButton->setText(tr("Read"));
	ui->secureToolButton->setText(tr("Secure"));
	ui->illegalToolButton->setText(tr("Illegal"));
	ui->protectToolButton->setText(tr("Protect"));
	ui->bufferToolButton->setText(tr("Buffer"));
	ui->chipSelectToolButton->setText(tr("Chip"));

	//安装事件过滤器
	ui->defineToolButton->installEventFilter(this);
	ui->eraseToolButton->installEventFilter(this);
	ui->blankToolButton->installEventFilter(this);
	ui->programToolButton->installEventFilter(this);
	ui->verifyToolButton->installEventFilter(this);
	ui->readToolButton->installEventFilter(this);
	ui->secureToolButton->installEventFilter(this);
	ui->illegalToolButton->installEventFilter(this);
	ui->protectToolButton->installEventFilter(this);
	ui->bufferToolButton->installEventFilter(this);
	ui->chipSelectToolButton->installEventFilter(this);

	//设置文本颜色
	m_bDark = AngKGlobalInstance::ReadValue("Skin", "mode").toInt() == (int)ViewMode::Dark ? true : false;
	QPalette palette;
	QColor skinColor;
	m_bDark ? skinColor = QColor(227, 227, 227) : skinColor = QColor(125, 125, 125);
	palette.setColor(QPalette::ButtonText, skinColor);
	ui->defineToolButton->setPalette(palette);
	ui->eraseToolButton->setPalette(palette);
	ui->blankToolButton->setPalette(palette);
	ui->programToolButton->setPalette(palette);
	ui->verifyToolButton->setPalette(palette);
	ui->readToolButton->setPalette(palette);
	ui->secureToolButton->setPalette(palette);
	ui->illegalToolButton->setPalette(palette);
	ui->protectToolButton->setPalette(palette);
	ui->bufferToolButton->setPalette(palette);
	ui->chipSelectToolButton->setPalette(palette);

	connect(ui->defineToolButton, &QToolButton::clicked, this, &AngKToolWidget::onSlotClick_defineToolButton);
	connect(ui->eraseToolButton, &QToolButton::clicked, this, &AngKToolWidget::onSlotClick_eraseToolButton);
	connect(ui->blankToolButton, &QToolButton::clicked, this, &AngKToolWidget::onSlotClick_blankToolButton);
	connect(ui->programToolButton, &QToolButton::clicked, this, &AngKToolWidget::onSlotClick_programToolButton);
	connect(ui->verifyToolButton, &QToolButton::clicked, this, &AngKToolWidget::onSlotClick_verifyToolButton);
	connect(ui->readToolButton, &QToolButton::clicked, this, &AngKToolWidget::onSlotClick_readToolButton);
	connect(ui->secureToolButton, &QToolButton::clicked, this, &AngKToolWidget::onSlotClick_secureToolButton);
	connect(ui->illegalToolButton, &QToolButton::clicked, this, &AngKToolWidget::onSlotClick_illegalToolButton);
	connect(ui->protectToolButton, &QToolButton::clicked, this, &AngKToolWidget::onSlotClick_protectToolButton);
	connect(ui->bufferToolButton, &QToolButton::clicked, this, &AngKToolWidget::onSlotClick_bufferToolButton);
	connect(ui->chipSelectToolButton, &QToolButton::clicked, this, &AngKToolWidget::sgnOpenSelectChipDlg);

	ui->illegalToolButton->hide();
	ui->protectToolButton->hide();
}

void AngKToolWidget::ShowButtonFromJson(BaseOper baseInfo)
{
	ui->eraseToolButton->setHidden(!baseInfo.bErase);
	ui->blankToolButton->setHidden(!baseInfo.bBlank);
	ui->programToolButton->setHidden(!baseInfo.bProg);
	ui->verifyToolButton->setHidden(!baseInfo.bVerify);
	ui->readToolButton->setHidden(!baseInfo.bRead);
	ui->secureToolButton->setHidden(!baseInfo.bSecure);
}

int AngKToolWidget::GetSelectOperType()
{
	return m_nLastId;
}

void AngKToolWidget::SetButtonEnable(OperationTagType nType)
{
	m_buttonGroup->blockSignals(true); // 阻止发送信号
	m_buttonGroup->buttonClicked((int)nType);
	switch (nType)
	{
	case OperationTagType::None:
		break;
	case OperationTagType::Erase:
		break;
	case OperationTagType::Blank:
		break;
	case OperationTagType::Program:
		break;
	case OperationTagType::Verify:
		break;
	case OperationTagType::Secure:
		break;
	case OperationTagType::Read:
		break;
	case OperationTagType::Self:
		break;
	case OperationTagType::CheckSum:
		break;
	default:
		break;
	}
	m_buttonGroup->blockSignals(false); // 恢复发送信号
}

bool AngKToolWidget::eventFilter(QObject* watched, QEvent* event)
{
	QToolButton* toolBtn = qobject_cast<QToolButton*>(watched);
	QMouseEvent* _mouse = dynamic_cast<QMouseEvent*>(event);

	if (event->type() == QEvent::HoverEnter)
	{
		ButtonHoverEnterState(qobject_cast<QToolButton*>(watched));
	}
	else if (event->type() == QEvent::HoverLeave)
	{
		ButtonRecoverState(qobject_cast<QToolButton*>(watched));
	}
	else if (_mouse != nullptr && _mouse->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonPress)
	{
		ButtonPressState(qobject_cast<QToolButton*>(watched));
	}
	else if (_mouse != nullptr && _mouse->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonDblClick)
	{
		ButtonPressState(qobject_cast<QToolButton*>(watched));
	}
	//else if (_mouse->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonRelease)
	//{
	//	ButtonHoverEnterState(qobject_cast<QToolButton*>(watched));
	//}

	return QWidget::eventFilter(watched, event);
}

void AngKToolWidget::onSlotClick_defineToolButton(bool state)
{
	QToolButton* toolBtn = qobject_cast<QToolButton*>(sender());
}

void AngKToolWidget::onSlotClick_eraseToolButton(bool state)
{

}

void AngKToolWidget::onSlotClick_blankToolButton(bool state)
{
}

void AngKToolWidget::onSlotClick_programToolButton(bool state)
{
}

void AngKToolWidget::onSlotClick_verifyToolButton(bool state)
{
}

void AngKToolWidget::onSlotClick_readToolButton(bool state)
{
}

void AngKToolWidget::onSlotClick_secureToolButton(bool state)
{
}

void AngKToolWidget::onSlotClick_illegalToolButton(bool state)
{
}

void AngKToolWidget::onSlotClick_protectToolButton(bool state)
{

}

void AngKToolWidget::onSlotClick_bufferToolButton(bool state)
{
	emit sgnOpenBufferTool();
}

void AngKToolWidget::ButtonHoverEnterState(QToolButton* btn)
{
	if (btn->isChecked())
		return;

	QPalette palette = btn->palette();
	QColor skinColor;
	m_bDark ? skinColor = QColor(255, 255, 255) : skinColor = QColor(77, 77, 77);
	palette.setColor(QPalette::ButtonText, skinColor);
	btn->setPalette(palette);

	if (btn == ui->defineToolButton)
	{
		ui->defineToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/defineButtonHover.svg"));
	}
	else if (btn == ui->eraseToolButton)
	{
		ui->eraseToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/eraseButtonHover.svg"));
	}
	else if (btn == ui->blankToolButton)
	{
		ui->blankToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/blankButtonHover.svg"));
	}
	else if (btn == ui->programToolButton)
	{
		ui->programToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/programButtonHover.svg"));
	}
	else if (btn == ui->verifyToolButton)
	{
		ui->verifyToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/verifyButtonHover.svg"));
	}
	else if (btn == ui->readToolButton)
	{
		ui->readToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/readButtonHover.svg"));
	}
	else if (btn == ui->secureToolButton)
	{
		ui->secureToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/secureButtonHover.svg"));
	}
	else if (btn == ui->illegalToolButton)
	{
		ui->illegalToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/illegalButtonHover.svg"));
	}
	else if (btn == ui->protectToolButton)
	{
		ui->protectToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/protectButtonHover.svg"));
	}
	else if (btn == ui->bufferToolButton)
	{
		ui->bufferToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/bufferButtonHover.svg"));
	}
	else if (btn == ui->chipSelectToolButton)
	{
		ui->chipSelectToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/chipSelectHover.svg"));
	}
}

void AngKToolWidget::ButtonRecoverState(QToolButton* btn)
{
	if (btn->isChecked())
		return;

	QPalette palette = btn->palette();
	QColor skinColor;
	m_bDark ? skinColor = QColor(227, 227, 227) : skinColor = QColor(125, 125, 125);
	palette.setColor(QPalette::ButtonText, skinColor);
	btn->setPalette(palette);

	if (btn == ui->defineToolButton)
	{
		ui->defineToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/defineButtonNormal.svg"));
	}
	else if (btn == ui->eraseToolButton)
	{
		ui->eraseToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/eraseButtonNormal.svg"));
	}
	else if (btn == ui->blankToolButton)
	{
		ui->blankToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/blankButtonNormal.svg"));
	}
	else if (btn == ui->programToolButton)
	{
		ui->programToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/programButtonNormal.svg"));
	}
	else if (btn == ui->verifyToolButton)
	{
		ui->verifyToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/verifyButtonNormal.svg"));
	}
	else if (btn == ui->readToolButton)
	{
		ui->readToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/readButtonNormal.svg"));
	}
	else if (btn == ui->secureToolButton)
	{
		ui->secureToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/secureButtonNormal.svg"));
	}
	else if (btn == ui->illegalToolButton)
	{
		ui->illegalToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/illegalButtonNormal.svg"));
	}
	else if (btn == ui->protectToolButton)
	{
		ui->protectToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/protectButtonNormal.svg"));
	}
	else if (btn == ui->bufferToolButton)
	{
		ui->bufferToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/bufferButtonNormal.svg"));
	}
	else if (btn == ui->chipSelectToolButton)
	{
		ui->chipSelectToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/chipSelectNormal.svg"));
	}
}

void AngKToolWidget::ButtonPressState(QToolButton* btn)
{
	QPalette palette = btn->palette();
	palette.setColor(QPalette::ButtonText, QColor(230, 0, 18));
	btn->setPalette(palette);

	if (btn == ui->defineToolButton)
	{
		ui->defineToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/defineButtonPress.svg"));
	}
	else if (btn == ui->eraseToolButton)
	{
		ui->eraseToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/eraseButtonPress.svg"));
	}
	else if (btn == ui->blankToolButton)
	{
		ui->blankToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/blankButtonPress.svg"));
	}
	else if (btn == ui->programToolButton)
	{
		ui->programToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/programButtonPress.svg"));
	}
	else if (btn == ui->verifyToolButton)
	{
		ui->verifyToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/verifyButtonPress.svg"));
	}
	else if (btn == ui->readToolButton)
	{
		ui->readToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/readButtonPress.svg"));
	}
	else if (btn == ui->secureToolButton)
	{
		ui->secureToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/secureButtonPress.svg"));
	}
	else if (btn == ui->illegalToolButton)
	{
		ui->illegalToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/illegalButtonPress.svg"));
	}
	else if (btn == ui->protectToolButton)
	{
		ui->protectToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/protectButtonPress.svg"));
	}
	else if (btn == ui->bufferToolButton)
	{
		ui->bufferToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/bufferButtonPress.svg"));
	}
	else if (btn == ui->chipSelectToolButton)
	{
		ui->chipSelectToolButton->setIcon(QIcon(Utils::AngKPathResolve::localRelativeSkinPath() + "PushButton/chipSelectPress.svg"));
	}
}
