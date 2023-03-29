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
    virtual void LireArchive() = 0;

};




#endif
