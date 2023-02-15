#include "CBR.h"
#include <archive.h>
#include <archive_entry.h>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include "ArchiveExtraction.h"


CBR::CBR(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(ui.extractButton, &QPushButton::clicked, this, &CBR::extractArchive);
}

CBR::~CBR()
{

}


void CBR::extractArchive()
{
    ArchiveExtraction a;
    a.LireArchive("data/ex2.cbr");
    a.DecompresserArchive(0, "data/ex2.cbr");
    QGraphicsScene* scene = new QGraphicsScene(this);
    ui.graphicsView->setScene(scene);

    QGraphicsTextItem* textItem = new QGraphicsTextItem(QString::number(a.GetNombreTotalePage()));
    textItem->setPos(50, 50);
    scene->addItem(textItem);



}