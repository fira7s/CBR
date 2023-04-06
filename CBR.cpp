#include "CBR.h"
#include <archive.h>
#include <archive_entry.h>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include "CommonArchives.h"
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
#include <chrono>
#include <thread>


//global params
QCache<int, ImageData> cache;
QReadWriteLock cache_lock;
CommonArchives current_Archive;
std::string current_archive_path;
int currentPage=-1;
bool single_view=true;
std::chrono::steady_clock::time_point lastCallTime;
bool done_load=false;
int r = 0;

// thread 2 params
bool current_page_changed=false;
int  page_num_total;
std::mutex preload_mutex;
bool preloaded=true;
int preload_left_size=13;
int preload_right_size=13;


CBR::CBR(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    QFont font("Segoe UI", 10);
    QSize iconSize(32, 32);

    connect(ui.extractButton, &QPushButton::clicked, this, &CBR::extractArchive);

    ui.extractButton->setFont(font);
    QIcon icon1("assets/load.png");
    ui.extractButton->setIconSize(iconSize);
    ui.extractButton->setIcon(icon1.pixmap(iconSize));


    connect(ui.suivantButton, &QPushButton::clicked, this, &CBR::PageSuivante);
    ui.suivantButton->setFont(font);
    QIcon icon2("assets/next.png");
    ui.suivantButton->setIconSize(iconSize);
    ui.suivantButton->setIcon(icon2.pixmap(iconSize));

    connect(ui.precedantButton, &QPushButton::clicked, this, &CBR::PagePrecedante);    
    ui.precedantButton->setFont(font);
    QIcon icon3("assets/back.png");
    ui.precedantButton->setIconSize(iconSize);
    ui.precedantButton->setIcon(icon3.pixmap(iconSize));



    connect(ui.createButton, &QPushButton::clicked, this, &CBR::createArchive);   
    ui.createButton->setFont(font);
    QIcon icon4("assets/create.png");
    ui.createButton->setIconSize(iconSize);
    ui.createButton->setIcon(icon4.pixmap(iconSize));


    connect(ui.SommaireButton, &QPushButton::clicked, this, &CBR::sommaire);  
    ui.SommaireButton->setFont(font);
    QIcon icon5("assets/sommaire.png");
    ui.SommaireButton->setIconSize(iconSize);
    ui.SommaireButton->setIcon(icon5.pixmap(iconSize));


    connect(ui.SelectButton, &QPushButton::clicked, this, &CBR::select_page);
    ui.SelectButton->setFont(font);
    QIcon icon6("assets/select.png");
    ui.SelectButton->setIconSize(iconSize);
    ui.SelectButton->setIcon(icon6.pixmap(iconSize));


    connect(ui.SaveButton, &QPushButton::clicked, this, &CBR::SaveImage);    
    ui.SaveButton->setFont(font);
    QIcon icon7("assets/save.png");
    ui.SaveButton->setIconSize(iconSize);
    ui.SaveButton->setIcon(icon7.pixmap(iconSize));

    ui.label->setFont(font);
    connect(ui.radioButton, &QRadioButton::clicked, this, &CBR::single_view_change);
    ui.radioButton->setChecked(true);
    connect(ui.radioButton_2, &QRadioButton::clicked, this, &CBR::double_view_change);
    ui.radioButton->setFont(font);
    ui.radioButton_2->setFont(font);
    QAction* aProposAction = new QAction(tr("A propos"), this);
    menuBar()->addAction(aProposAction);
    connect(aProposAction, &QAction::triggered, this, &CBR::showAboutDialog);
    p = PreLoadWorker();
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CBR::checkVariable);
    timer->start(2);
} 

CBR::~CBR()
{

}

void CBR::checkVariable() {
    if (done_load == true) {
        display();
    }
}
void CBR::loadasync()
{
    p.loadAndCacheImage(currentPage); 
    if(!single_view){ p.loadAndCacheImage(currentPage-1); }
    done_load = true;
    qDebug() << "ruuning";
}

