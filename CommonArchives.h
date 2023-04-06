#ifndef COMMONARCHIVES_H
#define COMMONARCHIVES_H

#include "Archives.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <opencv2/opencv.hpp>

extern bool current_page_changed;

class CommonArchives : public Archives
{
public:
    CommonArchives() :Archives() {};

    ~CommonArchives();
    CommonArchives(CommonArchives& ar1);
    CommonArchives(std::string path1);
    void SetNombreTotalPages(int nombre);
    void setPath(std::string path);
    void LireArchive();
    void Extract(const char* filename, int do_extract, int flags, int numPage, cv::Mat& a);
};


#endif

