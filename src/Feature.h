#pragma once

#include <vector>
#include <opencv2/core.hpp>

namespace Ue5
{
    using FeatureMat   = std::vector< cv::Mat >;
    using FeatureValue = std::vector< double >;

    class Feature
    {
        public:
            virtual double compare( FeatureValue a, FeatureValue b ) = 0;

            virtual std::string  getFilterName()       = 0;
            virtual FeatureValue getNormedAccumulate() = 0;

            virtual FeatureValue calculate( cv::InputArray image, std::vector< uint > points ) = 0;
            virtual FeatureValue calculate( cv::InputArray image ) = 0;
            virtual void         accumulate( cv::InputArray image, std::vector< uint > points ) = 0;
            virtual void         accumulate( cv::InputArray image ) = 0;

            virtual void clearAccu() = 0;
            // ==================================================================
            enum FeatureType : uint8_t
            {
                Simple = 0,
                Descriptor = 1
            };

            virtual FeatureType getFeatureType() = 0;
            virtual double      compare( FeatureMat grp, cv::InputArray image ) = 0;
            virtual FeatureMat  getNormedAccumulateMat() = 0;
    };
}
