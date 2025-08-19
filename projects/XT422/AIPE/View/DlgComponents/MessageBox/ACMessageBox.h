#pragma once

#include <QDialog>
#include <QMessageBox>
#include "AngKDialog.h"
namespace Ui { class ACMessageBox; };

class ACMessageBox : public AngKDialog
{
	Q_OBJECT

public:
	enum class ACMsgButton {
		MSG_NO_BTN = 0,
		MSG_OK_BTN = 1,
		MSG_CANCEL_BTN = 2,
		MSG_OK_CANCEL_BTN = 3
	};

	enum class ACMsgType {
		MSG_CANCEL = -1,
		MSG_OK = 0
	};

	explicit ACMessageBox(QWidget* parent = Q_NULLPTR,
						  const QString& title = tr("Tip"),
						  const QString& text = "",
						  ACMsgButton defaultButton = ACMsgButton::MSG_CANCEL_BTN);

	~ACMessageBox();

	void InitButton();

	void ShowButton(ACMsgButton defaultButton);

	static ACMsgType showInformation(QWidget* parent,
									 const QString& title, 
									 const QString& text, 
									 ACMsgButton buttons = ACMsgButton::MSG_OK_BTN);
														
	static ACMsgType showWarning(QWidget* parent,
								 const QString& title,
								 const QString& text,
								 ACMsgButton buttons = ACMsgButton::MSG_OK_BTN);

	static ACMsgType showError(QWidget* parent,
							   const QString& title,
							   const QString& text,
							   ACMsgButton buttons = ACMsgButton::MSG_OK_BTN);

private:
	void setLabelProperty(const char* name, const QVariant& value);

public slots:
	void onSlotClickOKButton();

	void onSlotClickCancelButton();

private:
	Ui::ACMessageBox *ui;
	ACMsgType m_clickButton;
};
