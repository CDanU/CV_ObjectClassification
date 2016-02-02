﻿#pragma once

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
    std::string getCurrentPath();
    void        search( std::vector< std::string >& files, std::string path, std::string filter );
    void        match( std::vector< std::string >& list, std::string search, std::string regexStr, bool ignoreCase = false, bool split = false );
}
