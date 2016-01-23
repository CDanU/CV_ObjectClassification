#include <iostream>

#include "FeatureAverageColor.h"

using namespace cv;
using namespace std;

typedef cv::Point3_< uint8_t > Pixel;

namespace Ue5
{
// =============================================================================

    FeatureValue FeatureAverageColor::calculate( cv::InputArray _image, std::vector< uint > points )
    {
        if( points.empty() )
        {
            return calculate( _image );
        }
        // ---------------------------------------------------------------------

        auto image = _image.getMat();

        auto ret = FeatureValue();
        uint64 r, g, b, n;

        r = g = b = 0;
        n = points.size();

        for( uint idx : points )
        {
            int row = idx / image.cols;
            int col = idx - image.cols * row;

            Pixel * ptr = image.ptr< Pixel >( col, row );
            b += ptr->x;
            g += ptr->y;
            r += ptr->z;
        }

        ret.push_back( r / n );
        ret.push_back( g / n );
        ret.push_back( b / n );

        cout << "r:" << ret.at( 0 ) << ", g:" << ret.at( 1 ) << ", b:" << ret.at( 2 ) << endl;

        return ret;
    }

    FeatureValue FeatureAverageColor::calculate( cv::InputArray image )
    {
        auto ret = FeatureValue();

        Scalar_< double > avgColor = mean( image, noArray() );

        ret.push_back( avgColor[2] );
        ret.push_back( avgColor[1] );
        ret.push_back( avgColor[0] );

        cout << "r:" << ret.at( 0 ) << ", g:" << ret.at( 1 ) << ", b:" << ret.at( 2 ) << endl;

        return ret;
    }

// =============================================================================

    FeatureAverageColor::FeatureAverageColor(){}

    FeatureAverageColor::~FeatureAverageColor(){}
}
