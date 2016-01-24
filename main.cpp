/*
  * Daniel Chumak & Mario Link
  *
  * Name: CV_ObjectClassification
  * Date: 06.01.2016
  * Version: 0.1.0
  * Description:
  * This Application ... TEXT ...
  */

#include "includes.h"
#include "fileSearch.h"
#include <string>
#include "showImage.h"
#include "db.h"


const char * locale = setlocale( LC_ALL, "" ); // fuer das Anzeigen von UTF-8 zeichen in der Console

std::string workpath = "../../50Objects/"; // getCurrentPath();
std::vector< std::string > files;

/* ShutDown Event */
void OnExit()
{
    closeDB();
}

/* Main */
int main()
{
    REGISTER_SHUTDOWN_EVENT

        createDB( "test.yml" );

    std::string nnn;

    // Sleep(5000);
    openDB();
    readFromDB( "Name", nnn );
    std::cout << nnn.c_str();

    /* Console Input */

    std::cout <<
        "##############################################\n" <<
        "# Name        : CV_ObjectClassification\n" <<
        "# Author      : Daniel Chumak, Mario Link\n" <<
        "# Version     : 0.1.0\n" <<
        "# Copyright   : HtwDanielMario@2016\n" <<
        "# Description : This Application ... TEXT ..\n" <<
        "##############################################\n\n";

    std::string cmd;
    size_t f;
    char input[100];

    fileSearch( files, workpath, "*.jpg" );

    while( true )
    {
        std::cout << "\n> ";
        std::cin.getline( input, sizeof(input) );
        cmd = input;

        if( cmd == "list" )
        {
            if( !files.empty() ) { files.clear(); }
            fileSearch( files, workpath, "*.jpg" );

            std::cout << "List Files:\n";
            unsigned int count = 0;

            for( auto& f : files )
            {
                std::cout << count << ": " << f << std::endl;
                count++;
            }
        }
        else if( ( f = cmd.find( "show" ) ) == 0 )
        {
            std::string numbStr = cmd.size() > 5 ? cmd.substr( 5 ) : "";
            int index           = 0;
            if( numbStr != "" ) { std::istringstream( numbStr ) >> index; }
            if( !files.empty() && ( index >= 0) && ( index < files.size() ) ) { showImage( workpath + files[index] ); }
            else { std::cout << "Invalid Index or no files in list!\n:" << index; }
        }
        else if( ( cmd == "q") || ( cmd == "exit") )
        {
            break;
        }
        else { std::cout << "Unknown Command"; }
        cmd = "";
    }

    std::cin.ignore();
    return 0;
}
