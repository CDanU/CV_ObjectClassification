/*
  * fileSearh.h
  * Search for files in a folder
  */

#include <stdio.h>  /* defines FILENAME_MAX */

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
    path += filter;

    printf( "Search for \"%s\"\n\n", path.c_str() );

    std::wstringstream cls;
    cls << path.c_str();
    std::wstring wpath = cls.str();

#ifdef _WINDOWS_
    WIN32_FIND_DATA data;
    HANDLE h = FindFirstFile( wpath.c_str(), &data );

    if( h != INVALID_HANDLE_VALUE )
    {
        do
        {
            char * nPtr = new char[lstrlen( data.cFileName ) + 1];

            for( int i = 0; i < lstrlen( data.cFileName ); i++ )
            {
                nPtr[i] = char(data.cFileName[i]);
            }

            nPtr[lstrlen( data.cFileName )] = '\0';

            files.push_back( std::string( nPtr ) );
        }
        while( FindNextFile( h, &data ) );
    }
    else
    {
        std::cerr << "Error: No such folder." << std::endl;
    }

    FindClose( h );

#else
    #ifdef _LINUX_
    // !TODO
    #endif
#endif
}
