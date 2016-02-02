#include "Console.h"

#include <iostream>
#include <sstream>

#include "ShowImage.h"

using namespace std;

void Ue5::Console()
{
    cout <<
        "##############################################\n" <<
        "# Name        : CV_ObjectClassification\n" <<
        "# Author      : Daniel Chumak, Mario Link\n" <<
        "# Version     : 0.1.0\n" <<
        "# Copyright   : HtwDanielMario@2016\n" <<
        "# Description : This Application ... TEXT ..\n" <<
        "##############################################\n";

    vector< string > files;
    string cmd;
    char input[100];
    string apppath = getCurrentPath() + "images/50Objects/";
    string imgpath = apppath;

    Init( apppath );

    while( true )
    {
        cout << "\n> ";
        cin.getline( input, sizeof(input) );
        cmd = input;

        if( cmd == "help" )
        {
            cout << endl << "[ Commands ]" << endl << endl <<
                "help              : this info" << endl <<
                "dir               : shows current working directory" << endl <<
                "cd ('path')       : sets working directory ('#reset' resets to default)" << endl <<
                "ls, list          : list images in current working directory" << endl <<
                "show (0..9..)     : displays an image given in list" << endl <<
                "training          : start Training for Classification" << endl <<
                "classify (0..9..) : start Classification for an image given in list" << endl <<
                "(q)uit, exit      : exit program" << endl;
        }
        else if( cmd == "dir" )
        {
            cout << "WorkPath:" << endl << imgpath << endl;
        }
        else if( cmd.find( "cd" ) == 0 )
        {
            string path = cmd.size() > 3 ? cmd.substr( 3 ) : "";

            if( path == "" ) { cout << "No path given." << endl; }
            else
            {
                if( path == "#reset" ) { imgpath = apppath; }
                else if( path.find( "../" ) != string::npos )
                {
                    imgpath += path;
                }
                else { imgpath = path; }

                cout << "WorkPath:" << endl << imgpath << endl;
            }
        }
        else if( ( cmd == "ls") || ( cmd == "list") )
        {
            if( !files.empty() ) { files.clear(); }
            Ue5::fileSearch( files, imgpath, "*.jpg" );

            cout << "List Files:\n";
            unsigned int count = 0;

            for( auto& f : files )
            {
                cout << count << ": " << f << endl;
                count++;
            }
        }
        else if( cmd.find( "show" ) == 0 )
        {
            int index = 0;

            string numbStr = cmd.size() > 5 ? cmd.substr( 5 ) : "";
            if( numbStr != "" ) { istringstream( numbStr ) >> index; }

            auto maxIndex = !files.empty() ? files.size() - 1 : 0;

            if( !files.empty() && (index >= 0) && ( index < files.size() ) )
            {
                showImage( imgpath + files[index] );
            }
            else { cout << "Invalid Index or no files in list! (Max:" << maxIndex << ")" << endl; }
        }
        else if( cmd == "training" )
        {
            DoTraining( imgpath );
        }
        else if( cmd.find( "classify" ) == 0 )
        {
            int index      = 0;
            string numbStr = cmd.size() > 5 ? cmd.substr( 5 ) : "";
            if( numbStr != "" ) { istringstream( numbStr ) >> index; }

            auto maxIndex = !files.empty() ? files.size() - 1 : 0;

            if( !files.empty() && (index >= 0) && ( index < files.size() ) )
            {
                DoClassify( imgpath + files[index] );
            }
            else { cout << "Invalid Index or no files in list! (Max:" << maxIndex << ")" << endl; }
        }
        else if( (cmd == "q") || (cmd == "quit") || (cmd == "exit") )
        {
            break;
        }
        else { cout << "Unknown Command" << endl; }
        cmd = "";
    }
}