void CBR::display()
{
    r = 0;
    done_load = false;
    QGraphicsScene* scene = new QGraphicsScene();
    ui.graphicsView->setScene(scene);
    if (single_view)
    {
        cv::Mat image = *cache.object(currentPage)->cv_image_ptr;
        QImage qimage(image.data, image.cols, image.rows, image.step, QImage::Format_BGR888);
        QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(qimage));
        scene->addItem(pixmapItem);
        ui.graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
        ui.graphicsView->setRenderHint(QPainter::Antialiasing);
        ui.graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);

        ui.graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

        ui.graphicsView->setMouseTracking(true);

        ui.graphicsView->viewport()->installEventFilter(this);
    }
    else {
        cv::Mat image1 = *cache.object(currentPage - 1)->cv_image_ptr;
        cv::Mat image2 = *cache.object(currentPage)->cv_image_ptr;

        QImage qimage1(image1.data, image1.cols, image1.rows, image1.step, QImage::Format_BGR888);
        QGraphicsPixmapItem* pixmapItem1 = new QGraphicsPixmapItem(QPixmap::fromImage(qimage1));

        QImage qimage2(image2.data, image2.cols, image2.rows, image2.step, QImage::Format_BGR888);
        QGraphicsPixmapItem* pixmapItem2 = new QGraphicsPixmapItem(QPixmap::fromImage(qimage2));


        scene->addItem(pixmapItem1);
        scene->addItem(pixmapItem2);

        int padding = 20;
        pixmapItem1->setPos(0, 0);
        pixmapItem2->setPos(image1.cols + padding, 0);


        ui.graphicsView->setScene(scene);
        ui.graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
        ui.graphicsView->setRenderHint(QPainter::Antialiasing);
        ui.graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);

        ui.graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
        ui.graphicsView->setMouseTracking(true);
        ui.graphicsView->viewport()->installEventFilter(this);
    }
    preload_mutex.lock();
    current_page_changed = false;
    preloaded = false;
    preload_mutex.unlock();
}

void CBR::extractArchive()
{
    QString selected_file = QFileDialog::getOpenFileName(nullptr, "Select archive", "", "Archive files (*.zip *.7z *.cbr *.rar *.cbz)");

    if (!selected_file.isEmpty() and current_archive_path!= selected_file.toStdString()) {
         current_archive_path = selected_file.toStdString();
         current_page_changed = true;
         cache.clear();
    }
    else if (selected_file.isEmpty()) { return; }
    current_Archive.setPath(current_archive_path);
    current_Archive.LireArchive();
    if (current_Archive.GetNombreTotalePage() == 0 and !selected_file.isEmpty())
    {
        QMessageBox::warning(nullptr, "Warning", "Archive vide ou ne contient pas d'images!");
        return;
    }
    page_num_total = current_Archive.GetNombreTotalePage();

    currentPage = 0;
    std::thread load(&CBR::loadasync, this);
    load.detach();
    r = 1;

}




void CBR::launch_load()
{
    r = 1;
    current_page_changed = true;
    std::thread load(&CBR::loadasync, this);
    load.detach();
   
}


void CBR::PageSuivante()
{
    qDebug() << "r="<<r;
    if (r==1)
    {
        qDebug() << "out";
        return;
    }
    if (currentPage >= 0)
    {
        if (currentPage < page_num_total - 1)
        {
            if (single_view)
            {
                currentPage += 1;
                launch_load();
            }
            else
            {
                currentPage += 2;
                launch_load();
            }
        }
        else
        {
            if (single_view)
            {
                display();
                QMessageBox::warning(nullptr, "Warning", "Il n'y a pas de page apres celle-ci !");

            }
            else
            {
                display();
                QMessageBox::warning(nullptr, "Warning", "Il n'y a pas de page apres celle-ci !");

            }
        }
    }
    else {
        QMessageBox::warning(nullptr, "Warning", "Veuillez charger un archive avant!");

    }






}


void CBR::PagePrecedante()
{
    qDebug() << "r=" << r;
    if (r == 1)
    {
        qDebug() << "out";
        return;
    }

    if (currentPage >= 0)
    {
        
        int test = currentPage;
        if (single_view)
        {
            test -= 1;
        }
        else { test -= 2; }
        if (test >= 0)
        {
            if (single_view)
            {
                currentPage -= 1;
                launch_load();
            }
            else
            {
                currentPage -= 2;
                if (currentPage == 0) { currentPage = 1; }
                launch_load();
            }

        }
        else
        {
            if (single_view)
            {
                display();
                QMessageBox::warning(nullptr, "Warning", "Il n'y a pas de page avant celle-ci !");

            }
            else
            {

                display();
                QMessageBox::warning(nullptr, "Warning", "Il n'y a pas de page avant celle-ci !");

            }

        }
    }
  else {
      QMessageBox::warning(nullptr, "Warning", "Veuillez charger un archive avant!");

    }


}


void CBR::sommaire()
{
    if (currentPage >= 0)
    {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Sommaire"));
        dialog.setMinimumSize(400, 400);

        QListWidget* listWidget = new QListWidget(&dialog);
        listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        listWidget->setSpacing(10);

        for (int i = 0; i < page_num_total; i++) {
            QListWidgetItem* item = new QListWidgetItem(QString("Page ") + QString::number(i + 1));
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            if (i == currentPage) {
                item = new QListWidgetItem(QString::fromStdString("<<< Page ") + QString::number(i + 1) + QString(" >>>"));
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                item->setForeground(Qt::red);
                item->setFont(QFont("", 15, QFont::Bold));
            }
            listWidget->addItem(item);
        }

        QVBoxLayout* layout = new QVBoxLayout(&dialog);
        layout->addWidget(listWidget);
        dialog.setLayout(layout);

        dialog.exec();
    }
    else {
        QMessageBox::warning(nullptr, "Warning", "Veuillez charger un archive avant!");

    }
}

