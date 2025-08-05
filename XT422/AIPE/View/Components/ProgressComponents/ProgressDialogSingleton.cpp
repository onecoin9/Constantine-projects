#include "ProgressDialogSingleton.h"
#include "../View/GlobalInit/StyleInit.h"
#include "AngKGlobalInstance.h"
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
ProgressDialogSingleton::ProgressDialogSingleton(QWidget* parent)
	: QObject(parent)
	, progressDialog(nullptr)
	, processBar(nullptr)
	, m_label(nullptr)
	, m_button(nullptr)
	, m_MainWgt(parent)
{
}

ProgressDialogSingleton::~ProgressDialogSingleton()
{
	if (progressDialog)
	{
		delete progressDialog;
		progressDialog = nullptr;
	}
}


void ProgressDialogSingleton::showProgressDialog(int max, const QString& labelText, const QString& titleText, DLG_SHOW_MODE mode)
{
	progressDialog = new QProgressDialog(m_MainWgt);

	progressDialog->setWindowTitle(titleText);
	progressDialog->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);


	progressDialog->setFixedSize(700, 100);
	//progressDialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint);
	//progressDialog->setWindowModality(Qt::WindowModal);
	progressDialog->setMinimumDuration(0); // 立即显示进度条

	connect(this, &ProgressDialogSingleton::progressChanged, progressDialog, [=](int nValue) {
		if (progressDialog) {
			progressDialog->setValue(nValue);

			//if (nValue >= 100) {
			//	closeProgressDialog();
			//}
		}
		});

	connect(progressDialog, &QProgressDialog::canceled, this, &ProgressDialogSingleton::canceled);

	processBar = new QProgressBar(progressDialog);
	processBar->setFixedSize(progressDialog->width() - 50, 10);
	processBar->setRange(0, max);
	processBar->setMaximum(max);
	processBar->setValue(0);
	processBar->setAlignment(Qt::AlignCenter);
	m_label = new QLabel(progressDialog);
	m_button = new QPushButton(tr("Cancel"), progressDialog);
	progressDialog->setCancelButton(m_button);
	QObject::connect(progressDialog, &QProgressDialog::canceled, [=]() {
		//ALOG_INFO("Progress canceled...", "CU");
		});

	QString styleFile ;
	switch ((ViewMode)AngKGlobalInstance::ReadValue("Skin", "mode").toInt())
	{
	case ViewMode::Light:
		styleFile = ":/Skin/Light/AngKProgressDialog.qss";
		break;
	case ViewMode::Dark:
		styleFile = ":/Skin/Dark/AngKProgressDialog.qss";
		break;
	default:
		break;
	}

	QFile f(styleFile);
	f.open(QFile::ReadOnly); 
	if (f.isOpen()) {
		QString styleStr = f.readAll();
		progressDialog->setStyleSheet(styleStr);
		processBar->setStyleSheet(styleStr);
		m_label->setStyleSheet(styleStr);
		f.close();
	}


	progressDialog->setBar(processBar);
	m_label->setText(labelText);
	m_label->setAlignment(Qt::AlignCenter);
	progressDialog->setLabel(m_label);
	if (mode == DLG_SHOW) {
		progressDialog->show();
		QApplication::processEvents();
	}
	else {
		progressDialog->exec();
	}
}

void ProgressDialogSingleton::updateProgress(int value)
{
	emit progressChanged(value);
	//processBar->setValue(value);
}

void ProgressDialogSingleton::closeProgressDialog()
{
	if (progressDialog) {
		progressDialog->setValue(progressDialog->maximum()); // 完成时将进度设为最大值
		progressDialog->close();
		progressDialog->deleteLater();
		progressDialog = nullptr;
	}
}

bool ProgressDialogSingleton::IsCancel()
{
	if (progressDialog)
		return progressDialog->wasCanceled();
	
	return false;
}

void ProgressDialogSingleton::onSlotCLoseProgressDialog()
{
	closeProgressDialog();
}