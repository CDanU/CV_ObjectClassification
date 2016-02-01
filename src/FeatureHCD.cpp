#define _USE_MATH_DEFINES

#include <stdint.h>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

#include "FeatureHCD.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

typedef Point3_< uint8_t > Pixel;

#if !defined(DEBUG) && defined(_DEBUG)
#define DEBUG
#endif

namespace Ue5
{
    // =============================================================================

    const int filter_sobelX[3][3] = {
        { 1, 0, -1 }, { 1, 0, -1 }, { 1, 0, -1 }
    };
    const int filter_sobelY[3][3] = {
        { 1, 1, 1 }, { 0, 0, 0 }, { -1, -1, -1 }
    };

    // =============================================================================

    vector< FeatureHCD::Corner > FeatureHCD::getCorner( int channel )
    {
        assert( channel < cornerList.size() );
        return cornerList[channel];
    }

    double FeatureHCD::gaussian( double x, double y, double sigma2 )
    {
        double t = (x * x + y * y) / (2 * sigma2);
        double u = 1.0 / (2 * M_PI * sigma2);
        double e = u * exp( -t );

        return e;
    }

    template< typename In = uchar, typename Out = In >
    void applyFilter( InputArray _data, OutputArray _output, const int* FilterArray, int kWidth, int kHeight, int Add = 0, bool norm = false, double normStrength = 1 )
    {
        // TODO checks

        Mat data         = _data.getMat();
        Mat output       = _output.getMat();
        const int height = data.rows;
        const int width  = data.cols;
        const int kernelSizeHalveWidth  = kWidth / 2;
        const int kernelSizeHalveHeight = kHeight / 2;
        const int endX = width - 1;         // kernelSizeHalveWidth;
        const int endY = height - 1;         // kernelSizeHalveHeight;
        int x, y, fx, fy;
        int xD = 0, yD = 0;

        double normDivisor = 0;
        double sum         = 0;

        const int borderBounds[4][4] = {
            { 0,                            width,                0,                              kernelSizeHalveHeight          },
            { 0,                            width,                height - kernelSizeHalveHeight, height                         },
            { 0,                            kernelSizeHalveWidth, kernelSizeHalveHeight,          height - kernelSizeHalveHeight },
            { width - kernelSizeHalveWidth, width,                kernelSizeHalveHeight,          height - kernelSizeHalveHeight },
        };

        if( norm )
        {
            for( int i = 0; i < kWidth * kHeight; i++ )
            {
                normDivisor += abs( FilterArray[i] );
            }

            normDivisor *= normStrength;
        }

        // border processing
        for( int bbNr = 0; bbNr < 4; bbNr++ )
        {
            for( x = borderBounds[bbNr][0]; x < borderBounds[bbNr][1]; x++ )
            {
                for( y = borderBounds[bbNr][2]; y < borderBounds[bbNr][3]; y++ )
                {
                    sum = 0;

                    for( fx = -kernelSizeHalveWidth; fx <= kernelSizeHalveWidth; fx++ )
                    {
                        for( fy = -kernelSizeHalveHeight; fy <= kernelSizeHalveHeight; fy++ )
                        {
                            xD = (x + fx);
                            if( xD < 0 ) { xD = 0; }
                            if( xD >= width ) { xD = width - 1; }

                            yD = (y + fy);
                            if( yD < 0 ) { yD = 0; }
                            if( yD >= height ) { yD = height - 1; }

                            sum += data.at< In >( yD, xD ) * (FilterArray[(kernelSizeHalveWidth + fx) + (kernelSizeHalveHeight + fy) * kWidth]);
                        }
                    }

                    if( norm ) { sum = round( sum / normDivisor ); }
                    sum += Add;

                    output.at< Out >( yD, xD ) = Out( sum );
                }
            }
        }

        int kernelX = 0;
        int kernelY = 0;

        // inner processing
        for( x = 0; x < width; x++ )
        {
            for( y = 0; y < height; y++ )
            {
                sum = 0;

                for( fx = -kernelSizeHalveWidth; fx <= kernelSizeHalveWidth; fx++ )
                {
                    kernelX = min( endX, max( 0, x + fx ) );

                    for( fy = -kernelSizeHalveHeight; fy <= kernelSizeHalveHeight; fy++ )
                    {
                        kernelY = min( endY, max( 0, y + fy ) );

                        sum += data.at< In >( kernelY, kernelX ) * (FilterArray[(kernelSizeHalveWidth + fx) + (kernelSizeHalveHeight + fy) * kWidth]);
                    }
                }

                if( norm ) { sum = round( sum / normDivisor ); }
                sum += Add;

                output.at< Out >( y, x ) = Out( sum );
            }
        }
    }

