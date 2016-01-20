// ============================================================================
// Name        : CV_ObjectClassification
// Author      : Daniel Chumak, Mario Link
// Version     : 0.3.0
// Copyright   : HtwDanielMario@2016
// Description : This Application ... TEXT ...
// ============================================================================

#include "includes.h"
#include "fileSearch.h"
#include "showImage.h"
#include "db.h"
#include "console.h"

const char * locale = setlocale( LC_ALL, "" ); // für das Anzeigen von UTF-8 zeichen in der Console
DB db;

/* ShutDown Event */
void OnExit()
{
    db.close(); // save & close db
}

int registerEvents()
{
    REGISTER_SHUTDOWN_EVENT
    return 0;
}

// Register Events by initalize this integer
const int _registerEvents = registerEvents();

/* Main */
int main()
{
    db_test( db );
    // console();
    return 0;
}
