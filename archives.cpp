#include "Archives.h"


Archives::Archives()
{
    nombreTotalPages = 0;
}


Archives::Archives(std::string path)
{
    CheminArchive = path;
    nombreTotalPages = 0;
}


std::string Archives::GetarchivePath()
{
    return CheminArchive;
}


int Archives::GetNombreTotalePage()
{
    return nombreTotalPages;
}


std::map <int, std::string> Archives::GetListeFichier()
{
    return ListeFichier;
}
