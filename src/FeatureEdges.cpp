#pragma once

#include <stdint.h>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>


#include "FeatureEdges.h"
#include "ApplyFilter.h"

namespace Ue5
{
    using namespace cv;
    using namespace std;

    using Pixel1 = uint8_t;
    using Pixel3 = cv::Point3_< Pixel1 >;

// =============================================================================

    const int FeatureEdges::filter_sobelX1D0[3][1] = {
        { 1 },
        { 2 },
        { 1 }
    };

    const int FeatureEdges::filter_sobelX1D1[1][3] = {
        { 1, 0, -1 }
    };


    const int FeatureEdges::filter_sobelY1D0[3][1] = {
        { 1  },
        { 0  },
        { -1 }
    };

    const int FeatureEdges::filter_sobelY1D1[1][3] = {
        { 1, 2, 1 }
    };


    const int FeatureEdges::filter_gauss1D0[3][1] = {
        { 1 },
        { 2 },
        { 1 }
    };

    const int FeatureEdges::filter_gauss1D1[1][3] = {
        { 1, 2, 1 }
    };

// =============================================================================
    uchar ISOData( InputArray pixels )
    {
        // TODO: checks

        auto img = pixels.getMat();

        uchar histo[256] = { 0 };
        int gray;
        uchar histoStart = 0;
        uchar histoEnd   = 255;
        uchar pivot      = 0;

        // create histogram & search first non zero values from both sides
        for( auto itS = img.begin< uchar >(), itE = img.end< uchar >(); itS != itE; itS++ )
        {
            histo[*itS]++;
        }

        bool histoStartFound = false;
        bool histoEndFound   = false;

        for( uchar i = 0, j = 255; i < 256; i++, j-- )
        {
            if( !histoStartFound && ( histo[i] != 0) )
            {
                histoStart      = i;
                histoStartFound = true;
            }

            if( !histoEndFound && ( histo[j] != 0) )
            {
                histoEnd      = j;
                histoEndFound = true;
            }

            if( histoStartFound && histoEndFound ) { break; }
        }

        // speedup
        // if only one peak: set the threshold before it (no -1: zero based)
        if( histoEnd - histoStart == 0 ) { return histoStart; }
        // if only two connected peaks: set the threshold between both peaks
        if( histoEnd - histoStart == 1 ) { return histoEnd; }

        int weightedValSum0, weightedValSum1, pixCount0, pixCount1;
        double result = 0;

        // search pivot position at which:  pivot - mean1 ≈ mean2 - pivot
        // --> pivot ≈ (mean1 + mean2)/2
        // see https://staff.fnwi.uva.nl/r.vandenboomgaard/mp/files/labIsodata.pdf
        for( pivot = histoStart; pivot < histoEnd; pivot++ )
        {
            weightedValSum0 = weightedValSum1 = pixCount0 = pixCount1 = 0;

            for( int i = histoStart; i <= pivot; i++ )
            {
                weightedValSum0 += i * histo[i];
                pixCount0       += histo[i];
            }

            for( int i = (pivot + 1); i <= histoEnd; i++ )
            {
                weightedValSum1 += i * histo[i];
                pixCount1       += histo[i];
            }

            result = ( weightedValSum0 / (1.0 * pixCount0) + weightedValSum1 / (1.0 * pixCount1) ) / 2.0;

            // System.out.println( pivot +" - "+ result);

            if( pivot == round( result ) ) { break; }
        }

        return pivot;
    }

    void binarize( InputArray _img, InputArray _imgDirection, int thresholdValue )
    {
        // TODO: checks
        auto img  = _img.getMat();
        auto imgD = _imgDirection.getMat();

        if( thresholdValue < 0 ) { thresholdValue = 0; }
        if( thresholdValue > 255 ) { thresholdValue = 255; }

        for( auto itS = img.begin< uchar >(),
             itE = img.end< uchar >(),
             itD = imgD.begin< uchar >(); itS != itE; itS++, itD++ )
        {
            if( *itS < thresholdValue )
            {
                *itS = 0;
                *itD = 0;
            }
            else
            {
                *itS = 255;
            }
        }
    }

