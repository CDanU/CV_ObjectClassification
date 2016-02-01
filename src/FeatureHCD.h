#pragma once

#include <vector>
#include <opencv2/core.hpp>

#include "Feature.h"

namespace Ue5
{
    /**
      * Harris Corner Detection
      */
    class FeatureHCD : public Feature
    {
        private:
            // private class that represents a corner
            class Corner
            {
                public:
                    int x, y;
                    double measure;
                    Corner( int x, int y, double measure )
                    {
                        this->x       = x;
                        this->y       = y;
                        this->measure = measure;
                    }

                    bool operator==( Corner& other )
                    {
                        return x == other.x && y == other.y && measure == other.measure;
                    }
            };

            // image info
            int width, height;

            // precomputed values of the derivatives
            cv::Mat1d Lx2;
            cv::Mat1d Ly2;
            cv::Mat1d Lxy;

            static const int MAX_FEATURE_VALUES = 20;

            double accu[MAX_FEATURE_VALUES];
            uint64 count;

            // corner filtering
            int radius           = 0;
            double gaussiansigma = 0;
            int minDistance      = 0;
            int minMeasure       = 0;

            // list of corners
            std::vector< std::vector< Corner > > cornerList;

        public:
            FeatureValue calculate( cv::InputArray image, std::vector< uint > points );
            FeatureValue calculate( cv::InputArray image );

            void         accumulate( cv::InputArray image, std::vector< uint > points );
            void         accumulate( cv::InputArray image );
            FeatureValue getNormedAccumulate();
            void         clearAccu();

            double      compare( FeatureValue a, FeatureValue b );
            std::string getFilterName();

            FeatureHCD();
            ~FeatureHCD();

            FeatureValue filter( cv::InputArray output, cv::InputArray input, int canal );

            std::vector< Corner > getCorner( int canal );

            double gaussian( double x, double y, double sigma2 );

            void    computeDerivatives( cv::InputArray image, int canal, int radius, double sigma );
            double  harrisMeasure( int x, int y );
            bool    isSpatialMaxima( cv::InputArray hmap, int x, int y );
            cv::Mat computeHarrisMap();
            float   getInsidePixel( cv::InputArray c, int x, int y, int canal );
            void    setInsidePixel( cv::InputArray c, int x, int y, int canal, float value );
    };
}
