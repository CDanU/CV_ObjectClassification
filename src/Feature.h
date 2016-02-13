#pragma once

#include <stdint.h>
#include <vector>
#include <opencv2/core.hpp>

namespace Ue5
{
    using FeatureMat   = std::vector< cv::Mat >;
    using FeatureValue = std::vector< double >;

    class Feature
    {
        protected:
            /**
             * specifies whether debug output should be displayed or not
             */
            bool isDebugMode = false;

        public:
            /**
             * set whether debug output should be displayed or not
             */
            void debugMode( bool debug ){ isDebugMode = debug; }

            /**
             * compares two feature values
             *
             * @param a : first feature
             * @param b : second feature
             * @return  : between 0 (no match) and 1 (complete match)
             */
            virtual double compare( FeatureValue a, FeatureValue b ) = 0;

            /**
             * Returns the name of the feature
             */
            virtual std::string  getFilterName()       = 0;

            /**
             * Returns a accumulated and normed feature value
             */
            virtual FeatureValue getNormedAccumulate() = 0;

            /**
             * Calculates and returns a feature value
             *
             * @param image : image on which the feature will be applied to
             *                generate a feature value
             */
            virtual FeatureValue calculate( cv::InputArray image )  = 0;

            /**
             * Calculates a feature value, keeps it in an internal accumulator
             * storage
             *
             * @param image : image on which the feature will be applied to
             *                generate a feature value
             */
            virtual void         accumulate( cv::InputArray image ) = 0;

            /**
             * clears the internal accumulator storage
             */
            virtual void clearAccu() = 0;
            // =================================================================

            //TODO: added to temp. support Descriptor Features, find a better solution

            /**
             * enum that is used to specify the type of a feature
             */
            enum FeatureType : uint8_t
            {
                Simple = 0,   //!< Simple
                Descriptor = 1//!< Descriptor
            };

            /**
             * Returns the feature type
             */
            virtual FeatureType getFeatureType() = 0;

            /**
             * Tries to find a set of descriptors in an image
             *
             * @param grp   : set of descriptors
             * @param image : image in which the descriptors will searched
             * @return      : between 0 (no match) and 1 (complete match)
             */
            virtual double      compare( FeatureMat grp, cv::InputArray image ) = 0;

            /**
             * returns accumulated set of descriptors
             */
            virtual FeatureMat  getNormedAccumulateMat() = 0;
    };
}
