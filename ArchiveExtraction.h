#include "Archives.h"
#include <stdio.h>
#include <string.h>
#include <vector>

class ArchiveExtraction : public Archives
{
public:
    ArchiveExtraction() :Archives() {};

    ~ArchiveExtraction();
    ArchiveExtraction(ArchiveExtraction& ar1);
    ArchiveExtraction(std::string path1);
    void SetNombreTotalPages(int nombre);
    void LireArchive(std::string path);
    //bool ChargerImage(int numeroPage, cv::Mat& image);
    void Extract(const char* filename, int do_extract, int flags, int numPage);
    bool DecompresserArchive(int numPage, std::string ArchivePathName1);

};