    void FeatureHCD::computeDerivatives( InputArray _image, int channel, int radius, double sigma )
    {
        this->Lx2 = Mat1d( this->height, this->width, double() );
        this->Ly2 = Mat1d( this->height, this->width, double() );
        this->Lxy = Mat1d( this->height, this->width, double() );

        Mat image = _image.getMat();
        Mat1d sobelx( height, width, double() );
        Mat1d sobely( height, width, double() );

#ifdef DEBUG
        Mat1d gauss( height, width, double() );
#endif

        applyFilter< uchar, double >( image, sobelx, &filter_sobelX[0][0], 3, 3, 128, true, 0.5 );     // 0.5 => 8*0.5 => sobel_val / 4
        applyFilter< uchar, double >( image, sobely, &filter_sobelY[0][0], 3, 3, 128, true, 0.5 );     // 0.5 => 8*0.5 => sobel_val / 4

        for( int y = 0; y < this->height; y++ )
        {
            for( int x = 0; x < this->width; x++ )
            {
                double val  = 0;
                double wsum = 0;

                for( int dy = -radius; dy <= radius; dy++ )
                {
                    for( int dx = -radius; dx <= radius; dx++ )
                    {

                        int xk = x + dx;
                        int yk = y + dy;
                        if( ( xk < 0) || ( xk >= this->width) ) { continue; }
                        if( ( yk < 0) || ( yk >= this->height) ) { continue; }

                        int gx = min( width - 1, max( 0, xk ) );
                        int gy = min( height - 1, max( 0, yk ) );

                        double weight = gaussian( dx, dy, sigma );
                        wsum += weight;

                        double sobelX = sobelx[yk][xk] - 128;
                        double sobelY = sobely[yk][xk] - 128;

#ifdef DEBUG
                        val += weight * image.at< uchar >( gy, gx );
#endif

                        this->Lx2[y][x] += weight * sobelX * sobelX;
                        this->Ly2[y][x] += weight * sobelY * sobelY;
                        this->Lxy[y][x] += weight * sobelX * sobelY;
                    }
                }

                this->Lx2[y][x] /= wsum;
                this->Ly2[y][x] /= wsum;
                this->Lxy[y][x] /= wsum;

                                #ifdef DEBUG
                // Debugging
                gauss[y][x] = round( val / wsum );
                                #endif
            }
        }

                #ifdef DEBUG
        // Debugging
        Mat sobelxImg;
        Mat sobelyImg;
        Mat gaussImg;

        sobelx.convertTo( sobelxImg, CV_8UC1 );
        sobely.convertTo( sobelyImg, CV_8UC1 );
        gauss.convertTo( gaussImg, CV_8UC1 );

        imshow( "Sobel X", sobelxImg );
        imshow( "Sobel Y", sobelyImg );
        imshow( "Gaussian", gaussImg );
                #endif
    }

    double FeatureHCD::harrisMeasure( int x, int y )
    {
        // matrix elements (normalized)
        double m00 = this->Lx2[y][x];
        double m01 = this->Lxy[y][x];
        double m10 = this->Lxy[y][x];
        double m11 = this->Ly2[y][x];

        // Harris corner measure = det(M)-lambda.trace(M)^2

        return m00 * m11 - m01 * m10 - 0.06 * (m00 + m11) * (m00 + m11);
    }

    bool FeatureHCD::isSpatialMaxima( InputArray _hmap, int x, int y )
    {
        Mat1d hmap = _hmap.getMat();
        int n      = 8;
        int dx[]   = { -1, 0, 1, 1, 1, 0, -1, -1 };
        int dy[]   = { -1, -1, -1, 0, 1, 1, 1, 0 };
        double w   = hmap[y][x];

        for( int i = 0; i < n; i++ )
        {
            double wk = hmap[y + dy[i]][x + dx[i]];
            if( wk >= w ) { return false; }
        }

        return true;
    }

