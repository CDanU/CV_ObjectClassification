#pragma once

template< typename In, typename Out = In >
void applyFilter( cv::InputArray _data, cv::OutputArray _output, const int* FilterArray, int kHeight, int kWidth, int constant = 0, bool norm = true )
{
    assert( kWidth % 2 != 0 && kHeight % 2 != 0 );

    cv::Mat data   = _data.getMat();
    cv::Mat output = _output.getMat();

    assert( data.channels() == output.channels() );
    assert( data.total() == output.total() );

    const int height = data.rows;
    const int width  = data.cols;
    const int kernelSizeHalveWidth  = kWidth / 2;
    const int kernelSizeHalveHeight = kHeight / 2;
    const int endX = width - kernelSizeHalveWidth;
    const int endY = height - kernelSizeHalveHeight;
    int x, y, fx, fy;
    int normDivisor = 0;
    int64 sum       = 0;
    int xD, yD;

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
    }

    // border processing
    for( int bbNr = 0; bbNr < 4; bbNr++ )
    {
        for( y = borderBounds[bbNr][2]; y < borderBounds[bbNr][3]; y++ )
        {
            for( x = borderBounds[bbNr][0]; x < borderBounds[bbNr][1]; x++ )
            {
                sum = 0;

                for( fy = -kernelSizeHalveHeight; fy <= kernelSizeHalveHeight; fy++ )
                {
                    for( fx = -kernelSizeHalveWidth; fx <= kernelSizeHalveWidth; fx++ )
                    {
                        xD = (x + fx);
                        if( xD < 0 ) { xD = 0; }
                        if( xD >= width ) { xD = width - 1; }

                        yD = (y + fy);
                        if( yD < 0 ) { yD = 0; }
                        if( yD >= height ) { yD = height - 1; }

                        sum += data.at< In >( yD, xD ) * ( FilterArray[(kernelSizeHalveWidth + fx) + (kernelSizeHalveHeight + fy) * kWidth] );
                    }
                }

                if( norm ) { sum /= normDivisor; }
                sum += constant;

                output.at< Out >( y, x ) = sum;
            }
        }
    }

    // inner processing
    for( y = kernelSizeHalveHeight; y < endY; y++ )
    {
        for( x = kernelSizeHalveWidth; x < endX; x++ )
        {
            sum = 0;

            for( fy = -kernelSizeHalveHeight; fy <= kernelSizeHalveHeight; fy++ )
            {
                for( fx = -kernelSizeHalveWidth; fx <= kernelSizeHalveWidth; fx++ )
                {
                    sum += data.at< In >( (y + fy), (x + fx) ) * ( FilterArray[(kernelSizeHalveWidth + fx) + (kernelSizeHalveHeight + fy) * kWidth] );
                }
            }

            if( norm ) { sum /= normDivisor; }
            sum += constant;

            output.at< Out >( y, x ) = sum;
        }
    }
}


template< typename Type >
void maxFilter( cv::InputArray _data, cv::OutputArray _output, int kWidth, int kHeight, bool borderProcessing = true )
{
    assert( kWidth % 2 != 0 && kHeight % 2 != 0 );
    assert( _data.channels() == _output.channels() );
    assert( _data.total() == _output.total() );

    _data.copyTo( _output );
    cv::Mat output = _output.getMat();

    const int height = output.rows;
    const int width  = output.cols;
    const int kernelSizeHalveWidth  = kWidth / 2;
    const int kernelSizeHalveHeight = kHeight / 2;
    const int endX = width - kernelSizeHalveWidth;
    const int endY = height - kernelSizeHalveHeight;
    int xD, yD;
    Type maxVal, centerVal;

    // inner processing
    for( int y = kernelSizeHalveHeight; y < endY; y++ )
    {
        for( int x = kernelSizeHalveWidth; x < endX; x++ )
        {
            centerVal = output.at< Type >( y, x );
            if( centerVal == 0 ) { continue; }

            maxVal = 0;

            for( int fy = -kernelSizeHalveHeight; fy <= kernelSizeHalveHeight; fy++ )
            {
                for( int fx = -kernelSizeHalveWidth; fx <= kernelSizeHalveWidth; fx++ )
                {
                    maxVal = std::max( maxVal, output.at< Type >( y + fy, x + fx ) );
                }
            }

            if( centerVal < maxVal )
            {
                output.at< Type >( y, x ) = 0;
            }
            else
            {
                // erase everything around the center
                for( int fy = -kernelSizeHalveHeight; fy <= kernelSizeHalveHeight; fy++ )
                {
                    for( int fx = -kernelSizeHalveWidth; fx <= kernelSizeHalveWidth; fx++ )
                    {
                        if( (fy == 0) && (fx == 0) ) { continue; }
                        output.at< Type >( (y + fy), (x + fx) ) = 0;
                    }
                }
            }
        }
    }

    if( !borderProcessing ) { return; }
// ______________________________________________________________________________


    const int borderBounds[4][4] = {
        { 0,                            width,                0,                              kernelSizeHalveHeight          },
        { 0,                            width,                height - kernelSizeHalveHeight, height                         },
        { 0,                            kernelSizeHalveWidth, kernelSizeHalveHeight,          height - kernelSizeHalveHeight },
        { width - kernelSizeHalveWidth, width,                kernelSizeHalveHeight,          height - kernelSizeHalveHeight },
    };


    // border processing
    for( int bbNr = 0; bbNr < 4; bbNr++ )
    {
        for( int y = borderBounds[bbNr][2]; y < borderBounds[bbNr][3]; y++ )
        {
            for( int x = borderBounds[bbNr][0]; x < borderBounds[bbNr][1]; x++ )
            {
                centerVal = output.at< Type >( y, x );
                if( centerVal == 0 ) { continue; }

                maxVal = 0;

                for( int fy = -kernelSizeHalveHeight; fy <= kernelSizeHalveHeight; fy++ )
                {
                    for( int fx = -kernelSizeHalveWidth; fx <= kernelSizeHalveWidth; fx++ )
                    {
                        xD = (x + fx);
                        if( xD < 0 ) { continue; }
                        if( xD >= width ) { continue; }

                        yD = (y + fy);
                        if( yD < 0 ) { continue; }
                        if( yD >= height ) { continue; }

                        maxVal = std::max( maxVal, output.at< Type >( yD, xD ) );
                    }
                }

                if( centerVal < maxVal )
                {
                    output.at< Type >( y, x ) = 0;
                }
                else
                {
                    for( int fy = -kernelSizeHalveHeight; fy <= kernelSizeHalveHeight; fy++ )
                    {
                        for( int fx = -kernelSizeHalveWidth; fx <= kernelSizeHalveWidth; fx++ )
                        {
                            if( (fy == 0) && (fx == 0) ) { continue; }

                            xD = (x + fx);
                            if( xD < 0 ) { continue; }
                            if( xD >= width ) { continue; }

                            yD = (y + fy);
                            if( yD < 0 ) { continue; }
                            if( yD >= height ) { continue; }

                            output.at< Type >( yD, xD ) = 0;
                        }
                    }
                }
            }
        }
    }
}
