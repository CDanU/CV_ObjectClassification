#pragma once

#include "FileSearch.h"

namespace Ue5
{
    std::vector< std::string > GetFeatureNames();

    void DoTraining( std::string workpath );
    void DoClassify( std::string imagePath );
    void ShowMatrix( std::string workpath );
    void ShowFeature( int index, std::string imgpath );
    void Init( std::string appPath );
    void Console();
}
