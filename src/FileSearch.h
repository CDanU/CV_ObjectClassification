#pragma once

#include <dirent.h>
#include <string>
#include <vector>

#ifdef WIN32
    #include <Windows.h>
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

namespace Ue5
{
    //! returns current working dir path
    std::string getCurrentPath();

    /**
     * Searches files in a directory that match a regular expression
     *
     * @param files  : vector that will be populated with found files
     * @param path   : directory path in which files will be searched
     * @param filter : regex that will be applied on filenames
     */
    void search( std::vector< std::string >& files, std::string path, std::string filter );

    /**
     * Applies a regular expression on a string and returns eiterh the first
     * or all matched groups ( depending on split )
     *
     * @param list       : list that will be populated
     * @param search     : string on wich regular expression will be applied
     * @param regexStr   : regular expression
     * @param ignoreCase : whether to ignore case or not
     * @param split      : whether to return all splits (t) or only the first (f)
     */
    void match( std::vector< std::string >& list, std::string search, std::string regexStr, bool ignoreCase = false, bool split = false );
}
