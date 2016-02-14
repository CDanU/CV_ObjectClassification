#include "Console.h"
#include "ConsoleFeatureFunctons.h"
#include "Classification.h"
#include "FeatureAverageColor.h"
#include "FeatureEdges.h"

#include <iostream>
#include <opencv2/highgui.hpp>

using FeatureList = std::vector< std::unique_ptr< Ue5::Feature > >;
Ue5::Classification * classification = NULL;
FeatureList featureList;

void initFeatures()
{
    featureList.push_back( std::unique_ptr< Ue5::Feature >( new Ue5::FeatureAverageColor() ) );
    featureList.push_back( std::unique_ptr< Ue5::Feature >( new Ue5::FeatureEdges() ) );
}

std::vector< std::string > Ue5::GetFeatureNames()
{
    std::vector< std::string > list;

    for( auto& f : featureList )
    {
        list.push_back( f->getFilterName() );
    }

    return list;
}

void Ue5::CleanUp()
{
    if( classification ) { delete classification; classification = NULL; }       // save & close db
    featureList.clear();
}

void Ue5::ShowFeature( int index, std::string imgpath )
{
    auto & feature = featureList[index];

    std::cout << "Apply feature '" << feature->getFilterName() << "' to " << imgpath << "..." << std::endl;

    feature->debugMode( true );
    feature->accumulate( cv::imread( imgpath ) );
    feature->debugMode( false );

    std::cout << "Feature Done." << std::endl;
}

void initClassify( std::string groupsConfigPath, std::vector< std::unique_ptr< Ue5::Feature > >& featureList )
{
    initFeatures();
    classification = new Ue5::Classification( featureList, groupsConfigPath );
}

void Ue5::Init( std::string appPath )
{
    std::cout << "Init...\n";

    initClassify( "storage.json", featureList );
    std::cout << "Init done.\n";
}

// classify single image
void Ue5::DoClassify( std::string imagePath )
{
    classification->classify( imagePath );
}

void Ue5::DoTraining( std::string workpath )
{
    std::cout << "Start Training...\n";

    classification->setPicturePath( workpath );
    classification->training();
    std::cout << "Training done.\n";
}

void Ue5::ShowMatrix( std::string workpath )
{
    std::cout << "Show Classification Matrix.\n";

    classification->setPicturePath( workpath );
    classification->showMatrix();
}
