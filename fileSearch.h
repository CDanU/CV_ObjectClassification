/*
  * fileSearh.h
  * Search for files in a folder
  */

#include <stdio.h>  /* defines FILENAME_MAX */
#include <stdlib.h>
#include <dirent.h>

#ifdef WIN32
    #include <Windows.h>
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #define _LINUX_ // unterscheidung von MAC OS und anderen noch notwenig. Am besten check auf Ubuntu etc.
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <errno.h>

std::string getCurrentPath()
{
    char cCurrentPath[FILENAME_MAX];

    if( !GetCurrentDir( cCurrentPath, sizeof(cCurrentPath) ) )
    {
        std::cerr << "GetCurrentDir failed with " << errno << std::endl;
        return "";
    }

    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';   /* not really required */

    std::string path( cCurrentPath );
    std::replace( path.begin(), path.end(), '\\', '/' );
    path += "/";

    printf( "The current working directory is \"%s\"\n\n", path.c_str() );

    return path;
}

void fileSearch( std::vector< std::string >& files, std::string path, std::string filter = "*.*" )
{
    {
        printf( "Search for %s in \"%s\"\n\n", filter.c_str(), path.c_str() );

        DIR * dir;
        if( ( dir = opendir( path.c_str() ) ) != NULL )
        {
            struct dirent * ent;

            std::transform( filter.begin(), filter.end(), filter.begin(), tolower );

            bool allTypes = false;
            bool allNames = false;

            std::string filterName;
            std::string filterType;

            std::size_t pos = filter.find( "." );

            if( pos != std::string::npos )
            {
                filterName = filter.substr( 0, pos );
                filterType = filter.substr( pos + 1 );
                allTypes   = (filterType[0] == '*');
                allNames   = (filterName[0] == '*');
            }

            std::string name;
            std::string type;

            while( ( ent = readdir( dir ) ) != NULL )
            {
                // std::cout << ent->d_type << std::endl;
                name = ent->d_name;
                pos  = name.find( "." );
                type = (pos == std::string::npos) ? "?" : name.substr( pos + 1 );

                std::transform( type.begin(), type.end(), type.begin(), tolower );

                if( ( filter.compare( name ) == 0) || allTypes || ( filterType.compare( type ) == 0) )
                {
                    files.push_back( std::string( ent->d_name ) );
                }
            }

            closedir( dir );
        }
        else
        {
            perror( "could not open directory" );
            return;
        }
    }
}
