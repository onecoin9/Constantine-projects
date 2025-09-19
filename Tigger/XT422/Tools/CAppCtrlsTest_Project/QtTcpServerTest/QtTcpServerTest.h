#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtTcpServerTest.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>

enum SocketType
{
	TCP_Type,
	UDP_Type
};

class QSettings;
class QtTcpServerTest : public QMainWindow
{
	Q_OBJECT

public:
	QtTcpServerTest(QWidget *parent = Q_NULLPTR);

	void TestLink();

	void InitSetting();

	QString localGlobalSettingFile(bool& isCreate);

private:
	QVariant settingReadValue(QString strGroup, QString strValue);

	void settingWriteValue(QString strGroup, QString strName, QVariant strValue);
public slots:
	void readyRead_SLOT();

	void connected_SLOT();

	void connected_udp_SLOT();

	void openFile_SLOT();

	void saveFile_SLOT();
private:
	Ui::QtTcpServerTestClass ui;
	QTcpSocket* socket;
	QUdpSocket* usocket;
	QSettings* m_globalSettings;
	QString m_strCurOpenFilePath;
};
