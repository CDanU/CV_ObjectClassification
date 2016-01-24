#include "includes.h"

FileStorage writeDB;
FileStorage readDB;
std::ifstream dbstream;
std::string dbfile;

void closeDB()
{
    if( !readDB.isOpened() ) { return; }
    readDB.release();
    // while (dbstream.is_open());
}

void openDB( int flags = FileStorage::READ )
{
    // if (writeDB.isOpened()) return;
    // writeDB.open(dbfile, FileStorage::READ); // schreiben und lesen danach geht wohl nicht?
}

template< typename T >
void readFromDB( string key, T& var )
{
    if( !readDB.isOpened() ) { return; }
    readDB[key] >> var;
}

void createDB( string file )
{
    dbfile = file;
    // dbstream = ifstream(file);

    writeDB.open( dbfile, FileStorage::WRITE );
    if( !writeDB.isOpened() ) { return; }

    writeDB << "Name" << "CV_ObjectClassification DB";

    // alternative solution for numbers
    cvWriteInt( *writeDB, "frameCount", 5 );   // !!! BUG : fs << "nameA"<<5; fs << "nameB"<<"a" => nameA:5; nameA:a (windows;visual studio) !!!

    time_t rawtime;
    time( &rawtime );
    writeDB << "calibrationDate" << asctime( localtime( &rawtime ) );
    cv::Mat cameraMatrix = ( cv::Mat_< double >( 3, 3 ) << 1000, 0, 320, 0, 1000, 240, 0, 0, 1);
    cv::Mat distCoeffs   = ( cv::Mat_< double >( 5, 1 ) << 0.1, 0.01, -0.001, 0, 0);
    writeDB << "cameraMatrix" << cameraMatrix << "distCoeffs" << distCoeffs;
    writeDB << "features" << "[";
    for( int i = 0; i < 3; i++ )
    {
        int x     = rand() % 640;
        int y     = rand() % 480;
        uchar lbp = rand() % 256;

        writeDB << "{:" << "x" << x << "y" << y << "lbp" << "[:";
        for( int j = 0; j < 8; j++ )
        {
            writeDB << ( (lbp >> j) & 1 );
        }

        writeDB << "]" << "}";
    }

    writeDB << ']';

    writeDB.release();
}
