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

    string FeatureAverageColor::getFilterName(){ return "AverageColor"; }

    double FeatureAverageColor::compare( FeatureValue a, FeatureValue b )
    {
        double ret = 0;

        if( (a.size() != 3) || (b.size() != 3) )
        {
            cerr << "Wrong amount of parameter: should be 3, is: a(" << a.size() << ") | b(" << b.size() << ")" << endl;
            return 0;
        }

        ret += 1 - abs( a[0] - b[0] ) / 255;
        ret += 1 - abs( a[1] - b[1] ) / 255;
        ret += 1 - abs( a[2] - b[2] ) / 255;

        return ret / 3;
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

        r = g = b = 0;
        n = points.size();

        for( auto p : points )
        {
            Pixel * ptr = image.ptr< Pixel >( p.second, p.first );
            b += ptr->x;
            g += ptr->y;
            r += ptr->z;
        }

        ret.push_back( r / (n * 1.0) );
        ret.push_back( g / (n * 1.0) );
        ret.push_back( b / (n * 1.0) );

        // cout << "r:" << ret.at( 0 ) << ", g:" << ret.at( 1 ) << ", b:" << ret.at( 2 ) << endl;


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

    double FeatureAverageColor::compare( FeatureMat grp, cv::InputArray image )
    {
        cerr << "compare( FeatureMat grp, cv::InputArray image ): Not supported for simple Features!" << endl;
        return 0;
    }

    FeatureMat FeatureAverageColor::getNormedAccumulateMat()
    {
        cerr << "getNormedAccumulateMat(): Not supported for simple Features" << endl;
        return FeatureMat();
    }
}
