#include "DriverXmlParseTool.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	DriverXmlParseTool w;
	w.show();
	return a.exec();
}
