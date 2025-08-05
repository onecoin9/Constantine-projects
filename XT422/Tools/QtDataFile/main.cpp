#include "QtDataFile.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtDataFile w;
    w.show();
    return a.exec();
}
