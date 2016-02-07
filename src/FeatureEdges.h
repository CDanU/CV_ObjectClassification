#pragma once

#include <vector>
#include <opencv2/core.hpp>

#include "Feature.h"

namespace Ue5
{
    class FeatureEdges : public Feature
    {
        private:
            int accu[4];
            uint64 count;
            static const int filter_sobelX[3][3];
            static const int filter_sobelY[3][3];
            static const int filter_gauss[3][3];

            void generateSobel( cv::InputArray grayImg, cv::OutputArray sobel, cv::OutputArray sobelx, cv::OutputArray sobely );
            void nonMaxSuppression( cv::InputOutputArray image_input, cv::OutputArray image_direction, cv::InputArray image_sobelX, cv::InputArray image_sobelY );

        public:
            enum Directions : uint8_t
            {
                UNKNOWN = 0,
                UP45 = 90,
                DOWN45 = 145,
                HORIZONTAL = 200,
                VERTICAL = 255
            };

            static Directions getDirection( int gx, int gy );
            static void       applyFilter( cv::InputArray Data, cv::OutputArray Output, const int* FilterArray, int kWidth, int kHeight, int C, bool norm );

            FeatureValue calculate( cv::InputArray image, std::vector< uint > points );
            FeatureValue calculate( cv::InputArray image );

            void         accumulate( cv::InputArray image, std::vector< uint > points );
            void         accumulate( cv::InputArray image );
            FeatureValue getNormedAccumulate();
            void         clearAccu();

            double      compare( FeatureValue a, FeatureValue b );
            std::string getFilterName();

            FeatureEdges();
            ~FeatureEdges();

            // =================================================================
            FeatureType getFeatureType();
            double      compare( FeatureMat grp, cv::InputArray image );
            FeatureMat  getNormedAccumulateMat();
    };
}
