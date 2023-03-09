#ifndef ARCHIVES_H
#define ARCHIVES_H

#include <vector>
#include <string>
#include <map>
#include <opencv2/opencv.hpp>

class Archives
{
protected:
    std::string CheminArchive;
    int nombreTotalPages;
    std::map <int, std::string> ListeFichier;
public:
    Archives();
    Archives(std::string path);
    std::string GetarchivePath();
    int GetNombreTotalePage();
    std::map <int, std::string> GetListeFichier();
    virtual bool ChargerImage(int numeroPage, cv::Mat&)=0;
    virtual void LireArchive() = 0;
    virtual bool DecompresserArchive(int numPage, std::string ArchivePathName1) = 0;

};




#endif