void CBR::onViewDoubleClicked(QMouseEvent* event)
{
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
        m_lastMousePos = event->pos();
        m_dragging = true;
    }
}

void CBR::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging && ui.graphicsView->underMouse())
    {
        QPoint offset = event->pos() - m_lastMousePos;
        ui.graphicsView->setSceneRect(ui.graphicsView->sceneRect().translated(-offset.x(), -offset.y()));
        m_lastMousePos = event->pos();
    }
}


void CBR::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        m_dragging = false;
    }
}


void CBR::createArchive()
{
    QString archiveName = QFileDialog::getSaveFileName(this, "Save archive", QDir::homePath(), "Comic Book Archive files (*.cbr)");

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

    QStringList images = QFileDialog::getOpenFileNames(this, "Selectionez les images a ajouter", QDir::homePath(), "Images (*.jpg *.jpeg *.png *.bmp)");
    for (const auto& image : images) {
        QFile file(image);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "Error", "Cannot open file " + file.fileName());
            continue;
        }

        QFileInfo fileInfo(image);
        entry = archive_entry_new();
        archive_entry_set_pathname(entry, fileInfo.fileName().toStdString().c_str());
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

    QFile archiveFile(archiveName);
    if (archiveFile.open(QIODevice::ReadWrite)) {
        QByteArray cbrHeader = "Comic Book Archive\r\n";
        archiveFile.seek(0);
        archiveFile.write(cbrHeader);
        archiveFile.close();
    }
    else {
        QMessageBox::critical(this, "Error", "Cannot add CBR header");
        return;
    }

    QMessageBox::information(this, "Archive created", "L'archive a ete creee avec succes.");
}






void CBR::select_page()
{
    if (currentPage >= 0)
    {
        QWidget mainWidget;
        mainWidget.setWindowTitle("Select Page");
        int page_number = getNumberFromUser(&mainWidget);
        if (page_number >= 1 and page_number<=page_num_total) {
            current_page_changed = true;
            currentPage = page_number-1; }
        else if(page_number == 0 || page_number > page_num_total){
            QMessageBox::warning(nullptr, "Warning", "Veuillez verfier votre saisie ! Regardez le sommaire pour en savoir plus");
        }
        QGraphicsScene* scene = new QGraphicsScene(this);
        ui.graphicsView->setScene(scene);
        if (single_view)
        {
            launch_load();

        }
        else
        {
            if (currentPage == 0) { currentPage += 1; }
            launch_load();
        }
    }
    else {
        QMessageBox::warning(nullptr, "Warning", "Veuillez charger un archive avant!");

    }

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
        return -1; 
    }
}



void CBR::SaveImage()
{
    if (currentPage >= 0 and single_view)
    {
        cv::Mat image;
        CommonArchives a(current_archive_path);
        a.LireArchive();
        image = *cache.object(currentPage)->cv_image_ptr;
        std::string originalFilename = a.GetListeFichier()[currentPage];
        QString qstr = QString::fromStdString(originalFilename);
        QString directory = QFileDialog::getExistingDirectory(this, tr("Save Image"), "");

        if (directory.isEmpty()) {
            return;
        }

        QString filePath = directory + "/" + QString::fromStdString(originalFilename);

        cv::imwrite(filePath.toStdString(), image);
        QMessageBox::information(this, "Page enregistree", "La page a ete enregistre avec succes.");

    }
    if(currentPage >= 0 and !single_view){ QMessageBox::warning(nullptr, "Warning", "L'enregistrement n'est possible que dans le mode 1 page"); }
    else if (currentPage<0){
        QMessageBox::warning(nullptr, "Warning", "Veuillez charger un archive avant!");

    }
}


void CBR::showAboutDialog()
{
    QMessageBox::information(this, tr("About"), tr("Projet IN204 2022/2023 -- ENSTA PARIS."));
}


void CBR::single_view_change()
{
    if (currentPage >= 0)
    {
        if (single_view == false and currentPage != 0) { currentPage -= 1; }
        single_view = true;
        launch_load();
    }
    else { single_view = true; }
}

void CBR::double_view_change()
{
    if (currentPage >= 0)
    {
    if (single_view = true and currentPage!= page_num_total) { currentPage += 1; }
    single_view = false;
    launch_load();
}
    else { single_view = false; }
}