    FeatureEdges::Directions FeatureEdges::getDirection( int gx, int gy )
    {
        // http://blog.pkh.me/p/14-fun-and-canny-optim-for-a-canny-edge-detector.html

        if( gx == 0 ) { return Directions::VERTICAL; }

        int tanPI_8gx, tan3PI_8gx;

        if( gx < 0 ) { gx = -gx; gy = -gy; }
        gy = gy * ( 1 << 16 );

        // round((sqrt(2)-1) * (1<<16)) = 27146
        // round((sqrt(2)+1) * (1<<16)) = 158218
        tanPI_8gx  = 27146 * gx;
        tan3PI_8gx = 158218 * gx;

        if( (gy > -tan3PI_8gx) && (gy < -tanPI_8gx) ) { return Directions::UP45; }
        if( (gy > -tanPI_8gx) && (gy < tanPI_8gx) ) { return Directions::HORIZONTAL; }
        if( (gy > tanPI_8gx) && (gy < tan3PI_8gx) ) { return Directions::DOWN45; }

        return Directions::VERTICAL;
    }

    void FeatureEdges::nonMaxSuppression( InputOutputArray image_input, OutputArray _image_direction, InputArray _image_sobelX, InputArray _image_sobelY )
    {
        // TODO checks
        Mat input           = image_input.getMat();
        Mat inputC          = input.clone();
        Mat image_direction = _image_direction.getMat();
        Mat image_sobelX    = _image_sobelX.getMat();
        Mat image_sobelY    = _image_sobelY.getMat();

        const int endX = input.cols - 1;
        const int endY = input.rows - 1;
        Directions direction;

        uchar rInput;
        char sobelx_val, sobely_val;

        for( int x = 1; x < endX; x++ )
        {
            for( int y = 1; y < endY; y++ )
            {
                rInput     = inputC.at< uchar >( y, x );
                sobelx_val = -128 + image_sobelX.at< uchar >( y, x );
                sobely_val = -128 + image_sobelY.at< uchar >( y, x );

                direction = getDirection( sobelx_val, sobely_val );

                if( ( (sobelx_val == 0) && (sobely_val == 0) ) )
                {
                    image_direction.at< uchar >( y, x ) = (uchar) Directions::UNKNOWN;
                    input.at< uchar >( y, x )           = 0;
                }
                else if( ( (direction == Directions::HORIZONTAL)
                           && ( ( rInput < input.at< uchar >( y - 1, x ) ) || ( rInput < input.at< uchar >( y + 1, x ) ) ) )
                         || ( ( direction == Directions::VERTICAL)
                              && ( ( rInput < input.at< uchar >( y, x - 1 ) ) || ( rInput < input.at< uchar >( y, x + 1 ) ) ) )
                         || ( ( direction == Directions::UP45)
                              && ( ( rInput < input.at< uchar >( y + 1, x - 1 ) ) || ( rInput < input.at< uchar >( y - 1, x + 1 ) ) ) )
                         || ( ( direction == Directions::DOWN45)
                              && ( ( rInput < input.at< uchar >( y + 1, x + 1 ) ) || ( rInput < input.at< uchar >( y - 1, x - 1 ) ) ) ) )
                {
                    input.at< uchar >( y, x )           = 0;
                    image_direction.at< uchar >( y, x ) = (uchar) Directions::UNKNOWN;
                }
                else
                {
                    image_direction.at< uchar >( y, x ) = (uchar) direction;
                }
            }
        }
    }

