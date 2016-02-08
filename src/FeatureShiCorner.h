#pragma once

#include <vector>
#include <opencv2/core.hpp>
#include "Feature.h"

namespace Ue5
{
    using FeatureValue = std::vector< double >;

    class FeatureShiCorner : public Feature
    {
        private:
            static const int filter_derX[1][3];
            static const int filter_derY[3][1];
            static const int filter_gauss[3][3];
            static const int descRadius = 2;

            void         computeRawCornerMat( cv::InputArray image, cv::OutputArray corner );
            FeatureValue genDescriptor( cv::InputArray _image, std::vector< std::pair< int, int > > points );

            std::vector< std::pair< int, int > > genPoints( cv::InputArray _image );

        public:
            double compare( FeatureValue a, FeatureValue b );

            std::string  getFilterName();
            FeatureValue getNormedAccumulate();

            FeatureValue calculate( cv::InputArray image );
            void         accumulate( cv::InputArray image );

            void clearAccu();

            FeatureShiCorner();
            ~FeatureShiCorner();
            // =================================================================
            FeatureType getFeatureType();
            double      compare( FeatureMat grp, cv::InputArray image );
            FeatureMat  getNormedAccumulateMat();
    };
}
