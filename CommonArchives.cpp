#include "CommonArchives.h"
#include <algorithm>
#include <archive_entry.h>
#include <archive.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <map>
#include<filesystem>
#include <iostream>
#include <map>
#include <algorithm>
#include<QDebug>

CommonArchives::~CommonArchives()
{
    nombreTotalPages = 0;
}


CommonArchives::CommonArchives(CommonArchives& ar1)
{
    CheminArchive = ar1.GetarchivePath();
    nombreTotalPages = ar1.GetNombreTotalePage();
}


CommonArchives::CommonArchives(std::string path1)
{

    CheminArchive = path1;
    nombreTotalPages = 0;
}


void CommonArchives::SetNombreTotalPages(int nombre)
{

    nombreTotalPages = nombre;
}


void CommonArchives::LireArchive()
{
    const char* CheminFichier = CheminArchive.c_str();

    std::vector<std::string> filenames;
    struct archive* a;
    struct archive_entry* entry;

    int r;
    int counteurNmbrPages = 0;

    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    if (CheminFichier != NULL && strcmp(CheminFichier, "-") == 0)
        CheminFichier = NULL;
    if ((r = archive_read_open_filename(a, CheminFichier, 10240))){}

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        std::string NomFichier = archive_entry_pathname(entry);
        size_t PositionType = NomFichier.rfind(".");
        std::string TypeFichier(NomFichier.begin() + PositionType + 1, NomFichier.end());
        transform(TypeFichier.begin(), TypeFichier.end(), TypeFichier.begin(), ::tolower);

        if (TypeFichier == "png" || TypeFichier == "jpg" || TypeFichier == "bmp") {
            filenames.push_back(NomFichier);
            counteurNmbrPages++;
        }
    }

    SetNombreTotalPages(counteurNmbrPages);

    std::sort(filenames.begin(), filenames.end());

    ListeFichier.clear();
    for (int i = 0; i < filenames.size(); i++) {
        ListeFichier.insert(std::pair<int, std::string>(i, filenames[i]));
    }

    archive_read_close(a);
    archive_read_free(a);
}





void CommonArchives::Extract(const char* filename, int DoExtract, int flags, int numPage, cv::Mat& b)
{
    //LireArchive();
    struct archive* a;
    struct archive* ext;
    struct archive_entry* entry;

    int r;
    int counteurNmbrPages = 0;

    a = archive_read_new();

    ext = archive_write_new();
    archive_write_set_format_ustar(ext);

    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    if (filename != NULL && strcmp(filename, "-") == 0)
        filename = NULL;
    if ((r = archive_read_open_filename(a, filename, 10240))){}

    for (;;)
    {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r != ARCHIVE_OK){}
        std::string NomFichier = archive_entry_pathname(entry);
        if (DoExtract)
        {
            //qDebug() << ("------nom-------"+NomFichier).c_str();
            //qDebug() << ("-----liste-----"+ListeFichier[numPage]).c_str();
            if (NomFichier == ListeFichier[numPage]) {
                    size_t entry_size = archive_entry_size(entry);
                    std::vector<char> data(entry_size);
                    archive_read_data(a, &data[0], entry_size);
                    b = cv::imdecode(cv::Mat(data), cv::IMREAD_COLOR);
                    r = archive_write_data(ext, &data[0], entry_size);
                    if (r != ARCHIVE_OK){}
                    r = archive_write_finish_entry(ext);
                    if (r != ARCHIVE_OK){}
                    break;
            }
            counteurNmbrPages += 1;
        }

    }

    archive_write_free(ext);
    archive_read_close(a);
    archive_read_free(a);
}



void CommonArchives::setPath(std::string path)
{
    CheminArchive = path;
}