    Mat FeatureHCD::computeHarrisMap()
    {
        // Harris measure map
        Mat1d harrismap( this->height, this->width );
        double max = 0;

        // for each pixel in the image
        for( int y = 0; y < this->height; y++ )
        {
            for( int x = 0; x < this->width; x++ )
            {
                // compute ans store the harris measure
                harrismap[y][x] = harrisMeasure( x, y );
                if( harrismap[y][x] > max ) { max = harrismap[y][x]; }
            }
        }

        // rescale measures in 0-100
        for( int y = 0; y < this->height; y++ )
        {
            for( int x = 0; x < this->width; x++ )
            {
                double h = harrismap[y][x];
                if( h < 0 ) { h = 0; } else { h = 100 * log( 1 + h ) / log( 1 + max ); }
                harrismap[y][x] = h;
            }
        }

        return harrismap;
    }

    FeatureValue FeatureHCD::filter( InputArray _output, InputArray _input, int channel )
    {
        auto ret = FeatureValue();

        auto width = _output.cols();

        this->width = width;
        auto height = _output.rows();
        this->height = height;

        auto input  = _input.getMat();
        auto output = _output.getMat();

        // precompute derivatives
        computeDerivatives( input, channel, this->radius, this->gaussiansigma );

        // Harris measure map
        Mat1d harrismap = computeHarrisMap();

        // copy of the original image (a little darker)
        auto it_sI = input.begin< uchar >();
        auto it_sO = output.begin< uchar >();
        auto it_eO = output.end< uchar >();

        while( it_sO != it_eO )
        {
            *it_sO = *it_sI * 0.80;
            it_sI++;
            it_sO++;
        }

        auto corners = getCorner( channel );

        // for each pixel in the hmap, keep the local maxima
        for( int y = 1; y < height - 1; y++ )
        {
            for( int x = 1; x < width - 1; x++ )
            {
                double h = harrismap[y][x];
                if( h < this->minMeasure ) { continue; }
                if( !isSpatialMaxima( harrismap, (int) x, (int) y ) ) { continue; }
                // add the corner to the list
                corners.push_back( Corner( x, y, h ) );
            }
        }

        // remove corners to close to each other (keep the highest measure)
        auto corners_backup = corners;
        int i = 0;
        for( vector< Corner >::iterator it = corners_backup.begin(); it != corners_backup.end(); ++it, ++i )
        {
            auto p = *it;
            for( auto n : corners_backup )
            {
                if( n == p ) { continue; }
                int dist = (int) sqrt( (p.x - n.x) * (p.x - n.x) + (p.y - n.y) * (p.y - n.y) );
                if( dist > this->minDistance ) { continue; }
                corners.erase( corners.begin() + i );
                break;
            }
        }

        if( !corners.size() ) { corners.push_back( Corner( 200, 200, 0 ) ); }

        int count = 0;

        // Display corners over the image (cross)
        int x = 0;
        int y = 0;
        for( Corner p : corners )
        {
            for( int dx = -2; dx <= 2; dx++ )
            {
                if( ( (p.x + dx) < 0 ) || ( (p.x + dx) >= width ) ) { continue; }
                x = (int) p.x + dx;
                y = (int) p.y;
                output.at< uchar >( y, x ) = 0;
            }

            for( int dy = -2; dy <= 2; dy++ )
            {
                if( (p.y + dy < 0) || ( (p.y + dy) >= height ) ) { continue; }
                x = (int) p.x;
                y = (int) p.y + dy;
                output.at< uchar >( y, x ) = 0;
            }

            // cap at max values
            if( count < MAX_FEATURE_VALUES )
            {
                ret.push_back( p.measure );
                ++count;
            }
        }

        return ret;
    }

    float FeatureHCD::getInsidePixel( InputArray image, int x, int y, int channel )
    {
        auto width  = image.cols();
        auto height = image.rows();

        if( ( x < 0) || ( y < 0) || ( x >= width) || ( y >= height) ) { return 0; }
        return image.getMat().at< uchar >( y, x );
    }

