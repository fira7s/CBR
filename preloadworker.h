#ifndef PRELOADWORKER_H
#define PRELOADWORKER_H

#include <QImage>
#include "cache.h"
#include <opencv2/opencv.hpp>
#include "cache.h"
#include "CommonArchives.h"

extern bool current_page_changed;
extern int page_num_total;
extern std::string current_archive_path;
extern std::mutex preload_mutex;
extern bool preloaded;
extern int currentPage;
extern CommonArchives current_Archive;
extern int preload_left_size;
extern int preload_right_size;




class PreLoadWorker
{
public:
    PreLoadWorker();
    void loadAndCacheImage(const int page_num);
    void parallelLoadPage();

};



#endif

