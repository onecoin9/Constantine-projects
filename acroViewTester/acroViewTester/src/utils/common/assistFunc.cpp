#include "acroViewTester.h"
void acroViewTester::loadStyle(const QString& qssFile)
{
	QFile file(qssFile);
	if (file.open(QFile::ReadOnly)) {
		QString qss = QLatin1String(file.readAll());
		setStyleSheet(qss);
		file.close();
	}
}

QString acroViewTester::getLocalIPAddress()
{
	QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
	//for (const QNetworkInterface& iface : interfaces) 
	//{
	//    if (iface.flags() & QNetworkInterface::IsUp &&
	//        !(iface.flags() & QNetworkInterface::IsLoopBack)) {

	//        QList<QNetworkAddressEntry> entries = iface.allAddresses();
	//        for (const QNetworkAddressEntry& entry : entries) {
	//            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
	//                return entry.ip().toString();
	//            }
	//        }
	//    }
	//}
	return "N/A";
}

QString acroViewTester::getCurrentVersion()
{
	int major = APP_VERSION_MAJOR;
	int minor = APP_VERSION_MINOR;
	int patch = APP_VERSION_PATCH;
	QString versionInfo = QString("%1.%2.%3")
		.arg(major)
		.arg(minor)
		.arg(patch);
	 qDebug().noquote() << QStringLiteral("当前版本: ") + versionInfo;

	//提示更新部分
	//bool isVersionNewer(int major, int minor, int patch) {
	//    return (major > APP_VERSION_MAJOR) ||
	//        (major == APP_VERSION_MAJOR && minor > APP_VERSION_MINOR) ||
	//        (major == APP_VERSION_MAJOR && minor == APP_VERSION_MINOR && patch >= APP_VERSION_PATCH);
	//}
	//if (isVersionNewer(1, 0, 2)) {
	//    // 提示更新
	//}
	return versionInfo;

}

void TestNullPointer() {
	int* p = nullptr;
	*p = 100;  // 这里会崩溃并生成dump
}

void TestArrayOverflow() {
	int arr[5] = { 1, 2, 3, 4, 5 };
	for (int i = 0; i <= 5; i++) {
		arr[i] = 0;  // 当i=5时越界访问
	}
}
void TestDivideByZero() {
	int a = 100;
	int b = 0;
	int c = a / b;  // 除零错误
}

void TestStackOverflow() {
	TestStackOverflow();  // 无限递归导致栈溢出
}

void acroViewTester::setupTestButtons()
{
	
	connect(ui.btnNull, &QPushButton::clicked, this, &acroViewTester::onTestNullPointer);
	connect(ui.btnArray, &QPushButton::clicked, this, &acroViewTester::onTestArrayOverflow);
	connect(ui.btnDiv, &QPushButton::clicked, this, &acroViewTester::onTestDivideByZero);
	connect(ui.btnStack, &QPushButton::clicked, this, &acroViewTester::onTestStackOverflow);

}
void acroViewTester::onTestNullPointer()
{
	TestNullPointer();
}

void acroViewTester::onTestArrayOverflow()
{
	TestArrayOverflow();
}

void acroViewTester::onTestDivideByZero()
{
	TestDivideByZero();
}

void acroViewTester::onTestStackOverflow()
{
	TestStackOverflow();
}