    void FeatureHCD::setInsidePixel( InputArray image, int x, int y, int channel, float value )
    {
        auto width  = image.cols();
        auto height = image.rows();

        if( ( x < 0) || ( y < 0) || ( x >= width) || ( y >= height) ) { return; }
        image.getMat().at< uchar >( y, x ) = value;
    }


    // =============================================================================

    double FeatureHCD::compare( FeatureValue a, FeatureValue b )
    {
        if( (a.size() != 4) || (b.size() != 4) )
        {
            cerr << "Wrong amount of parameter: should be 3, is: a(" << a.size() << ") | b(" << b.size() << ")" << endl;
            return 0;
        }

        double e, f, g, h;

        e = a[0] / b[0];
        if( e > 1 ) { e = 2 - e; }

        f = a[1] / b[1];
        if( f > 1 ) { f = 2 - f; }

        g = a[2] / b[2];
        if( g > 1 ) { g = 2 - g; }

        h = a[3] / b[3];
        if( h > 1 ) { h = 2 - h; }

        return (e + f + g + h) / 4;
    }

    string FeatureHCD::getFilterName(){ return "SIFT KeyPoint Detection"; }

    FeatureValue FeatureHCD::calculate( InputArray image, vector< uint > points )
    {
        cerr << "Feature does not support Points" << endl;
        return calculate( image );
    }

    // harrisFastDetectionOperator
    FeatureValue FeatureHCD::calculate( InputArray image )
    {
        const auto height = image.rows();
        const auto width  = image.cols();

        int count_channels = 1;
        int channel        = 0;

        radius        = 3;
        gaussiansigma = 100;
        minMeasure    = 80;
        minDistance   = 1;

        // setup
        // -------------------------------------
        cornerList = vector< vector< Corner > >();
        for( int i = 0; i < count_channels; i++ )
        {
            cornerList.push_back( vector< Corner >() );
        }

        // -------------------------------------

        Mat image_gray( height, width, CV_8UC1, Scalar( 0 ) );
        Mat out( height, width, CV_8UC1, Scalar( 0 ) );

        Mat Lx2Img;
        Mat Ly2Img;
        Mat LxyImg;

        cvtColor( image, image_gray, CV_BGR2GRAY );
        auto ret = filter( out, image_gray, channel );

        // fill rest with zero when MAX_FEATURE_VALUES not reached yet
        for( uint i = ret.size(); i < MAX_FEATURE_VALUES; ++i )
        {
            ret.push_back( 0 );
        }

        Lx2.convertTo( Lx2Img, CV_8UC1 );
        Ly2.convertTo( Ly2Img, CV_8UC1 );
        Lxy.convertTo( LxyImg, CV_8UC1 );

                #ifdef DEBUG
        // Debgugging
        imshow( "image", image_gray );
        imshow( "Lx2", Lx2 );
        imshow( "Ly2", Ly2 );
        imshow( "Lxy", Lxy );
        imshow( "Harris Corner Detection image", out );
        waitKey( 0 );
                #endif

        return ret;
    }

    void FeatureHCD::accumulate( InputArray image, vector< uint > points )
    {
        FeatureValue result = FeatureHCD::calculate( image, points );

        for( uint i = 0; i < MAX_FEATURE_VALUES; ++i )
        {
            accu[i] += result[i];
        }

        count++;
    }

    void FeatureHCD::accumulate( InputArray image )
    {
        FeatureValue result = FeatureHCD::calculate( image );

        for( uint i = 0; i < MAX_FEATURE_VALUES; ++i )
        {
            accu[i] += result[i];
        }

        count++;
    }

    FeatureValue FeatureHCD::getNormedAccumulate()
    {
        auto ret = FeatureValue();

        for( uint i = 0; i < MAX_FEATURE_VALUES; ++i )
        {
            ret.push_back( accu[i] / count );
        }

        return ret;
    }

    void FeatureHCD::clearAccu()
    {
        for( uint i = 0; i < MAX_FEATURE_VALUES; ++i )
        {
            accu[i] = 0;
        }

        count = 0;
    }

    // =============================================================================

    FeatureHCD::FeatureHCD()
    {
        clearAccu();
    }

    FeatureHCD::~FeatureHCD(){}
}
