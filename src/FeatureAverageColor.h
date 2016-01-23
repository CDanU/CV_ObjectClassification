#pragma once

#include "Feature.h"

namespace Ue5
{
    class FeatureAverageColor : public Feature
    {
        public:
            FeatureValue calculate( cv::InputArray image, std::vector< uint > points );
            FeatureValue calculate( cv::InputArray image );

            FeatureAverageColor();
            ~FeatureAverageColor();
    };
}
