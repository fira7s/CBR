#ifndef CACHE_H
#define CACHE_H

#include <mutex>
#include <thread>
#include <QSharedData>
#include <QCache>
#include <QImage>
#include <QReadWriteLock>
#include <opencv2/opencv.hpp>

class ImageData
{
public:
    cv::Mat* cv_image_ptr;
    ImageData() {
        cv_image_ptr = new cv::Mat;
    }
    ~ImageData() {
        delete cv_image_ptr;
    }
};


extern QCache<int, ImageData> cache;
extern QReadWriteLock cache_lock;





#endif // CACHE_H