    void FeatureEdges::generateSobel( cv::InputArray grayImg, cv::OutputArray _sobel, cv::OutputArray sobelx, cv::OutputArray sobely )
    {
        // TODO checks
        // assert( grayImg.total() == _sobel.total() == sobelx.total() == sobely.total() );
        /*
           if( ( CV_8UC1 != grayImg.type() ) || ( CV_8UC1 != _sobel.type() ) || ( CV_8UC1 != sobelx.type() ) || ( CV_8UC1 != sobely.type() ) )
           {
            cerr << "CV_8UC1 != grayImg.type() || CV_8UC1 != _sobel.type() || CV_8UC1 != sobelx.type() || CV_8UC1 != sobely.type()" << endl;
            cerr << "CV_8UC1: " << CV_8UC1
                 << " | grayImg.type(): " << grayImg.type()
                 << " | _sobel.type(): " << _sobel.type()
                 << " | sobelx.type(): " << sobelx.type()
                 << " | sobely.type(): " << sobely.type() << endl;
            throw Exception();
           }
          */


        auto sobelX = sobelx.getMat();
        auto sobelY = sobely.getMat();
        auto sobel  = _sobel.getMat();
        Mat tmp( sobel.rows, sobel.cols, CV_16SC1, Scalar( 0 ) );

        applyFilter< uint8_t, int16_t >( grayImg, tmp, &filter_sobelX1D1[0][0], 1, 3 );
        applyFilter< int16_t >( tmp, sobelX, &filter_sobelX1D0[0][0], 3, 1 );

        applyFilter< uint8_t, int16_t >( grayImg, tmp, &filter_sobelY1D1[0][0], 1, 3 );
        applyFilter< int16_t >( tmp, sobelY, &filter_sobelY1D0[0][0], 3, 1 );

        /*
           applyFilter< uchar >( grayImg, sobelx, &filter_sobelX[0][0], 3, 3, 128, true );
           applyFilter< uchar >( grayImg, sobely, &filter_sobelY[0][0], 3, 3, 128, true );
           //*/

        auto it_sX = sobelX.begin< int16_t >();
        auto it_sY = sobelY.begin< int16_t >();
        auto it_sS = sobel.begin< uchar >();
        auto it_sE = sobel.end< uchar >();

        while( it_sS != it_sE )
        {
            *it_sS = (uchar) hypot( ( *it_sX ), ( *it_sY ) );
            it_sS++;
            it_sX++;
            it_sY++;
        }
    }

// =============================================================================

