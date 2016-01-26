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

#include "db.h"
#include "Console.h"
#include "Training.h"
#include "Classification.h"
#include "FeatureAverageColor.h"

#ifdef WIN32
    #include <Windows.h>
    #define REGISTER_SHUTDOWN_EVENT SetConsoleCtrlHandler( OnConsoleClose, TRUE );
    BOOL WINAPI OnConsoleClose( DWORD dwCtrlType ){ OnExit(); return FALSE; }
#else
    #define REGISTER_SHUTDOWN_EVENT
#endif


using FeatureList = std::vector < std::unique_ptr< Ue5::Feature > >;


const char * locale = setlocale( LC_ALL, "" ); // für das Anzeigen von UTF-8 zeichen in der Console
Ue5::DB db;
FeatureList featureList;
// Register Events by initializing this integer
const int _registerEvents = registerEvents();


/* ShutDown Event */
void OnExit()
{
    db.close(); // save & close db
    featureList.clear();
}

int registerEvents()
{
    REGISTER_SHUTDOWN_EVENT
    return 0;
}

void init()
{
    db.open( "storrage.json" );
    //db.clear(); // reset db

    featureList.push_back( std::unique_ptr< Ue5::Feature >( new Ue5::FeatureAverageColor() ) );
}

void training()
{
    std::cout << "Start Training...\n";

    Ue5::Training t( "images/50Objects/", featureList, db );

    t.start();
}

static void classify( std::string imagePath, std::string groupsConfigPath, std::vector< std::unique_ptr< Ue5::Feature > >& featureList )
{
    std::cout << "Start Classification...\n";

    Ue5::Classification c( featureList, groupsConfigPath );

    c.start( imagePath );
}


/* Main */
int main()
{
    try
    {
        init();
        // training();
        classify( "images/50Objects/apple1_4.JPG", "storrage.json", featureList );
        // console();
        // db_test( db );
    }
    catch( const std::exception& ex )
    {
        std::cout << ex.what();
        exit( -1 );
    }

    std::cin.ignore();

    return 0;
}
