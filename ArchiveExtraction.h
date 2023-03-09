#ifndef ARCHIVEEXTRACTION_H
#define ARCHIVEEXTRACTION_H

#include "Archives.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <opencv2/opencv.hpp>


class ArchiveExtraction : public Archives
{
public:
    ArchiveExtraction() :Archives() {};

    ~ArchiveExtraction();
    ArchiveExtraction(ArchiveExtraction& ar1);
    ArchiveExtraction(std::string path1);
    void SetNombreTotalPages(int nombre);
    void LireArchive();
    bool ChargerImage(int numeroPage,cv::Mat&);
    void Extract(const char* filename, int do_extract, int flags, int numPage, cv::Mat& a);
    bool DecompresserArchive(int numPage, std::string ArchivePathName1);

};


#endif
