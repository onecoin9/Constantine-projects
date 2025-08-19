#pragma once

#include <QObject>
#include <QProgressDialog>

class ProgressDialogSingleton : public QObject
{
	Q_OBJECT

public:
	enum DLG_SHOW_MODE
	{
		DLG_SHOW = 0,
		DLG_EXEC
	};
	//static ProgressDialogSingleton& getInstance()
	//{
	//	static ProgressDialogSingleton instance; // 单例对象
	//	return instance;
	//}

	void showProgressDialog(int max, const QString& labelText = "Processing...", const QString& titleText = "Aprog", DLG_SHOW_MODE mode = DLG_SHOW);

	void updateProgress(int value);

	void closeProgressDialog();

	bool IsCancel();
signals:
	void progressChanged(int value);

	void canceled();

public slots:
	void onSlotCLoseProgressDialog();
public:
	ProgressDialogSingleton(QWidget* object = nullptr);
	~ProgressDialogSingleton();

private:
	QProgressBar* processBar;
	QProgressDialog* progressDialog;
	QWidget* m_MainWgt;
	QLabel* m_label;
	QPushButton* m_button;
};
