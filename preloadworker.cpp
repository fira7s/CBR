#include "preloadworker.h"

#include<QDebug>
#include <archive_entry.h>
#include <archive.h>

PreLoadWorker::PreLoadWorker() {
    g_is_page_current_changed = false;
    g_is_preload_run = false;
    ArchiveExtraction a("data/ex3.zip");
    archive = a;
    //parallelLoadPage();
}

void PreLoadWorker::loadAndCacheImage(const int page_num) {
    //check if it already exists
    qDebug() << "Cache size: " << cache.size();
    ImageData* image_data_ptr;
    cache_lock.lockForRead();
    if (cache.contains(page_num)) {
   

        cache_lock.unlock();
        return;
    }
    cache_lock.unlock();
    //if it dosent exist
    image_data_ptr = new ImageData;
    //archive.ChargerImage(page_num, *(image_data_ptr->cv_image_ptr));
    qDebug() << "ok1";
    const char* c_str = g_archive_path.c_str();
    archive.Extract(c_str, 1, ARCHIVE_EXTRACT_TIME,page_num, *(image_data_ptr->cv_image_ptr));
    qDebug() << "ok3";
    cache_lock.lockForWrite();
    cache.insert(page_num,image_data_ptr);
    qDebug() << "Cache size: " << cache.size();
    cache_lock.unlock();
}

/*void PreLoadWorker::parallelLoadPage() {//using extern preload_params
    while (1) {

        std::unique_lock <std::mutex> lck(g_preload_mutex);
        g_preload_cv.wait(lck); //wait for the preload signal

        if (g_is_exit == true) return;
        if (g_is_path_changed == true) {
            image_processor.loadArchive(g_archive_path);
            g_is_path_changed = false;
        }

        g_is_preload_run = true;

        bool left_exceed = false;
        bool right_exceed = false;
        int page_type = g_preload_params.page_type;
        int page_num_current = g_preload_params.page_num_current;
        int page_preload_left_size = g_preload_params.page_preload_left_size;
        int page_preload_right_size = g_preload_params.page_preload_right_size;
        for (int i = 1; i <= qMax<int>(page_preload_left_size, page_preload_right_size); i++) {


            if (page_num_current + i <= g_page_num_total && i <= page_preload_right_size) {
                loadAndCacheImage(page_num_current + i, page_type);
            }
            else right_exceed = true;

            if (page_num_current - i >= 1 && i <= page_preload_left_size) {
                loadAndCacheImage(page_num_current - i, page_type);
            }
            else left_exceed = true;

            if (left_exceed == true && right_exceed == true) break;
            if (g_is_page_current_changed == true || g_is_path_changed == true) {
                //need to rerun this function from new params
                if (g_is_page_current_changed == true) g_is_page_current_changed = false;
                break;

            }
        }
        g_is_preload_run = false;
    }
    return;


}*/
