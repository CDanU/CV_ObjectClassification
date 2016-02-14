#pragma once

#include <vector>
#include <opencv2/core.hpp>

#include "Feature.h"

namespace Ue5
{
    class FeatureAverageColor : public Feature
    {
        private:
            //! accumolator that saves r,g,b values
            double accu[3];

            //! counts number of accumulated r,g,b triples
            uint64 count;

            /**
             * converts rgb colors to lab colors
             *
             * @param rgb : input array containing rgb values
             * @param lab : output array in which lab values will be saved
             */
            void rgb2lab( const double rgb[3], double lab[3] );

            /**
             * TODO
             * @param image
             * @return
             */
            FeatureValue calculateP( cv::InputArray image );

        public:
            //! @see Feature::compare()
            double compare( FeatureValue a, FeatureValue b );

            //! @see Feature::getFilterName()
            std::string  getFilterName();

            //! @see Feature::getNormedAccumulate()
            FeatureValue getNormedAccumulate();

            //! @see Feature::calculate()
            FeatureValue calculate( cv::InputArray image );

            //! @see Feature::accumulate()
            void         accumulate( cv::InputArray image );

            //! @see Feature::clearAccu()
            void clearAccu();

            FeatureAverageColor();
            ~FeatureAverageColor();

            // =================================================================
            //! @see Feature::getFeatureType()
            FeatureType getFeatureType();
    };
}
