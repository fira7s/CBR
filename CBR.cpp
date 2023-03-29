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
#include <QDialog>
#include <QPushButton>
#include <QDirIterator>
#include <QDialog>
#include <QPushButton>
#include <QIcon>
#include <QWheelEvent>
#include <QPointF> 
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QListWidget>
#include<QInputDialog>
#include<QMessageBox>
#include "cache.h"



QCache<int, ImageData> cache;
QReadWriteLock cache_lock;
ArchiveExtraction current_Archive;
std::string g_archive_path;
int currentPage;



bool g_is_page_current_changed=false;
bool g_is_path_changed=false;
int g_page_num_total=191;
std::mutex g_preload_mutex;
bool preloaded=true;
int page_preload_left_size=10;
int page_preload_right_size=10;


CBR::CBR(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(ui.extractButton, &QPushButton::clicked, this, &CBR::extractArchive);
    connect(ui.suivantButton, &QPushButton::clicked, this, &CBR::PageSuivante);
    connect(ui.precedantButton, &QPushButton::clicked, this, &CBR::PagePrecedante);
    connect(ui.createButton, &QPushButton::clicked, this, &CBR::createArchive);
    connect(ui.SommaireButton, &QPushButton::clicked, this, &CBR::sommaire);
    connect(ui.SelectButton, &QPushButton::clicked, this, &CBR::loadImageFromZip);
    connect(ui.SaveButton, &QPushButton::clicked, this, &CBR::SaveImage);
    QAction* aProposAction = new QAction(tr("A propos"), this);
    menuBar()->addAction(aProposAction);
    connect(aProposAction, &QAction::triggered, this, &CBR::showAboutDialog);
    p = PreLoadWorker();
    g_archive_path = "data/ex3.zip";
}

CBR::~CBR()
{

}


void CBR::extractArchive()
{
    /*ArchiveExtraction a("data/ex3.zip");
    a.LireArchive();
    a.DecompresserArchive(0, "data/ex3.zip");
    QGraphicsScene* scene = new QGraphicsScene(this);
    ui.graphicsView->setScene(scene);

    // Print number of pages
    QGraphicsTextItem* textItem = new QGraphicsTextItem(QString::number(a.GetNombreTotalePage()));
    textItem->setPos(50, 50);
    scene->addItem(textItem);

    // Print filenames
    int y = 80; // Starting y position for filenames
    std::map<int, std::string> fileMap = a.GetListeFichier();
    for (auto const& file : fileMap)
    {
        QGraphicsTextItem* filenameItem = new QGraphicsTextItem(QString::fromStdString(file.second));
        filenameItem->setPos(50, y);    
        scene->addItem(filenameItem);
        y += 20; // Increase y position for next filename
    }*/
    QGraphicsScene* scene = new QGraphicsScene(this);
    ui.graphicsView->setScene(scene);
    current_Archive.setPath(g_archive_path);
    current_Archive.LireArchive();
    g_page_num_total = current_Archive.GetNombreTotalePage();
    currentPage = 0;
    g_preload_mutex.lock();
    p.loadAndCacheImage(currentPage);
    preloaded = false;
    g_preload_mutex.unlock();
    qDebug() << "ok4";
    cv::Mat image;
    image = *cache.object(0)->cv_image_ptr;
    qDebug() << "ok5";
    //image = a.ChargerImage(v.get_page_Number());
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
void CBR::sommaire()
{
    ArchiveExtraction a(g_archive_path);
    a.LireArchive();
    std::map<int, std::string> m_fileNames = a.GetListeFichier();
    std::string m_currentFile = m_fileNames[currentPage];

    // Create a new dialog window
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Sommaire"));
    dialog.setMinimumSize(400, 400);

    // Create a new list widget
    QListWidget* listWidget = new QListWidget(&dialog);
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection); // Allow only one item to be selected
    listWidget->setSpacing(10); // Set the spacing between items

    // Add each file name to the list widget
    for (const auto& fileName : m_fileNames) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(fileName.second));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter); // Center the text horizontally and vertically
        if (fileName.second.compare(m_currentFile) == 0) {
            item = new QListWidgetItem(QString::fromStdString("<<< " + fileName.second + " >>>"));
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter); // Center the text horizontally and vertically
            item->setForeground(Qt::red); // Highlight the current file name in red
            item->setFont(QFont("", 15, QFont::Bold)); // Set the font to bold
        }
        listWidget->addItem(item);
    }

    // Add the list widget to a vertical layout and set the layout for the dialog
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->addWidget(listWidget);
    dialog.setLayout(layout);

    // Display the dialog window and wait for the user to close it
    dialog.exec();
}


    
void CBR::PageSuivante()
{


    QGraphicsScene* scene = new QGraphicsScene(this);
    ui.graphicsView->setScene(scene);

    if (currentPage < g_page_num_total - 1)
    {
        currentPage += 1;
        g_is_page_current_changed = true;
        g_preload_mutex.lock();
        p.loadAndCacheImage(currentPage);
        preloaded = false;
        g_preload_mutex.unlock();
        qDebug() << "ok4";
        cv::Mat image;
        image = *cache.object(currentPage)->cv_image_ptr;
        qDebug() << "ok5";
        //image = a.ChargerImage(v.get_page_Number());
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
    else
    {
        g_preload_mutex.lock();
        p.loadAndCacheImage(currentPage);
        preloaded = false;
        g_preload_mutex.unlock();
        qDebug() << "ok4";
        cv::Mat image;
        image = *cache.object(currentPage)->cv_image_ptr;
        qDebug() << "ok5";
        //image = a.ChargerImage(v.get_page_Number());
        QImage qimage(image.data, image.cols, image.rows, image.step, QImage::Format_BGR888);
        QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(qimage));
        scene->addItem(pixmapItem);
        ui.graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
        ui.graphicsView->setRenderHint(QPainter::Antialiasing);
        ui.graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);

        ui.graphicsView->setDragMode(QGraphicsView::ScrollHandDrag); // enable scroll hand drag

        ui.graphicsView->setMouseTracking(true);

        ui.graphicsView->viewport()->installEventFilter(this);

        QMessageBox::warning(nullptr, "Warning", "There is no page after this one!");

    }


}


