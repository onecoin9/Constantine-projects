#include "QtTcpServerTest.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QtTcpServerTest w;
	w.show();
	return a.exec();
}
