// ============================================================================
// Name        : CV_Ue05_Chumak_Link.cpp
// Author      : Daniel Chumak, Mario Link
// Version     :
// Copyright   : Your copyright notice
// Description : ...
// ============================================================================

#include <iostream>
#include <string.h>
#include <dirent.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

// ------------------------------------------------------------------------------
#define SSTR( x ) dynamic_cast< std::ostringstream& >( ( std::ostringstream() << std::dec << x ) ).str()
// ------------------------------------------------------------------------------

using namespace cv;
using namespace std;


int main()
{
    Mat input = imread( "images/50Objects/book_3.JPG", CV_LOAD_IMAGE_COLOR );

    Scalar_< int > avgColor = mean( input, noArray() );
    Scalar_< int > compare( 20, 30, 40, 0 );

    double colorDistance = norm( avgColor, compare, NORM_L2 );

    cout << avgColor << endl;
    cout << compare << endl;
    cout << colorDistance << endl;

    return ( 0);
}
