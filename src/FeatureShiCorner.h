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
            //! first derivative filter x direction
            static const int filter_derX[1][3];

            //! first derivative filter Y direction
            static const int filter_derY[3][1];

            //! 2D fauss filter
            static const int filter_gauss[3][3];

            //! descriptor radius
            static const int descRadius = 2;

            /**
             * Generates simple descriptor based on pixels in descRadius around
             * found corners
             *
             * @param _image : image from which the descriptor will be extracted
             * @param points : list of corner points
             * @return list of descriptors
             */
            FeatureValue genDescriptor( cv::InputArray _image, std::vector< std::pair< int, int > > points );

        public:
            /**
             * Uses Shi-Tomasi algorithm to generate corner image
             * additionally 20% of the weakest corners and non lokal maxima in
             * a 5x5 window are thrown away
             *
             * @param image  : image in which corners are searched
             * @param corner : output container in which the corner image will
             *                 be saved
             */
            void computeRawCornerMat( cv::InputArray image, cv::OutputArray corner );

            /**
             * Generates (x,y) list of non null pixel values in an image
             * @param _image : image from which points are derived
             * @return       : list of (x,y) pairs
             */
            std::vector< std::pair< int, int > > genPoints( cv::InputArray _image );

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

            FeatureShiCorner();
            ~FeatureShiCorner();
            // =================================================================
            //! @see Feature::getFeatureType()
            FeatureType getFeatureType();
    };
}
