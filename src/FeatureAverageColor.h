#pragma once

#include <vector>
#include <opencv2/core.hpp>

#include "Feature.h"

namespace Ue5
{
    class FeatureAverageColor : public Feature
    {
        private:
            double accu[3];
            uint64 count;
            void rgb2lab( const double rgb[3], float lab[3] );
            FeatureValue calculateP( cv::InputArray image );

        public:
            double compare( FeatureValue a, FeatureValue b );

            std::string  getFilterName();
            FeatureValue getNormedAccumulate();

            FeatureValue calculate( cv::InputArray image );
            void         accumulate( cv::InputArray image );

            void clearAccu();

            FeatureAverageColor();
            ~FeatureAverageColor();

            // =================================================================
            FeatureType getFeatureType();
            double      compare( FeatureMat grp, cv::InputArray image );
            FeatureMat  getNormedAccumulateMat();
    };
}
