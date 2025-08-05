#include "QtTcpServerTest.h"

#include <QPushButton>
#include <QTextEdit>
#include <QByteArray>
#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QUdpSocket> 

#define APP_VERSION  ("TCPJsonRPC Client V1.2")
QtTcpServerTest::QtTcpServerTest(QWidget *parent)
	: QMainWindow(parent)
	, socket(nullptr)
	, usocket(nullptr)
	, m_globalSettings(Q_NULLPTR)
{
	ui.setupUi(this);
	InitSetting();
	TestLink();
	setWindowTitle(QString(APP_VERSION));
	ui.socketComboBox->addItem("TCP", TCP_Type);
	ui.socketComboBox->addItem("UDP", UDP_Type);
	ui.textEdit->setFont(QFont("宋体", 12));
	ui.receiver->setFont(QFont("宋体", 12));

	m_strCurOpenFilePath = settingReadValue("AppConfig", "OpenFilePath").toString();
}

void QtTcpServerTest::TestLink()
{
	socket = new QTcpSocket(this);
	usocket = new QUdpSocket(this);

	connect(ui.connectButton, &QPushButton::clicked, this, [=]() {

		socket->abort();
		QString ipAddress = settingReadValue("IPAddress", "ip").toString();
		int nPort = settingReadValue("Port", "port").toInt();
		//连接服务器
		if(ui.socketComboBox->currentData().toInt() == TCP_Type)
			socket->connectToHost(ipAddress, nPort);
		else if(ui.socketComboBox->currentData().toInt() == UDP_Type)
			usocket->connectToHost(ipAddress, nPort);
	});

	connect(ui.cleanDataButton, &QPushButton::clicked, this, [=]() {
		ui.receiver->setText("");
		ui.textEdit->setText("");
	
	});

	connect(ui.sendButton, &QPushButton::clicked, this, [=]() {
		ui.receiver->setText("");///点击发送的时候先清空对应的数据
		if (ui.textEdit->toPlainText().isEmpty())
		{
			QMessageBox::critical(this, "critical", QStringLiteral("不允许发送空数据！"),
				QMessageBox::Yes);
			return;
		}

		//将输入框的内容写入socket缓冲区
		QByteArray writeBuf; 
		QDataStream DataStream(&writeBuf, QIODevice::ReadWrite);
		DataStream.setByteOrder(QDataStream::LittleEndian);

		uint32_t strJsonLen = (uint32_t)ui.textEdit->toPlainText().size() + 1;
		DataStream << strJsonLen;
		DataStream.writeRawData(ui.textEdit->toPlainText().toStdString().c_str(), strJsonLen);

		qDebug() << writeBuf;
		if (ui.socketComboBox->currentData().toInt() == TCP_Type)
		{
			socket->write(writeBuf);
			socket->flush();
		}
		else if (ui.socketComboBox->currentData().toInt() == UDP_Type)
		{
			usocket->write(writeBuf);
			//刷新socket缓冲区
			usocket->flush();
		}

	});

	connect(socket, SIGNAL(connected()), this, SLOT(connected_SLOT()));
	connect(usocket, SIGNAL(connected()), this, SLOT(connected_udp_SLOT()));

	connect(ui.openButton, &QPushButton::clicked, this, &QtTcpServerTest::openFile_SLOT);
	connect(ui.saveButton, &QPushButton::clicked, this, &QtTcpServerTest::saveFile_SLOT);
}

void QtTcpServerTest::InitSetting()
{
	QString strAppPath = QCoreApplication::applicationDirPath();
	QSettings::setDefaultFormat(QSettings::IniFormat);
	bool bCreate = true;
	m_globalSettings = new QSettings(localGlobalSettingFile(bCreate), QSettings::IniFormat);
	m_globalSettings->setIniCodec("utf-8");

	if (!bCreate)
	{
		settingWriteValue("IPAddress", "ip", "127.0.0.1");
		settingWriteValue("Port", "port", 1030);
		settingWriteValue("AppConfig", "OpenFilePath", strAppPath);
	}
}

