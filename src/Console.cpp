#define NOMINMAX

#include "Console.h"

#include <iostream>
#include <sstream>
#include <functional>
#include <map>
#include <algorithm>
#include <cctype>

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

    const int inputLength = 256;
    string input;
    char cinput[inputLength];
    string apppath = getCurrentPath() + "images/50Objects/";
    string imgpath = apppath;

    Init( apppath );

    enum ReturnCode
    {
        Nothing,
        Valid,
        Error,
        Quit
    };

    auto searchFiles = [&](){
                           if( !files.empty() ) { files.clear(); }
                           Ue5::search( files, imgpath, ".+[.]((jpe?g)|(png)|(bmp))$" );
                       };

    auto listFiles = [&]( string input ){
                         searchFiles();

                         cout << "List Files:\n";
                         unsigned int count = 0;

                         for( auto& f : files )
                         {
                             cout << count << ": " << f << endl;
                             count++;
                         }

                         return Valid;
                     };

    auto quit = [&]( string input ){ return Quit; };

    auto is_number = [&]( const std::string& s ){
                         return !s.empty() && std::find_if( s.begin(), s.end(), []( char c ){ return !std::isdigit( c ); } ) == s.end();
                     };

    auto help = [&]( string input ){
                    cout << endl << "[ Commands ]" << endl << endl
                         << "(h)elp            : this info" << endl
                         << "path              : shows current image directory" << endl
                         << "cd ('path')       : sets image directory ('#reset' resets to default)" << endl
                         << "ls, list          : list images in current image directory" << endl
                         << "show (0..9..)     : displays an image given in list" << endl
                         << "train             : start Training for Classification" << endl
                         << "classify (0..9..) : start Classification for an image given in list" << endl
                         << "matrix            : start Classification Matrix" << endl
                         << "(q)uit, exit      : exit program" << endl;
                    return Valid;
                };

    map< string, function< ReturnCode( string ) > > commands {
        {
            "q", quit
        },
        {
            "quit", quit
        },
        {
            "exit", quit
        },
        {
            "help", help
        },
        {
            "h", help
        },
        {
            "path", [&]( string input ){
                cout << "ImagePath:" << endl << imgpath << endl;
                return Valid;
            }
        },
        {
            "cd", [&]( string input ){
                auto size   = string( "cd" ).size() + 1;
                string path = input.size() > size ? input.substr( size ) : "";

                if( path == "" ) { cout << "No path given." << endl; return Error; }
                else
                {
                    if( path == "#reset" ) { imgpath = apppath; }
                    else if( path.find( "../" ) != string::npos )
                    {
                        imgpath += path;
                    }
                    else { imgpath = path; }

                    cout << "ImagePath:" << endl << imgpath << endl;
                }

                return Valid;
            }
        },
        {
            "list", listFiles
        },
        {
            "ls", listFiles
        },
        {
            "show", [&]( string input ){
                if( files.empty() ) { searchFiles(); }

                auto size     = string( "show" ).size() + 1;
                int index     = 0;
                auto maxIndex = !files.empty() ? files.size() - 1 : 0;

                string numbStr = input.size() > size ? input.substr( size ) : "";
                bool error     = ( !numbStr.size() || numbStr.size() > to_string( maxIndex ).size() || !is_number( numbStr ) );

                if( !error )
                {
                    istringstream( numbStr ) >> index;

                    if( !files.empty() && (index >= 0) && ( index < files.size() ) )
                    {
                        showImage( imgpath + files[index] );
                        return Valid;
                    }
                }

                cout << "Invalid Index or no files in list! (Max:" << maxIndex << ")" << endl;
                return Error;
            }
        },
        {
            "train", [&]( string input ){ DoTraining( imgpath ); return Valid; }
        },
        {
            "classify", [&]( string input ){
                auto size      = string( "classify" ).size() + 1;
                int index      = 0;
                string numbStr = input.size() > size ? input.substr( size ) : "";

                if( numbStr != "" ) { istringstream( numbStr ) >> index; }

                auto maxIndex = !files.empty() ? files.size() - 1 : 0;

                if( !files.empty() && (index >= 0) && ( index < files.size() ) )
                {
                    DoClassify( imgpath + files[index] );
                }
                else { cout << "Invalid Index or no files in list! (Max:" << maxIndex << ")" << endl; }

                return Valid;
            }
        },
        {
            "matrix", [&]( string input ){
                return Nothing;
            }
        }
    };

    char clr[100];
    while( true )
    {
        input = "";
        cout << endl << "> ";
        cin.getline( cinput, sizeof(cinput) );

        // BUGFIX : fixes problem with cin with greater length than 'inputLength' (256) chars
        if( cin.fail() )
        {
            cin.clear();
            // fflush(stdin);
            cin.ignore( numeric_limits< streamsize >::max(), '\n' );
        }

        input     = cinput;
        cinput[0] = '\0';

        auto ret       = Nothing;
        auto findSpace = input.find( " " );
        auto cmd       = findSpace != string::npos ? input.substr( 0, findSpace ) : input;

        auto it = commands.find( cmd );
        if( it != commands.end() ) { ret = it->second( input ); }

        if( ret == Quit ) { break; }
        else if( ret == Error )
        {
            cout << "Command Error" << endl;
        }
        else if( ret == Nothing )
        {
            cout << "Unknown Command" << endl;
        }
    }
}
