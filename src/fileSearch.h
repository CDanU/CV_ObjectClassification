/*
  * fileSearh.h
  * Search for files in a folder
  */

#include <cerrno>
#include <dirent.h>
#include <stdio.h>
#include <iostream>
#include <string>
#ifdef WIN32
    #include <Windows.h>
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

static std::string getCurrentPath()
{
    using namespace std;

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

    printf( "The current working directory is \"%s\"\n", path.c_str() );

    return path;
}

static void fileSearch( std::vector< std::string >& files, std::string path, std::string filter = "*.*" )
{
    printf( "Search for %s in \"%s\"\n", filter.c_str(), path.c_str() );

    DIR * dir = opendir( path.c_str() );

    if( dir == NULL )
    {
        perror( "could not open directory" );
        return;
    }

    std::transform( filter.begin(), filter.end(), filter.begin(), ::tolower );

    struct dirent * ent;
    bool allTypes = false;
    std::string filterType;
    std::size_t pos = filter.find( "." );

    if( pos != std::string::npos )
    {
        filterType = filter.substr( pos + 1 );
        allTypes   = (filterType[0] == '*');
    }

    std::string name;
    std::string type;

    while( ( ent = readdir( dir ) ) != NULL )
    {
        name = ent->d_name;
        pos  = name.find( "." );
        type = (pos == std::string::npos) ? "?" : name.substr( pos + 1 );

        transform( type.begin(), type.end(), type.begin(), tolower );

        if( ( filter.compare( name ) == 0) || allTypes || ( filterType.compare( type ) == 0) )
        {
            files.push_back( std::string( ent->d_name ) );
        }
    }

    closedir( dir );
}
