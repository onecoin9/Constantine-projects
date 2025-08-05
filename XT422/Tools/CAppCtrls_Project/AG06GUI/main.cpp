#include "AG06GUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	AG06GUI w;
	w.show();
	return a.exec();
}
