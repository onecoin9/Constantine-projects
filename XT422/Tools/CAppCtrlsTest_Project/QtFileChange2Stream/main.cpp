#include "QtFileChange2Stream.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QtFileChange2Stream w;
	w.show();
	return a.exec();
}
