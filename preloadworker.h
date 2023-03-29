#ifndef PRELOADWORKER_H
#define PRELOADWORKER_H

#include <QImage>
#include "cache.h"
#include <opencv2/opencv.hpp>
#include "cache.h"
#include "ArchiveExtraction.h"
#include "currentView.h"

extern bool g_is_page_current_changed;
extern bool g_is_path_changed;
extern bool g_is_preload_run;
extern int g_page_num_total;
extern std::string g_archive_path;
extern ImagePreloadParams g_preload_params;
extern std::mutex g_preload_mutex;
extern std::condition_variable g_preload_cv;
extern bool preloaded;
extern int currentPage;
extern ArchiveExtraction current_Archive;
extern int page_preload_left_size;
extern int page_preload_right_size;
class PreLoadWorker
{
public:
    PreLoadWorker();

    void loadAndCacheImage(const int page_num);


    void parallelLoadPage();



private:
    ImageData image_processor;
    ArchiveExtraction archive;
};

#endif

