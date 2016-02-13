#pragma once

#include <vector>
#include <opencv2/core.hpp>

#include "Feature.h"

namespace Ue5
{
    class FeatureEdges : public Feature
    {
        private:
            /**
              * Accumulator that saves amount of pixels having gradient
              * directions defined in the Directions enum
              */
            int accu[4];

            //! counts amount of quadruples stored into the accumulator
            uint64 count;

            //! 2D sobelX filter
            static const int filter_sobelX[3][3];

            //! 2D sobelY filter
            static const int filter_sobelY[3][3];

            //! 2D Gauss filter
            static const int filter_gauss[3][3];
            /**
              * Generates sobelX, sobelY and sobelMagnitude images
              *
              * @param grayImg : 8bit gray input image to create sobel images
              * @param sobel   : container in which sobelMag. image will be saved
              * @param sobelx  : container in which sobelX image will be saved
              * @param sobely  : container in which sobelY image will be saved
              */
            void generateSobel( cv::InputArray grayImg, cv::OutputArray sobel, cv::OutputArray sobelx, cv::OutputArray sobely );

            /**
              * Applies a non maximum suppression on a sobel image
              *
              * @param image_input     : 8bit sobel image on which the non
              *                          maximum suppression will be applied
              * @param image_direction : container in which a gradient directions
              *                          image will be saved,  is filtered based
              *                          on the filtering process of the sobel
              *                          image
              * @param image_sobelX    : 8bit sobelX input image
              * @param image_sobelY    : 8bit sobelY input image
              */
            void nonMaxSuppression( cv::InputOutputArray image_input, cv::OutputArray image_direction, cv::InputArray image_sobelX, cv::InputArray image_sobelY );

        public:
            /**
              * enum to distinguish gradient directions,
              * enum values used when debug output is generated
              */
            enum Directions : uint8_t
            {
                UNKNOWN = 0,
                UP45 = 90,
                DOWN45 = 145,
                HORIZONTAL = 200,
                VERTICAL = 255
            };

            /**
             * Returns gradient direction based on 8-bit values of
             * gradient x and y
             *
             * @param gx : 8bit vgradient x value
             * @param gy : 8bit vgradient y value
             * @return gradient direction
             */
            static Directions getDirection( int gx, int gy );


            //! @see Feature::calculate()
            FeatureValue calculate( cv::InputArray image );

            //! @see Feature::accumulate()
            void         accumulate( cv::InputArray image );

            //! @see Feature::getNormedAccumulate()
            FeatureValue getNormedAccumulate();

            //! @see Feature::clearAccu()
            void         clearAccu();

            //! @see Feature::compare()
            double      compare( FeatureValue a, FeatureValue b );

            //! @see Feature::getFilterName()
            std::string getFilterName();

            FeatureEdges();
            ~FeatureEdges();

            // =================================================================

            //! @see Feature::getFeatureType()
            FeatureType getFeatureType();

            //! @see Feature::compare()
            double      compare( FeatureMat grp, cv::InputArray image );

            //! @see Feature::getNormedAccumulateMat()
            FeatureMat  getNormedAccumulateMat();
    };
}