void CBR::PagePrecedante()
{

    QGraphicsScene* scene = new QGraphicsScene(this);
    ui.graphicsView->setScene(scene);

    if (currentPage > 0)
    {
        currentPage -= 1;
        g_is_page_current_changed = true;
        g_preload_mutex.lock();
        p.loadAndCacheImage(currentPage);
        preloaded = false;
        g_preload_mutex.unlock();
        qDebug() << "ok4";
        cv::Mat image;
        image = *cache.object(currentPage)->cv_image_ptr;
        qDebug() << "ok5";
        //image = a.ChargerImage(v.get_page_Number());
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
    else
    {
        g_preload_mutex.lock();
        p.loadAndCacheImage(currentPage);
        preloaded = false;
        g_preload_mutex.unlock();
        qDebug() << "ok4";
        cv::Mat image;
        image = *cache.object(currentPage)->cv_image_ptr;
        qDebug() << "ok5";
        //image = a.ChargerImage(v.get_page_Number());
        QImage qimage(image.data, image.cols, image.rows, image.step, QImage::Format_BGR888);
        QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(qimage));
        scene->addItem(pixmapItem);
        ui.graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
        ui.graphicsView->setRenderHint(QPainter::Antialiasing);
        ui.graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);

        ui.graphicsView->setDragMode(QGraphicsView::ScrollHandDrag); // enable scroll hand drag

        ui.graphicsView->setMouseTracking(true);

        ui.graphicsView->viewport()->installEventFilter(this);
        QMessageBox::warning(nullptr, "Warning", "There is no page before this one!");

    }


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
        // check if the mouse is over the graphics view
        if (ui.graphicsView->underMouse()) {
            onViewDoubleClicked(dynamic_cast<QMouseEvent*>(event));
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void CBR::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton && ui.graphicsView->underMouse())
    {
        // start dragging
        m_lastMousePos = event->pos();
        m_dragging = true;
    }
}

void CBR::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging && ui.graphicsView->underMouse())
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


void CBR::loadImageFromZip()
{
    QWidget mainWidget;
    mainWidget.setWindowTitle("Select Page");
    int page_number = getNumberFromUser(&mainWidget);
    currentPage = page_number;

    QGraphicsScene* scene = new QGraphicsScene(this);
    ui.graphicsView->setScene(scene);
    g_is_page_current_changed = true;
    g_preload_mutex.lock();
    p.loadAndCacheImage(currentPage);
    preloaded = false;
    g_preload_mutex.unlock();
    qDebug() << "ok4";
    cv::Mat image;
    image = *cache.object(currentPage)->cv_image_ptr;
    qDebug() << "ok5";
    //image = a.ChargerImage(v.get_page_Number());
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

int CBR::getNumberFromUser(QWidget* parent)
{
    bool ok;
    int number = QInputDialog::getInt(parent, "Enter Number", "Number:", 0, 0, INT_MAX, 1, &ok);
    if (ok)
    {
        return number;
    }
    else
    {
        return -1; // ou une autre valeur pour indiquer une erreur ou une annulation de l'utilisateur
    }
}



void CBR::SaveImage()
{
    // Load the image and its filename from the archive
    cv::Mat image;
    ArchiveExtraction a(g_archive_path);
    a.LireArchive();
    image = *cache.object(currentPage)->cv_image_ptr;
    std::string originalFilename = a.GetListeFichier()[currentPage];
    QString qstr = QString::fromStdString(originalFilename);
    qDebug() << qstr;
    // Prompt the user to choose a directory to save the image
    QString directory = QFileDialog::getExistingDirectory(this, tr("Save Image"), "");

    // Check if the user canceled the dialog or didn't choose a directory
    if (directory.isEmpty()) {
        return;
    }

    // Combine the directory and the original filename to create the full file path
    QString filePath = directory + "/" + QString::fromStdString(originalFilename);

    // Save the image to the specified file path
    cv::imwrite(filePath.toStdString(), image);
}


void CBR::showAboutDialog()
{
    QMessageBox::information(this, tr("About"), tr("Projet IN204 2022/2023 -- ENSTA PARIS."));
}