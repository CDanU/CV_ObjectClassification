#pragma once

/* Console Input */

#include "fileSearch.h"
#include "showImage.h"

static const std::string workpath = "../../50Objects/"; // getCurrentPath();

static void console()
{
    std::cout <<
        "##############################################\n" <<
        "# Name        : CV_ObjectClassification\n" <<
        "# Author      : Daniel Chumak, Mario Link\n" <<
        "# Version     : 0.1.0\n" <<
        "# Copyright   : HtwDanielMario@2016\n" <<
        "# Description : This Application ... TEXT ..\n" <<
        "##############################################\n\n";

    std::vector< std::string > files;
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
        else if( cmd.find( "show" ) == 0 )
        {
            int index = 0;

            std::string numbStr = cmd.size() > 5 ? cmd.substr( 5 ) : "";
            if( numbStr != "" ) { std::istringstream( numbStr ) >> index; }

            if( !files.empty() && ( index >= 0) && ( index < files.size() ) )
            {
                showImage( workpath + files[index] );
            }
            else { std::cout << "Invalid Index or no files in list!\n:" << index; }
        }
        else if( (cmd == "q") || (cmd == "exit") )
        {
            break;
        }
        else { std::cout << "Unknown Command"; }
        cmd = "";
    }
}