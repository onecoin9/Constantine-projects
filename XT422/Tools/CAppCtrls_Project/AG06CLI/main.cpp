#include <QtCore/QCoreApplication>
#include "CLITest.h"

int main(int argc, char *argv[])
{
	int Ret = 0;
	QCoreApplication a(argc, argv);
	CLITest();
	Ret = a.exec();

	return Ret;
}
