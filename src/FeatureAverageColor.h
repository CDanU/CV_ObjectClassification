#pragma once

#include "IFeature.h"

namespace Ue5
{
    class FeatureAverageColor : public IFeature
    {
        public:
            FeatureValue calculate( cv::Mat image, std::vector< uint > points );

            FeatureAverageColor();
            ~FeatureAverageColor();
    };
}
