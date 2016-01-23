#pragma once

#include <vector>
#include <opencv2/core.hpp>

namespace Ue5
{
    using FeatureValue = std::vector< double >;

    class Feature
    {
        public:
            virtual FeatureValue calculate( cv::InputArray image, std::vector< uint > points ) = 0;
            virtual FeatureValue calculate( cv::InputArray image ) = 0;
    };
}
