#include "AngKShowLogArea.h"
#include "ui_AngKShowLogArea.h"
#include "../View/GlobalInit/StyleInit.h"
#include <QScrollBar>

AngKShowLogArea::AngKShowLogArea(QWidget *parent)
	: QWidget(parent)
	, m_nMinHeight(0)
{
	ui = new Ui::AngKShowLogArea();
	ui->setupUi(this);

	ui->logText->moveCursor(QTextCursor::End);
	ui->label->setText(tr("Log"));
	ui->logText->setReadOnly(true);
	ui->logText->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	connect(ui->expandCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(sgnLogAreaExpand(int)));
	m_nMinHeight = this->minimumHeight();
	ui->logText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	this->setObjectName("AngKShowLogArea");
	QT_SET_STYLE_SHEET(objectName());
}

AngKShowLogArea::~AngKShowLogArea()
{
	delete ui;
}

void AngKShowLogArea::setLogText(QString text)
{

	bool bFlag = false;
	QScrollBar* vScrollBar = ui->logText->verticalScrollBar();
	if (vScrollBar->value() == vScrollBar->maximum()) {
		bFlag = true;
	}

	QTextCursor cursor = ui->logText->textCursor();
	cursor.movePosition(QTextCursor::End);
	cursor.insertText(text);

	if(bFlag)
		vScrollBar->setValue(vScrollBar->maximum());
}

int AngKShowLogArea::GetUIMinHeight()
{
	return m_nMinHeight;
}

void AngKShowLogArea::SetExpandCheck(bool bCheck)
{
	ui->expandCheckBox->setChecked(bCheck);
}

bool AngKShowLogArea::GetExpandCheck()
{
	if (ui->expandCheckBox->isChecked())
	{
		return true;
	}

	return false;
}
