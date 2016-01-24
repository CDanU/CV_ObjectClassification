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

        public:
            double compare( FeatureValue a, FeatureValue b );

            std::string  getFilterName();
            FeatureValue getNormedAccumulate();

            FeatureValue calculate( cv::InputArray image, std::vector< uint > points );
            FeatureValue calculate( cv::InputArray image );
            void         accumulate( cv::InputArray image, std::vector< uint > points );
            void         accumulate( cv::InputArray image );

            void clearAccu();

            FeatureAverageColor();
            ~FeatureAverageColor();
    };
}
