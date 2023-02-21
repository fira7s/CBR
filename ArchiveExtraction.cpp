#include "ArchiveExtraction.h"
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


ArchiveExtraction::~ArchiveExtraction()
{}


ArchiveExtraction::ArchiveExtraction(ArchiveExtraction& ar1)
{
    CheminArchive = ar1.GetarchivePath();
    nombreTotalPages = ar1.GetNombreTotalePage();
}


ArchiveExtraction::ArchiveExtraction(std::string path1)
{

    CheminArchive = path1;
    nombreTotalPages = 0;
}


void ArchiveExtraction::SetNombreTotalPages(int nombre)
{

    nombreTotalPages = nombre;
}

static void Echouer(const char* f, const char* m, int r)
{

}

static int verbose = 0;

static void Notifier(const char* f, const char* m)
{

}
static void message(const char* m){}
static int CopierDonnees(struct archive* ar, struct archive* aw)
{
    int r;
    const void* buff;
    size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
    int64_t offset;
#else
    off_t offset;
#endif

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r != ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK) {
            Notifier("archive_write_data_block()",
                archive_error_string(aw));
            return (r);
        }
    }
}

void ArchiveExtraction::LireArchive()
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
    if ((r = archive_read_open_filename(a, CheminFichier, 10240)))
        Echouer("archive_read_open_filename()", archive_error_string(a), r);

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




bool ArchiveExtraction::DecompresserArchive(int numPage, std::string path)
{//Decompresser une page d'une archive
    if (numPage<0 || numPage>nombreTotalPages)
    {
        return false;
    }

    const char* ArchivePathName = path.c_str();
    const char* CheminFichier = ArchivePathName;

    int flags, mode;
    mode = 'x';
    verbose = 0;
    flags = ARCHIVE_EXTRACT_TIME;

    switch (mode) {
    case 't':
        Extract(CheminFichier, 0, flags, numPage);
        break;
    case 'x':
        Extract(CheminFichier, 1, flags, numPage);
        break;
    }
    return true;

}


void ArchiveExtraction::Extract(const char* filename, int DoExtract, int flags, int numPage)
{

    struct archive* a;
    struct archive* ext;
    struct archive_entry* entry;//structure permet d'index chaque document dans archive

    int r;
    int counteurNmbrPages = 0;

    a = archive_read_new();//initialise a pour la lecture

    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);

    archive_read_support_filter_all(a);//pour pouvoir lire tous les formats
    archive_read_support_format_all(a);

    if (filename != NULL && strcmp(filename, "-") == 0)
        filename = NULL;
    if ((r = archive_read_open_filename(a, filename, 10240)))// si on a bien ouvrir le file ou pas
        Echouer("archive_read_open_filename()", archive_error_string(a), r);

    for (;;)
    {
        r = archive_read_next_header(a, &entry);//lire chaque document dans l'archive
        if (counteurNmbrPages > numPage)
            break;
        if (r == ARCHIVE_EOF)//archive end of file
            break;
        if (r != ARCHIVE_OK)
            Echouer("archive_read_next_header()", archive_error_string(a), 1);
        if (verbose && DoExtract)
            message("x ");
        if (verbose || !DoExtract)
            message(archive_entry_pathname(entry));
        if (DoExtract)
        {
            if (numPage == counteurNmbrPages) {
                r = archive_write_header(ext, entry);
                if (r != ARCHIVE_OK)
                    Notifier("archive_write_header()", archive_error_string(ext));
                else {
                    CopierDonnees(a, ext);//copier un seul fichier de l'archive dans disque
                    r = archive_write_finish_entry(ext);
                    break;
                    if (r != ARCHIVE_OK)
                        Echouer("archive_write_finish_entry()", archive_error_string(ext), 1);

                }
            }
            counteurNmbrPages += 1;
        }
        if (verbose || !DoExtract)
            message("\n");
    }


    archive_read_close(a);
    archive_read_free(a);

    archive_write_close(ext);
    archive_write_free(ext);

}


cv::Mat ArchiveExtraction::ChargerImage(int numeroPage)
{//Charger une image d'un fichier
    LireArchive();
    std::string PathFile;
    PathFile = std::filesystem::current_path().string();
    char lettre1 = '\\';
    char lettre2 = '/';
    char* chaine = (char*)PathFile.c_str();
    for (size_t i = 0; i < strlen(chaine); i++)
    {
        if (chaine[i] == lettre1) PathFile[i] = lettre2;
    }
    PathFile = PathFile + "/" + ListeFichier[numeroPage];
    DecompresserArchive(numeroPage, CheminArchive);
    cv::Mat temp_image = cv::imread(PathFile, cv::IMREAD_COLOR); //Charge une image à partir d'un fichier
    return temp_image;
}