#pragma once

#include "FileSearch.h"

namespace Ue5
{
    void DoTraining( std::string workpath );
    void DoClassify( std::string imagePath );
    void ShowMatrix( std::string workpath );
    void Init( std::string appPath );
    void Console();
}
