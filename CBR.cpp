#include "CBR.h"
#include <archive.h>
#include <archive_entry.h>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include "ArchiveExtraction.h"
#include <opencv2/opencv.hpp>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QPoint>
#include <QEvent>
#include <QTimer>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QDirIterator>




CBR::CBR(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(ui.extractButton, &QPushButton::clicked, this, &CBR::extractArchive);
    connect(ui.suivantButton, &QPushButton::clicked, this, &CBR::PageSuivante);
    connect(ui.precedantButton, &QPushButton::clicked, this, &CBR::PagePrecedante);
    connect(ui.createButton, &QPushButton::clicked, this, &CBR::createArchive);




}

CBR::~CBR()
{

}


void CBR::extractArchive()
{
    /*ArchiveExtraction a;
    a.LireArchive("data/ex2.cbr");
    a.DecompresserArchive(0, "data/ex2.cbr");
    QGraphicsScene* scene = new QGraphicsScene(this);
    ui.graphicsView->setScene(scene);
    QGraphicsTextItem* textItem = new QGraphicsTextItem(QString::number(a.GetNombreTotalePage()));
    textItem->setPos(50, 50);
    scene->addItem(textItem);*/
    v.set_current_archvie("data/ex3.zip");
    v.set_page_number(0);
    QGraphicsScene* scene = new QGraphicsScene(this);
    ui.graphicsView->setScene(scene);
    ArchiveExtraction a("data/ex3.zip");
    a.LireArchive("data/ex3.zip");
    cv::Mat image;
    image = a.ChargerImage(0);
    QImage qimage(image.data,image.cols,image.rows,image.step,QImage::Format_BGR888);
    QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(qimage));
    scene->addItem(pixmapItem);
    ui.graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
    ui.graphicsView->setRenderHint(QPainter::Antialiasing);
    ui.graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);

    ui.graphicsView->setDragMode(QGraphicsView::ScrollHandDrag); // enable scroll hand drag

    ui.graphicsView->setMouseTracking(true);

    ui.graphicsView->viewport()->installEventFilter(this);
}


void CBR::PageSuivante()
{


    QGraphicsScene* scene = new QGraphicsScene(this);
    ui.graphicsView->setScene(scene);
    ArchiveExtraction a(v.get_current_archvie());
    v.set_page_number(v.get_page_Number() + 1);
    a.LireArchive(v.get_current_archvie());
    cv::Mat image;
    image = a.ChargerImage(v.get_page_Number());
    QImage qimage(image.data, image.cols, image.rows, image.step, QImage::Format_BGR888);
    QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(qimage));
    scene->addItem(pixmapItem);
    ui.graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);


    ui.graphicsView->setRenderHint(QPainter::Antialiasing);
    ui.graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);

    ui.graphicsView->setDragMode(QGraphicsView::ScrollHandDrag); // enable scroll hand drag

    ui.graphicsView->setMouseTracking(true);

    ui.graphicsView->viewport()->installEventFilter(this);


}


void CBR::PagePrecedante()
{

    QGraphicsScene* scene = new QGraphicsScene(this);
    ui.graphicsView->setScene(scene);
    ArchiveExtraction a(v.get_current_archvie());
    v.set_page_number(v.get_page_Number() - 1);
    a.LireArchive(v.get_current_archvie());
    cv::Mat image;
    image = a.ChargerImage(v.get_page_Number());
    QImage qimage(image.data, image.cols, image.rows, image.step, QImage::Format_BGR888);
    QGraphicsPixmapItem * pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(qimage));
    scene->addItem(pixmapItem);
    ui.graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);


    ui.graphicsView->setRenderHint(QPainter::Antialiasing);
    ui.graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);

    ui.graphicsView->setDragMode(QGraphicsView::ScrollHandDrag); // enable scroll hand drag

    ui.graphicsView->setMouseTracking(true);

    ui.graphicsView->viewport()->installEventFilter(this);


}

void CBR::onViewDoubleClicked(QMouseEvent* event)
{
    // zoom in on the image
    if (event->button() == Qt::LeftButton)
    {
        ui.graphicsView->scale(1.8,1.8);
    }
    else if (event->button() == Qt::RightButton)
    {
        ui.graphicsView->scale(1 / 1.8, 1 / 1.8);
    }
}


bool CBR::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        onViewDoubleClicked(dynamic_cast<QMouseEvent*>(event));
        return true;
    }
    return QObject::eventFilter(obj, event);
}

void CBR::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        // start dragging
        m_lastMousePos = event->pos();
        m_dragging = true;
    }
}

void CBR::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging)
    {
        // calculate the offset and move the view
        QPoint offset = event->pos() - m_lastMousePos;
        ui.graphicsView->setSceneRect(ui.graphicsView->sceneRect().translated(-offset.x(), -offset.y()));
        m_lastMousePos = event->pos();
    }
}

void CBR::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        // stop dragging
        m_dragging = false;
    }
}



void CBR::createArchive()
{
    QString archiveName = QFileDialog::getSaveFileName(this, "Save archive", QDir::homePath(), "ZIP files (*.zip)");

    if (archiveName.isEmpty()) {
        return;
    }

    struct archive* a;
    struct archive_entry* entry;
    int len;
    char buffer[8192];

    QFile file(archiveName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Error", "Cannot create archive");
        return;
    }

    a = archive_write_new();
    archive_write_set_format_zip(a);
    archive_write_open_file(a, file.fileName().toStdString().c_str());

    QStringList images = QFileDialog::getOpenFileNames(this, "Select images to archive", QDir::homePath(), "Images (*.jpg *.jpeg *.png *.gif)");
    for (const auto& image : images) {
        QFile file(image);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "Error", "Cannot open file " + file.fileName());
            continue;
        }

        QFileInfo fileInfo(image);
        entry = archive_entry_new();
        archive_entry_set_pathname(entry, fileInfo.fileName().toStdString().c_str()); // Set only the file name as the path
        archive_entry_set_size(entry, file.size());
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);

        archive_write_header(a, entry);

        while ((len = file.read(buffer, sizeof(buffer))) > 0) {
            archive_write_data(a, buffer, len);
        }

        archive_entry_free(entry);
        file.close();
    }

    archive_write_close(a);
    archive_write_free(a);

    QMessageBox::information(this, "Archive created", "The archive was successfully created.");
}
