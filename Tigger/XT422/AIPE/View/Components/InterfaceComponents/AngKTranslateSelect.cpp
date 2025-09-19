#include "AngKTranslateSelect.h"
#include "ui_AngKTranslateSelect.h"
#include "StyleInit.h"
#include "GlobalDefine.h"
#include "AngKGlobalInstance.h"
#include "MessageType.h"
#include "ACMessageBox.h"
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QTranslator>
namespace AcroView
{
	AngKTranslateSelect::AngKTranslateSelect(QWidget* parent, bool bDark)
		: QWidget(parent)
		, m_curSelectLanguage((TranslateLanguage)AngKGlobalInstance::ReadValue("Language", "mode").toInt())
	{
		ui = new Ui::AngKTranslateSelect();
		ui->setupUi(this);

		connect(ui->enButton, &QPushButton::clicked, this, &AngKTranslateSelect::onSlotChangeCheckState);
		connect(ui->chButton, &QPushButton::clicked, this, &AngKTranslateSelect::onSlotChangeCheckState);
		connect(ui->jpButton, &QPushButton::clicked, this, &AngKTranslateSelect::onSlotChangeCheckState);
		
		if(!bDark)
			InitShadow();

		this->setObjectName("AngKTranslateSelect");
		QT_SET_STYLE_SHEET(objectName());
	}

	AngKTranslateSelect::~AngKTranslateSelect()
	{
		delete ui;
	}

	void AngKTranslateSelect::InitShadow()
	{
		QGraphicsDropShadowEffect* encheckShadow = new QGraphicsDropShadowEffect(this);
		encheckShadow->setOffset(0, 1);
		encheckShadow->setColor(QColor("#dfdfdf"));
		encheckShadow->setBlurRadius(4);

		QGraphicsDropShadowEffect* chcheckShadow = new QGraphicsDropShadowEffect(this);
		chcheckShadow->setOffset(0, 1);
		chcheckShadow->setColor(QColor("#dfdfdf"));
		chcheckShadow->setBlurRadius(4);

		QGraphicsDropShadowEffect* jpcheckShadow = new QGraphicsDropShadowEffect(this);
		jpcheckShadow->setOffset(0, 0);
		jpcheckShadow->setColor(QColor("#dfdfdf"));
		jpcheckShadow->setBlurRadius(4);

		ui->enButton->setGraphicsEffect(encheckShadow);
		ui->chButton->setGraphicsEffect(chcheckShadow);
		ui->jpButton->setGraphicsEffect(jpcheckShadow);
	}

	void AngKTranslateSelect::setCheckState(TranslateLanguage nType)
	{
		switch (nType)
		{
		case TranslateLanguage::English:
			ui->enButton->setChecked(true);
			ui->chButton->setChecked(false);
			ui->jpButton->setChecked(false);
			break;
		case TranslateLanguage::Chinese:
			ui->enButton->setChecked(false);
			ui->chButton->setChecked(true);
			ui->jpButton->setChecked(false);
			break;
		case TranslateLanguage::Janpanese:
			ui->chButton->setChecked(false);
			ui->enButton->setChecked(false);
			ui->jpButton->setChecked(true);
			break;
		default:
			break;
		}
	}

	void AngKTranslateSelect::onSlotChangeCheckState(bool bCheck)
	{
		QPushButton* clickButton = qobject_cast<QPushButton*>(sender());
		if (!bCheck)
		{
			clickButton->setChecked(!bCheck);
			return;
		}

		TranslateLanguage nType = TranslateLanguage::English;

		if (clickButton == ui->enButton)
		{
			nType = TranslateLanguage::English;
			ui->chButton->setChecked(false);
			ui->jpButton->setChecked(false);
		}
		else if (clickButton == ui->chButton)
		{
			nType = TranslateLanguage::Chinese;
			ui->enButton->setChecked(false);
			ui->jpButton->setChecked(false);
		}
		else if (clickButton == ui->jpButton)
		{
			nType = TranslateLanguage::Janpanese;
			ui->chButton->setChecked(false);
			ui->enButton->setChecked(false);
		}

		AngKGlobalInstance::WriteValue("Language", "mode", (int)nType);


		if (m_curSelectLanguage != nType) {
			ACMessageBox::ACMsgType ret = ACMessageBox::showWarning(this, tr("Warning"), tr("Switching translation requires restarting the client. Do you want to restart now ?"),
				ACMessageBox::ACMsgButton::MSG_OK_CANCEL_BTN);

			if (ret == ACMessageBox::ACMsgType::MSG_OK)
			{
				qApp->exit(MessageType::MESSAGE_RESTART);
			}
		}
	}
}