QString QtTcpServerTest::localGlobalSettingFile(bool& isCreate)
{
	QString settingPath = QCoreApplication::applicationDirPath() + "/Localcfg.ini";
	if (!QFileInfo::exists(settingPath))
	{
		QFile pfile(settingPath);
		pfile.open(QIODevice::WriteOnly | QIODevice::Text);

		isCreate = false;

		pfile.close();
	}

	return settingPath;
}

QVariant QtTcpServerTest::settingReadValue(QString strGroup, QString strValue)
{
	return m_globalSettings->value(strGroup + "/" + strValue);
}

void QtTcpServerTest::settingWriteValue(QString strGroup, QString strName, QVariant strValue)
{
	m_globalSettings->beginGroup(strGroup);
	m_globalSettings->setValue(strName, strValue);
	m_globalSettings->endGroup();
}

void QtTcpServerTest::readyRead_SLOT()
{
	QString buffer;
	QByteArray bytearray;
	qDebug() << "Client Received!";
	//读取缓冲区数据
	if (ui.socketComboBox->currentData().toInt() == TCP_Type)
	{
		bytearray = socket->readAll();
	}
	else if (ui.socketComboBox->currentData().toInt() == UDP_Type)
	{
		bytearray = usocket->readAll();
	}

	const char* strJson = bytearray.constData() + 4;
	buffer = QString(strJson);

	if (!buffer.isEmpty())
	{
		qDebug() << bytearray.data();
		//刷新显示
		ui.receiver->setPlainText(buffer);
	}
}

void QtTcpServerTest::connected_SLOT()
{
	connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead_SLOT()));
	ui.connectInfoTip->setText(settingReadValue("IPAddress", "ip").toString() + ":" + settingReadValue("Port", "port").toString() + QStringLiteral(" TCP连接成功!"));
}

void QtTcpServerTest::connected_udp_SLOT()
{
	connect(usocket, SIGNAL(readyRead()), this, SLOT(readyRead_SLOT()));
	ui.connectInfoTip->setText(settingReadValue("IPAddress", "ip").toString() + ":" + settingReadValue("Port", "port").toString() + QStringLiteral(" UDP连接成功!"));

}

void QtTcpServerTest::openFile_SLOT()
{
	QString filePath = QFileDialog::getOpenFileName(this, "Select As...",m_strCurOpenFilePath, tr("json Files(*.json);; All Files(*.*)"));
	QFileInfo fi = QFileInfo(filePath);
	if (filePath.isEmpty()) {
		return;
	}
	///读取并显示文件内容//
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		
		QMessageBox::critical(this, "critical", QStringLiteral("文件打开失败!"),
			QMessageBox::Yes);
	}
	else
	{
		m_strCurOpenFilePath = fi.absolutePath();
		settingWriteValue("AppConfig", "OpenFilePath", m_strCurOpenFilePath);
		QTextStream readStream(&file);
		readStream.setCodec("UTF-8");//设置文件流编码方式
		ui.textEdit->setText("");
		while (!readStream.atEnd())
		{
			ui.textEdit->append(readStream.readLine());
		}

		ui.connectInfoTip->setText(filePath + QStringLiteral(" 读取成功!"));
	}
	
	file.close();
}

void QtTcpServerTest::saveFile_SLOT()
{
	QString filePath = QFileDialog::getSaveFileName(this, "Save As...", m_strCurOpenFilePath, tr("All Files(*.*)"));
	QFileInfo fi = QFileInfo(filePath);
	if (filePath.isEmpty()) {
		return;
	}
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		QMessageBox::critical(this, "critical", QStringLiteral("文件保存失败！"),
			QMessageBox::Yes);
	}
	else
	{
		//method 1
		m_strCurOpenFilePath = fi.absolutePath();
		settingWriteValue("AppConfig", "OpenFilePath", m_strCurOpenFilePath);
		QTextStream stream(&file);
		stream << ui.textEdit->toPlainText();
		stream.flush();

		//method 2
		//QString arr = ui.textEdit->toPlainText();
		//file.write(arr.toUtf8());

		ui.connectInfoTip->setText(filePath + QStringLiteral(" 保存成功!"));
	}

	file.close();
}
