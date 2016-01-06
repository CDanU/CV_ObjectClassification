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



#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

const char * locale = setlocale( LC_ALL, "" ); // fuer das Anzeigen von UTF-8 zeichen in der Console

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

