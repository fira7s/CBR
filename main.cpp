#include "CBR.h"
#include <QtWidgets/QApplication>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QLatin1String>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CBR w;
	//open qss file
    QFile file("Combinear.qss");
    file.open(QFile::ReadOnly);

    // Read qss file contents into a QString using QLatin1String
    QString styleSheet{ QLatin1String(file.readAll()) };

	//setup stylesheet
	a.setStyleSheet(styleSheet);
    w.show();
    return a.exec();
}
