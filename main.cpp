/*
  * Daniel Chumak & Mario Link
  *
  * Name: CV_ObjectClassification
  * Date: 06.01.2016
  * Version: 0.1.0
  * Description:
  * This Application ... TEXT ...
  */

#include "fileSearch.h"
#include <string>



const char * locale = setlocale( LC_ALL, "" ); // für das Anzeigen von UTF-8 zeichen in der Console

std::vector< std::string > files;

int main()
{
    std::string workpath = getCurrentPath();

    fileSearch( files, "../../50Objects/", "*.jpg" );

    std::cout << "Files:\n";
    for( auto& f : files )
    {
        std::cout << f << std::endl;
    }

    std::cin.ignore();
    return 0;
}
