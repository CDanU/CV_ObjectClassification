#include <stdint.h>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "FeatureAverageColor.h"
#include "FeatureShiCorner.h"


using namespace cv;
using namespace std;

using Pixel = cv::Point3_< uint8_t >;

namespace Ue5
{
// =============================================================================

    void FeatureAverageColor::rgb2lab( const double _RGB[3], double lab[3] )
    {
        // http://www.f4.fhtw-berlin.de/~barthel/ImageJ/ColorInspector/HTMLHelp/farbraumJava.htm
        const double xyzRef[3] = { 0.964221, 1.0, 0.825211 };  // reference white D50
        const double eps       = 216 / 24389.0;
        const double k         = 24389 / 27.0;
        double rgb[3], xyz[3];

        // RGB to XYZ
        rgb[0] = _RGB[0] / 255.0;
        rgb[1] = _RGB[1] / 255.0;
        rgb[2] = _RGB[2] / 255.0;

        // assuming sRGB (D65)
        for( int i = 0; i < 3; i++ )
        {
            if( rgb[i] <= 0.04045 ) { rgb[i] /= 12; }
            else { rgb[i] = pow( ( rgb[i] + 0.055) / 1.055, 2.4 ); }
        }

        xyz[0] = (0.436052025 * rgb[0] + 0.385081593 * rgb[1] + 0.143087414 * rgb[2]) / xyzRef[0];
        xyz[1] = (0.222491598 * rgb[0] + 0.716886060 * rgb[1] + 0.060621486 * rgb[2]) / xyzRef[1];
        xyz[2] = (0.013929122 * rgb[0] + 0.097097002 * rgb[1] + 0.714185470 * rgb[2]) / xyzRef[2];

        // XYZ to Lab
        for( int i = 0; i < 3; i++ )
        {
            if( xyz[i] > eps ) { xyz[i] = pow( xyz[i], 0.333333333 ); }
            else { xyz[i] = ( k * xyz[i] + 16 ) / 116.0; }
        }

        lab[0] = ( 116 * xyz[1] ) - 16;
        lab[1] = 500 * (xyz[0] - xyz[1]);
        lab[2] = 200 * (xyz[1] - xyz[2]);

        lab[0] = (int) ( 2.55 * lab[0] + 0.5);
        lab[1] = (int) ( lab[1] + 0.5 );
        lab[2] = (int) ( lab[2] + 0.5 );
    }

    string FeatureAverageColor::getFilterName(){ return "AverageColor"; }

    double FeatureAverageColor::compare( FeatureValue a, FeatureValue b )
    {
        if( (a.size() != 3) || (b.size() != 3) )
        {
            cerr << "Wrong amount of parameters: should be 3, is: a(" << a.size() << ") | b(" << b.size() << ")" << endl;
            return 0;
        }
        double aLAB[3], bLAB[3];

        rgb2lab( a.data(), aLAB );
        rgb2lab( b.data(), bLAB );

        return 1 - sqrt( pow( aLAB[0] - bLAB[0], 2 ) + pow( aLAB[1] - bLAB[1], 2 ) + pow( aLAB[2] - bLAB[2], 2 ) ) / 255.0;
    }

    // *
    FeatureValue FeatureAverageColor::calculate( cv::InputArray _image )
    {
        Mat image_gray;
        Mat shiImg( _image.rows(), _image.cols(), CV_32SC1, Scalar( 0 ) );

        cvtColor( _image, image_gray, CV_BGR2GRAY );

        FeatureShiCorner shi;
        shi.computeRawCornerMat( image_gray, shiImg );
        auto points = shi.genPoints( shiImg );


        if( points.empty() )
        {
            return {
                       0, 0, 0
            };
        }
        // ---------------------------------------------------------------------

        auto image = _image.getMat();

        auto ret = FeatureValue();
        uint64 r, g, b, n;

        const int radius = 15;

        r = g = b = 0;

        n = 0;

        const auto width  = _image.cols();
        const auto height = _image.rows();


        for( auto p : points )
        {
            int x = p.first, y = p.second;

            for( int dy = -radius; dy <= radius; dy++ )
            {
                for( int dx = -radius; dx <= radius; dx++ )
                {
                    int fx = x + dx;
                    if( (fx < 0) || (fx >= width) ) { continue; }
                    int fy = y + dy;
                    if( (fy < 0) || (fy >= height) ) { continue; }

                    Pixel ptr = image.at< Pixel >( fy, fx );
                    b += ptr.x;
                    g += ptr.y;
                    r += ptr.z;

                    n++;
                }
            }
        }

        ret.push_back( r / (n * 1.0) );
        ret.push_back( g / (n * 1.0) );
        ret.push_back( b / (n * 1.0) );

        return ret;
    }
    // */

    FeatureValue FeatureAverageColor::calculateP( cv::InputArray image )
    {
        auto ret = FeatureValue();

        Scalar_< double > avgColor = mean( image, noArray() );

        ret.push_back( avgColor[2] );
        ret.push_back( avgColor[1] );
        ret.push_back( avgColor[0] );

        // cout << "r:" << ret.at( 0 ) << ", g:" << ret.at( 1 ) << ", b:" << ret.at( 2 ) << endl;

        return ret;
    }

    void FeatureAverageColor::accumulate( cv::InputArray image )
    {
        FeatureValue result = FeatureAverageColor::calculate( image );

        accu[0] += result[0];
        accu[1] += result[1];
        accu[2] += result[2];
        count++;
    }

    FeatureValue FeatureAverageColor::getNormedAccumulate()
    {
        auto ret = FeatureValue();

        ret.push_back( accu[0] / count );
        ret.push_back( accu[1] / count );
        ret.push_back( accu[2] / count );

        return ret;
    }

    void FeatureAverageColor::clearAccu()
    {
        accu[0] = accu[1] = accu[2] = 0;
        count   = 0;
    }

// =============================================================================

    FeatureAverageColor::FeatureAverageColor()
    {
        clearAccu();
    }

    FeatureAverageColor::~FeatureAverageColor(){}

// =============================================================================

    Feature::FeatureType FeatureAverageColor::getFeatureType()
    {
        return Feature::FeatureType::Simple;
    }
}
