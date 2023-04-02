#include "preloadworker.h"
#include<QDebug>
#include <archive_entry.h>
#include <archive.h>



PreLoadWorker::PreLoadWorker() {

    preloaded = true;
    std::thread preload([this]() { parallelLoadPage(); });
    preload.detach();

}


void PreLoadWorker::loadAndCacheImage(const int page_num) {

    qDebug() << "Cache size: " << cache.size();
    ImageData* image_data_ptr;
    cache_lock.lockForRead();
    if (cache.contains(page_num)) {
   

        cache_lock.unlock();
        return;
    }
    cache_lock.unlock();

    image_data_ptr = new ImageData;
    const char* c_str = current_archive_path.c_str();
    current_Archive.Extract(c_str, 1, ARCHIVE_EXTRACT_TIME,page_num, *(image_data_ptr->cv_image_ptr));
    cache_lock.lockForWrite();
    cache.insert(page_num,image_data_ptr);
    cache_lock.unlock();

}



void PreLoadWorker::parallelLoadPage() {

    while (1) {
        preload_mutex.lock();
        if (preloaded==false) {     
            bool left_exceed = false;
            bool right_exceed = false;
            int page_num_current = currentPage;
            for (int i = 1; i <= qMax<int>(preload_left_size, preload_right_size); i++) {
                if (page_num_current + i <= page_num_total && i <= preload_right_size) {
                    loadAndCacheImage(page_num_current + i);
                    qDebug() << "in1";
                }
                else right_exceed = true;

                if (page_num_current - i >= 1 && i <= preload_left_size) {
                    loadAndCacheImage(page_num_current - i);
                    qDebug() << "in2";
                }
                else left_exceed = true;

                if (left_exceed == true && right_exceed == true) break;
                if (current_page_changed == true || current_path_changed == true) {
                    if (current_page_changed == true) { current_page_changed = false;}
                    if (current_path_changed == true) {current_path_changed = false;}
                    qDebug() << "out";
                    break;

                }
            }
        }
        preloaded = true;
        preload_mutex.unlock();
    }
}
