#include <vector>
#include <string>
#include <map>
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
    //virtual bool ChargerImage(int numeroPage, cv::Mat& image) = 0;
    virtual void LireArchive(std::string path) = 0;
    virtual bool DecompresserArchive(int numPage, std::string ArchivePathName1) = 0;

};