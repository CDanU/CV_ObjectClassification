#pragma once

#include "FeatureShiCorner.h"

#include <stdint.h>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <limits>

#include "ApplyFilter.h"


namespace Ue5
{
// =============================================================================
    using namespace cv;
    using namespace std;

    using Pixel1 = uint8_t;
    using Pixel3 = cv::Point3_< Pixel1 >;

// =============================================================================


    const int FeatureShiCorner::filter_derX[1][3] = {
        { 1, 0, -1 }
    };
    const int FeatureShiCorner::filter_derY[3][1] = {
        { 1  },
        { 0  },
        { -1 }
    };
    const int FeatureShiCorner::filter_gauss[3][3] = {
        { 1, 2, 1 },
        { 2, 4, 2 },
        { 1, 2, 1 }
    };

// =============================================================================

    FeatureValue FeatureShiCorner::genDescriptor( InputArray _image, vector< pair< int, int > > points )
    {
        //TODO: finish

        auto out = FeatureValue();

        for( auto point : points )
        {
            // every kSize x kSize a new feature
            const int kSize   = descRadius * 2 + 1;
            const auto image  = _image.getMat();
            const auto height = image.rows;
            const auto width  = image.cols;
            auto x = point.first;
            auto y = point.second;

            for( int fy = -kSize; fy <= kSize; fy++ )
            {
                for( int fx = -kSize; fx <= kSize; fx++ )
                {
                    int xD = (x + fx);
                    int yD = (y + fy);

                    // not really needed since genPoints removes corners too
                    // to close to the image edges
                    if( (xD < 0) || (xD >= width) || (yD < 0) || (yD >= height) )
                    {
                        out.push_back( 0 );
                    }
                    else
                    {
                        out.push_back( (double) image.at< int32_t >( yD, xD ) );
                    }
                }
            }
        }

        return out;
    }

    vector< pair< int, int > > FeatureShiCorner::genPoints( InputArray _image )
    {
        auto outVec      = vector< pair< int, int > >();
        const Mat image  = _image.getMat();
        const int height = image.rows - descRadius;
        const int width  = image.cols - descRadius;
        int32_t val;

        for( int y = descRadius; y < height; y++ )
        {
            for( int x = descRadius; x < width; x++ )
            {
                val = image.at< int32_t >( y, x );
                if( val <= 0 ) { continue; }

                outVec.push_back( make_pair( x, y ) );
            }
        }

        return outVec;
    }

