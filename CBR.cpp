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


// thread 2 params
bool current_page_changed=false;
bool current_path_changed =false;
int  page_num_total;
std::mutex preload_mutex;
bool preloaded=true;
int preload_left_size=7;
int preload_right_size=7;


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
    current_archive_path = "data/ex3.zip";
} 

CBR::~CBR()
{

}


void CBR::extractArchive()
{
    QString selected_file = QFileDialog::getOpenFileName(nullptr, "Select archive", "", "Archive files (*.zip *.7z *.cbr)");

    if (!selected_file.isEmpty() and current_archive_path!= selected_file.toStdString()) {
         current_archive_path = selected_file.toStdString();
         cache.clear();
    }
    current_Archive.setPath(current_archive_path);
    current_Archive.LireArchive();
    page_num_total = current_Archive.GetNombreTotalePage();
    QGraphicsScene* scene = new QGraphicsScene();
    ui.graphicsView->setScene(scene);
    if (single_view) 
    {
        currentPage = 0;
        p.loadAndCacheImage(currentPage);
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
        preload_mutex.lock();
        preloaded = false;
        preload_mutex.unlock();
    }
    else
    {
        currentPage = 1;
        p.loadAndCacheImage(currentPage);
        p.loadAndCacheImage(currentPage-1);
        cv::Mat image1 = *cache.object(currentPage-1)->cv_image_ptr;
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
        preload_mutex.lock();
        preloaded = false;
        preload_mutex.unlock();
    }

}
void CBR::sommaire()
{
    if(currentPage>=0)
    { QDialog dialog(this);
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



void CBR::PageSuivante()
{
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCallTime);
    if (elapsed.count() < 350) {
        return;
    }

    lastCallTime = now;
    if (currentPage >= 0)
    {  
    QGraphicsScene* scene = new QGraphicsScene(this);
    ui.graphicsView->setScene(scene);
    if (currentPage < page_num_total - 1)
    {
        if (single_view)
        {
            current_page_changed = true;
            currentPage += 1;
            p.loadAndCacheImage(currentPage);
            cv::Mat image;
            image = *cache.object(currentPage)->cv_image_ptr;
            QImage qimage(image.data, image.cols, image.rows, image.step, QImage::Format_BGR888);
            QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(qimage));
            scene->addItem(pixmapItem);
            ui.graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
            ui.graphicsView->setRenderHint(QPainter::Antialiasing);
            ui.graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);

            ui.graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

            ui.graphicsView->setMouseTracking(true);

            ui.graphicsView->viewport()->installEventFilter(this);
            preload_mutex.lock();
            current_page_changed = false;
            preloaded = false;
            preload_mutex.unlock();

        }
        else
        {
            currentPage += 2;
            current_page_changed = true;
            p.loadAndCacheImage(currentPage);
            p.loadAndCacheImage(currentPage - 1);
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
            preload_mutex.lock();
            preloaded = false;
            preload_mutex.unlock();
        }
    }
    else
    {
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
            QMessageBox::warning(nullptr, "Warning", "Il n'y a pas de page apres celle-ci !");

        }
        else
        {

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
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCallTime);

    if (elapsed.count() < 350) {
        return;
    }

    lastCallTime = now;
    if (currentPage >= 0)
    {
        QGraphicsScene* scene = new QGraphicsScene(this);
        ui.graphicsView->setScene(scene);
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
                current_page_changed = true;
                p.loadAndCacheImage(currentPage);
                cv::Mat image;
                image = *cache.object(currentPage)->cv_image_ptr;
                QImage qimage(image.data, image.cols, image.rows, image.step, QImage::Format_BGR888);
                QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(qimage));
                scene->addItem(pixmapItem);
                ui.graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
                ui.graphicsView->setRenderHint(QPainter::Antialiasing);
                ui.graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);

                ui.graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

                ui.graphicsView->setMouseTracking(true);

                ui.graphicsView->viewport()->installEventFilter(this);
                preload_mutex.lock();
                preloaded = false;
                preload_mutex.unlock();
            }
            else
            {
                currentPage -= 2;
                if (currentPage ==0) { currentPage = 1; }
                current_page_changed = true;
                p.loadAndCacheImage(currentPage);
                p.loadAndCacheImage(currentPage - 1);
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
                preload_mutex.lock();
                preloaded = false;
                preload_mutex.unlock();
            }

        }
        else
        {
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
                QMessageBox::warning(nullptr, "Warning", "Il n'y a pas de page avant celle-ci !");

            }
            else
            {

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
                QMessageBox::warning(nullptr, "Warning", "Il n'y a pas de page avant celle-ci !");

            }

        }
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

    QMessageBox::information(this, "Archive created", "The archive was successfully created.");
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
            p.loadAndCacheImage(currentPage);
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
        else
        {
            if (currentPage == 0) { currentPage += 1; }
            p.loadAndCacheImage(currentPage);
            p.loadAndCacheImage(currentPage - 1);
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
        preloaded = false;
        preload_mutex.unlock();
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
        qDebug() << qstr;
        QString directory = QFileDialog::getExistingDirectory(this, tr("Save Image"), "");

        if (directory.isEmpty()) {
            return;
        }

        QString filePath = directory + "/" + QString::fromStdString(originalFilename);

        cv::imwrite(filePath.toStdString(), image);
    }
    if(currentPage >= 0 and !single_view){ QMessageBox::warning(nullptr, "Warning", "L'enregistrement n'est possible que dans le mode 1 page"); }
    else {
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
        current_page_changed = true;
        QGraphicsScene* scene = new QGraphicsScene(this);
        ui.graphicsView->setScene(scene);
        preload_mutex.lock();
        p.loadAndCacheImage(currentPage);
        preloaded = false;
        preload_mutex.unlock();
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
        preload_mutex.lock();
        preloaded = false;
        preload_mutex.unlock();
    }
    else { single_view = true; }
}

void CBR::double_view_change()
{
    if (currentPage >= 0)
    {
    if (single_view = true and currentPage!= page_num_total) { currentPage += 1; }
    single_view = false;
    current_page_changed = true;
    QGraphicsScene* scene = new QGraphicsScene(this);
    ui.graphicsView->setScene(scene);

        preload_mutex.lock();
        p.loadAndCacheImage(currentPage);
        p.loadAndCacheImage(currentPage - 1);
        preloaded = false;
        preload_mutex.unlock();
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
        preload_mutex.lock();
        preloaded = false;
        preload_mutex.unlock();
}
    else { single_view = false; }
}