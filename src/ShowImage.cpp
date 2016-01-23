#include "ShowImage.h"

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

void Ue5::showImage( std::string file )
{
    cv::Mat image = cv::imread( file, CV_LOAD_IMAGE_COLOR ); // Read the file

    if( !image.data )       // Check for invalid input
    {
        std::cerr << "Could not open or find the image" << std::endl;
        return;
    }

    cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE ); // Create a window for display.
    imshow( "Display window", image );     // Show our image inside it.

    cv::waitKey( 0 );     // Wait for a keystroke in the window
}