    void FeatureShiCorner::computeRawCornerMat( InputArray _image, OutputArray _corner )
    {
        // TODO check: _corner must be CV_32SC1

        const Mat image  = _image.getMat();
        const int height = image.rows;
        const int width  = image.cols;
        const int radius = 1;
        Mat derX( height, width, CV_32SC1, Scalar( 0 ) );
        Mat derY( height, width, CV_32SC1, Scalar( 0 ) );
        Mat Mx2( height, width, CV_32SC1, Scalar( 0 ) );
        Mat My2( height, width, CV_32SC1, Scalar( 0 ) );
        Mat Mxy( height, width, CV_32SC1, Scalar( 0 ) );


        applyFilter< uchar, int32_t >( _image, derX, &filter_derX[0][0], 3, 1, 0, true );
        applyFilter< uchar, int32_t >( _image, derY, &filter_derY[0][0], 1, 3, 0, true );


        int normDivisor     = 0;
        const int * pGauss  = &FeatureShiCorner::filter_gauss[0][0];
        int const * pGaussE = pGauss + 9;
        for(; pGauss != pGaussE; pGauss++ )
        {
            normDivisor += abs( *pGauss );
        }

        int32_t maxVal = 0;
        for( int y = 0; y < height; y++ )
        {
            for( int x = 0; x < width; x++ )
            {
                for( int dy = -radius; dy <= radius; dy++ )
                {
                    for( int dx = -radius; dx <= radius; dx++ )
                    {
                        int fx = x + dx;
                        if( (fx < 0) || (fx >= width) ) { continue; }
                        int fy = y + dy;
                        if( (fy < 0) || (fy >= height) ) { continue; }

                        int f = FeatureShiCorner::filter_gauss[(radius + dx)][(radius + dy)];

                        Mx2.at< int32_t >( y, x ) += f * pow( derX.at< int32_t >( fy, fx ), 2 );
                        My2.at< int32_t >( y, x ) += f * pow( derY.at< int32_t >( fy, fx ), 2 );
                        Mxy.at< int32_t >( y, x ) += f * derX.at< int32_t >( fy, fx ) * derY.at< int >( fy, fx );
                    }
                }

                Mx2.at< int32_t >( y, x ) /= normDivisor;
                My2.at< int32_t >( y, x ) /= normDivisor;
                Mxy.at< int32_t >( y, x ) /= normDivisor;
                maxVal = max( Mx2.at< int32_t >( y, x ), maxVal );
                maxVal = max( My2.at< int32_t >( y, x ), maxVal );
                maxVal = max( Mxy.at< int32_t >( y, x ), maxVal );
            }
        }

        Mat corners      = _corner.getMat();
        const auto it_cE = corners.end< int32_t >();
        auto it_cS       = corners.begin< int32_t >();
        auto it_Mx2S     = Mx2.begin< int32_t >();
        auto it_My2S     = My2.begin< int32_t >();
        auto it_MxyS     = Mxy.begin< int32_t >();

        // reduce to high values if necessary
        // maxval: 0..1 * 255^2, maxval^2 should not overflow for the next step
        // reduce to sqrt( 2^31-1 (signed int)  ) -> 46340
        const int maxValC = 46340;
        if( maxVal > maxValC )
        {
            cout << "maxVal > maxValC | maxVal: " << maxVal << endl;

            const double scaleFac = maxValC / (double) maxVal; // scaleFac = 0.xxxx

            while( it_cS != it_cE )
            {
                *it_cS   *= scaleFac;
                *it_Mx2S *= scaleFac;
                *it_My2S *= scaleFac;
                *it_MxyS *= scaleFac;

                it_cS++;
                it_Mx2S++;
                it_My2S++;
                it_MxyS++;
            }

            // reset iterators
            it_cS   = corners.begin< int32_t >();
            it_Mx2S = Mx2.begin< int32_t >();
            it_My2S = My2.begin< int32_t >();
            it_MxyS = Mxy.begin< int32_t >();
        }

        maxVal = 0;

        // calc eigenvalues
        int32_t trc, det;
        double ev_sqrt, trc_halve, eigVal1, eigVal2;
        while( it_cS != it_cE )
        {
            trc = *it_Mx2S + *it_My2S;
            det = *it_Mx2S * *it_My2S - *it_MxyS * *it_MxyS;

            ev_sqrt   = sqrt( ( (trc * trc) / 4 ) - det );
            trc_halve = trc / 2.0;

            eigVal1 = trc_halve + ev_sqrt;
            eigVal2 = trc_halve - ev_sqrt;

            if( (eigVal1 < 0) || (eigVal2 < 0) )
            {
                eigVal1 = 0;
                eigVal2 = 0;
            }

            *it_cS = (int32_t) min( eigVal1, eigVal2 );
            maxVal = max( (int32_t) min( eigVal1, eigVal2 ), maxVal );

            it_cS++;
            it_Mx2S++;
            it_My2S++;
            it_MxyS++;
        }

        // *
        if( maxVal != 0 )
        {
            const int threshold = maxVal * 0.2;
            for( auto it_cE = corners.end< int32_t >(), it_cS = corners.begin< int32_t >(); it_cS != it_cE; it_cS++ )
            {
                if( *it_cS < threshold ) { *it_cS = 0; }
            }
        }
        // */

        // *
        Mat cornersFiltered( height, width, CV_32SC1 );
        maxFilter< int32_t >( corners, cornersFiltered, 5, 5 );
        // */

        if( isDebugMode )
        {
            Mat derXd, derYd, cornersd;
            cornersFiltered.convertTo( cornersd, CV_8UC1 );
            derX.convertTo( derXd, CV_8UC1 );
            derY.convertTo( derYd, CV_8UC1 );

            // Display corners over the image (cross)
            Mat cornersdc = image.clone();

            auto cornerPoints = genPoints( cornersFiltered );

            for( auto p : cornerPoints )
            {
                for( int dx = -2; dx <= 2; dx++ )
                {
                    int x = p.first + dx;
                    int y = p.second;
                    if( ( x < 0) || ( x >= width) ) { continue; }
                    cornersdc.at< uchar >( y, x ) = 0;
                }

                for( int dy = -2; dy <= 2; dy++ )
                {
                    int x = p.first;
                    int y = p.second + dy;
                    if( ( y < 0) || ( y >= height) ) { continue; }

                    cornersdc.at< uchar >( y, x ) = 0;
                }
            }

            imshow( "image", image );
            imshow( "derX", derXd );
            imshow( "derY", derYd );
            imshow( "Shi Corner", cornersd );
            imshow( "Shi Corner Image", cornersdc );

            waitKey( 0 );
            destroyAllWindows();
            waitKey( 1 );
        }
    }

// =============================================================================

    double FeatureShiCorner::compare( FeatureValue a, FeatureValue b )
    {
        return 0;
    }

    std::string FeatureShiCorner::getFilterName(){ return "ShiCorner"; }

    FeatureValue FeatureShiCorner::getNormedAccumulate()
    {
        return FeatureValue();
    }

    FeatureValue FeatureShiCorner::calculate( cv::InputArray image )
    {
        Mat image_gray;

        cvtColor( image, image_gray, CV_BGR2GRAY );

        Mat corner( image.rows(), image.cols(), CV_32SC1, Scalar( 0 ) );
        computeRawCornerMat( image_gray, corner );
        auto points = genPoints( corner );


        // return genDescriptor;
        return FeatureValue();
    }

    void FeatureShiCorner::accumulate( cv::InputArray image )
    {
        calculate( image );
    }

    void FeatureShiCorner::clearAccu(){}

// =============================================================================

    FeatureShiCorner::FeatureShiCorner(){}

    FeatureShiCorner::~FeatureShiCorner(){}

// =============================================================================

    Feature::FeatureType FeatureShiCorner::getFeatureType()
    {
        return Feature::FeatureType::Simple;
    }

    double FeatureShiCorner::compare( FeatureMat grp, cv::InputArray image )
    {
        cerr << "compare( FeatureMat grp, cv::InputArray image ): Not supported for simple Features!" << endl;
        return 0;
    }

    FeatureMat FeatureShiCorner::getNormedAccumulateMat()
    {
        cerr << "getNormedAccumulateMat(): Not supported for simple Features" << endl;
        return FeatureMat();
    }
}