    FeatureValue FeatureEdges::calculate( InputArray image )
    {
        const auto height = image.rows();
        const auto width  = image.cols();
        Mat image_gray( height, width, CV_8UC1 );
        Mat image_gauss( height, width, CV_8UC1 );
        Mat image_sobel( height, width, CV_8UC1 );
        Mat image_direction( height, width, CV_8UC1 );
        Mat image_sobelX( height, width, CV_16SC1 );
        Mat image_sobelY( height, width, CV_16SC1 );

        // debug img
        Mat image_sobelClone;
        Mat image_NonMaxClone;
        Mat image_directionClone;
        Mat tmp( height, width, CV_8UC1 );

        cvtColor( image, image_gray, CV_BGR2GRAY );
        applyFilter< uchar >( image_gray, tmp, &filter_gauss1D0[0][0], 3, 1);
        applyFilter< uchar >( tmp, image_gauss, &filter_gauss1D1[0][0], 1, 3);

        generateSobel( image_gray, image_sobel, image_sobelX, image_sobelY );

        if( isDebugMode )
        {
            image_sobelClone = image_sobel.clone();
        }

        nonMaxSuppression( image_sobel, image_direction, image_sobelX, image_sobelY );
        const auto threshold = ISOData( image_sobel );

        if( isDebugMode )
        {
            image_directionClone = image_direction.clone();
            image_NonMaxClone    = image_sobel.clone();
        }

        binarize( image_sobel, image_direction, threshold );

        if( isDebugMode )
        {
            Mat image_sobelX_d, image_sobelY_d;
            image_sobelX.convertTo( image_sobelX_d, CV_8UC1 );
            image_sobelY.convertTo( image_sobelY_d, CV_8UC1 );

            imshow( "Gray image", image_gray );
            imshow( "Gauss image", image_gauss );
            imshow( "SobelX image", image_sobelX_d );
            imshow( "SobelY image", image_sobelY_d );
            imshow( "Sobel image", image_sobelClone );
            imshow( "NonMax image", image_NonMaxClone );
            imshow( "Direction image", image_directionClone );
            imshow( "Binarized image", image_sobel );
            imshow( "Binarized Direction image", image_direction );

            waitKey( 0 );
            destroyAllWindows();
            waitKey( 1 );
        }

        // ------------------------------------

        int count_45up       = 0;
        int count_45down     = 0;
        int count_horizontal = 0;
        int count_vertical   = 0;
        Directions d;

        for( auto itS = image_direction.begin< uchar >(), itE = image_direction.end< uchar >(); itS != itE; itS++ )
        {
            d = (Directions) * itS;

            switch( d )
            {
                case Directions::UP45:
                {
                    count_45up++;
                    break;
                }

                case Directions::DOWN45:
                {
                    count_45down++;
                    break;
                }

                case Directions::HORIZONTAL:
                {
                    count_horizontal++;
                    break;
                }

                case Directions::VERTICAL:
                {
                    count_vertical++;
                    break;
                }
            }
        }

        auto ret = FeatureValue();
        ret.push_back( count_45up );
        ret.push_back( count_45down );
        ret.push_back( count_horizontal );
        ret.push_back( count_vertical );

        return ret;
    }

    void FeatureEdges::accumulate( InputArray image )
    {
        FeatureValue ret = calculate( image );

        accu[0] += ret[0];
        accu[1] += ret[1];
        accu[2] += ret[2];
        accu[3] += ret[3];
        count++;
    }

    FeatureValue FeatureEdges::getNormedAccumulate()
    {
        auto ret = FeatureValue();

        ret.push_back( accu[0] / count );
        ret.push_back( accu[1] / count );
        ret.push_back( accu[2] / count );
        ret.push_back( accu[3] / count );

        return ret;
    }

    void FeatureEdges::clearAccu()
    {
        accu[0] = accu[1] = accu[2] = accu[3] = 0;
        count   = 0;
    }

    double FeatureEdges::compare( FeatureValue a, FeatureValue b )
    {
        double ret = 0;

        if( (a.size() != 4) || (b.size() != 4) )
        {
            cerr << "Wrong amount of parameter: should be 3, is: a(" << a.size() << ") | b(" << b.size() << ")" << endl;
            return 0;
        }

        double out = 0, tmp = 0;

        for( int i = 0; i < 4; i++ )
        {
            if( b[i] != 0 )
            {
                tmp = a[i] / b[i];
                if( tmp > 2 ) { continue; }
                if( tmp > 1 ) { tmp = 2 - tmp; } // 1.2 -> 0.8

                out += tmp;
            }
        }

        return out / 4;
    }

    string FeatureEdges::getFilterName()
    {
        return "EdgeDirections";
    }

    FeatureEdges::FeatureEdges()
    {
        clearAccu();
    }
    FeatureEdges::~FeatureEdges(){}

// =============================================================================

    Feature::FeatureType FeatureEdges::getFeatureType()
    {
        return Feature::FeatureType::Simple;
    }

    double FeatureEdges::compare( FeatureMat grp, cv::InputArray image )
    {
        cerr << "compare( FeatureMat grp, cv::InputArray image ): Not supported for simple Features!" << endl;
        return 0;
    }

    FeatureMat FeatureEdges::getNormedAccumulateMat()
    {
        cerr << "getNormedAccumulateMat(): Not supported for simple Features" << endl;
        return FeatureMat();
    }
}
