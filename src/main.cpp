// =============================================================================
// Name        : CV_ObjectClassification
// Author      : Daniel Chumak, Mario Link
// Version     : 0.3.0
// Copyright   : HtwDanielMario@2016
// Description : This Application ... TEXT ...
// =============================================================================


void OnExit();
int  registerEvents();

#include <iostream>

#include "Console.h"
#include "Classification.h"
#include "FeatureAverageColor.h"
#include "FeatureEdges.h"

#ifdef WIN32
    #include <Windows.h>
    #define REGISTER_SHUTDOWN_EVENT SetConsoleCtrlHandler( OnConsoleClose, TRUE );
BOOL WINAPI OnConsoleClose( DWORD dwCtrlType ){ OnExit(); return FALSE; }
#else
    #define REGISTER_SHUTDOWN_EVENT
#endif

using FeatureList = std::vector< std::unique_ptr< Ue5::Feature > >;

const char * locale = setlocale( LC_ALL, "" ); // für das Anzeigen von UTF-8 zeichen in der Console
Ue5::DB db;
Ue5::Classification * classification = NULL;
FeatureList featureList;
// Register Events by initializing this integer
const int _registerEvents = registerEvents();


/* ShutDown Event */
void OnExit()
{
    if( classification ) { delete classification; classification = NULL; }    // save & close db
    featureList.clear();
}

int registerEvents()
{
    REGISTER_SHUTDOWN_EVENT
    return 0;
}

void initFeatures()
{
    featureList.push_back( std::unique_ptr< Ue5::Feature >( new Ue5::FeatureAverageColor() ) );
    featureList.push_back( std::unique_ptr< Ue5::Feature >( new Ue5::FeatureEdges() ) );
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
    std::cout << "Start Classification...\n";

    classification->start( imagePath );
    std::cout << "Classification done.\n";
}

void Ue5::DoTraining( std::string workpath )
{
    std::cout << "Start Training...\n";

    classification->training( workpath );
    std::cout << "Training done.\n";
}

/* Main */
int main()
{
    try
    {
        Ue5::Console();
    }
    catch( const std::exception& ex )
    {
        std::cout << ex.what();
        std::cout << std::endl << "Press any key to exit." << std::endl;
        std::cin.ignore();
        exit( -1 );
    }

    return 0;
}
