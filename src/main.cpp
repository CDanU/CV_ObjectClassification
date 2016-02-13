// =============================================================================
// Name        : CV_ObjectClassification
// Author      : Daniel Chumak, Mario Link
// Version     : 1.0.0
// Copyright   : HtwDanielMario@2016
// Description : App for Object Classification
// =============================================================================


void OnExit();
int  registerEvents();

#include <iostream>

#include "Console.h"

#ifdef WIN32
    #include <Windows.h>
    #define REGISTER_SHUTDOWN_EVENT SetConsoleCtrlHandler( OnConsoleClose, TRUE );
BOOL WINAPI OnConsoleClose( DWORD dwCtrlType ){ OnExit(); return FALSE; }
#else
    #define REGISTER_SHUTDOWN_EVENT
#endif

const char * locale = setlocale( LC_ALL, "" ); // für das Anzeigen von UTF-8 zeichen in der Console

// Register Events by initializing this integer
const int _registerEvents = registerEvents();

/* ShutDown Event */
void OnExit(){ Ue5::CloseConsole(); }

int registerEvents()
{
    REGISTER_SHUTDOWN_EVENT
    return 0;
}

/* Main */
int main()
{
    try
    {
        Ue5::OpenConsole();
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
