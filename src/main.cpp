// ============================================================================
// Name        : CV_ObjectClassification
// Author      : Daniel Chumak, Mario Link
// Version     : 0.3.0
// Copyright   : HtwDanielMario@2016
// Description : This Application ... TEXT ...
// ============================================================================

#include "db.h"
#include "Training.h"
#include "FeatureAverageColor.h"

#include <iostream>
#include "Console.h"

void OnExit();

#ifdef WIN32
    #include <Windows.h>
    #define REGISTER_SHUTDOWN_EVENT SetConsoleCtrlHandler( OnConsoleClose, TRUE );
    BOOL WINAPI OnConsoleClose( DWORD dwCtrlType ){ OnExit(); return FALSE; }
#else
    #define REGISTER_SHUTDOWN_EVENT
#endif

const char * locale = setlocale( LC_ALL, "" ); // für das Anzeigen von UTF-8 zeichen in der Console
Ue5::DB db;
std::vector< std::unique_ptr< Ue5::Feature > > featureList;

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

// Register Events by initalize this integer
const int _registerEvents = registerEvents();

void init()
{
    db.open( "storrage.json" );
    db.clear(); // reset db

    featureList.push_back( std::unique_ptr< Ue5::Feature >( new Ue5::FeatureAverageColor() ) );
}

void training()
{
    std::cout << "Start Training...\n";

    Ue5::Training t( "../../50Objects/", featureList, db );

    t.start();
}

/* Main */
int main()
{
    try
    {
        init();
        training();
        // console();
        // db_test( db );
    }
    catch( const std::exception& ex )
    {
        std::cout << ex.what();
        std::cin.ignore();
    }

    return 0;
}
