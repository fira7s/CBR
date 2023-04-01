#include "CBR.h"
#include <QtWidgets/QApplication>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QLatin1String>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CBR w;
    QFile file("Combinear.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet{ QLatin1String(file.readAll()) };
	a.setStyleSheet(styleSheet);
    w.show();
    return a.exec();
}
