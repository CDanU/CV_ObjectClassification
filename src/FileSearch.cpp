#include "FileSearch.h"

#include <iostream>
#include <algorithm>
#include <errno.h>
#include <stdio.h>
#include <vector>
#include <regex>
#include <string>

using namespace std::regex_constants;
using namespace std;

string Ue5::getCurrentPath()
{
    using namespace std;

    char cCurrentPath[FILENAME_MAX];

    if( !GetCurrentDir( cCurrentPath, sizeof(cCurrentPath) ) )
    {
        cerr << "GetCurrentDir failed with " << errno << endl;
        return "";
    }

    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';     /* not really required */

    string path( cCurrentPath );
    replace( path.begin(), path.end(), '\\', '/' );
    path += "/";

    return path;
}

void Ue5::match( vector< string >& list, string search, string regexStr, bool ignoreCase, bool split )
{
    smatch match;

    if( regex_match( search, match, regex( regexStr, ECMAScript | (ignoreCase ? icase : ECMAScript) ) ) )
    {
        if( match.empty() ) { return; }

        if( !split ) { list.push_back( match[0] ); }

        else
        {
            for( size_t i = 0; i < match.size(); ++i )
            {
                ssub_match sub_match = match[i];
                string piece         = sub_match.str();
                list.push_back( piece );
            }
        }
    }
}

void Ue5::search( vector< string >& files, string path, string regexStr = ".*" )
{
    DIR * dir = opendir( path.c_str() );

    if( dir == NULL )
    {
        perror( "could not open directory" );
        return;
    }

    struct dirent * ent;

#ifdef WIN32
    auto isFile = 0x8000;
#else
    auto isFile = 0x8;
#endif

    while( ( ent = readdir( dir ) ) != NULL )
    {
        if( ent->d_type == isFile )
        {
            match( files, string( ent->d_name ), regexStr, true );
        }
    }

    closedir( dir );
}
