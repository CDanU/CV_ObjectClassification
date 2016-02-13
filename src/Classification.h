#pragma once

#include <string>
#include <map>

#include "JsonFileTree.h"
#include "Feature.h"

namespace Ue5
{
    using FeatureList = std::vector< std::unique_ptr< Ue5::Feature > >;

    class Classification
    {
        private:
            const FeatureList & featureList;
            JsonFileTree jsonfileTree;
            std::string picturePath;

        public:
            /**
             * sorts key,value pairs of a map by value and prints them out into a output stream
             *
             * @param out   : output stream
             * @param m     : map which will be printed
             */
            template< typename T, typename U >
            static void printMapSortedByVal( std::ostream& out, std::map< T, U >& m );

            /**
             * TODO
             *
             * @param matrix
             * @param row
             * @param img
             * @return
             */
            /**
             * TODO?
             *
             * @param imagePath
             */
            void start( std::string imagePath );

            /**
             * starts the training process, creates a storage json file
             */
            void training();

            /**
             *  starts classification of all images inside the image dir
             *  and creates a confusion matrix image as well as a text file
             */
            void showMatrix();

            /**
             * sets the image directory path
             *
             * @param picturePath : image directory path
             */
            void setPicturePath( std::string picturePath );

            /**
             * Constructor
             *
             * @param featureList      : list of features that will be used
             * @param groupsConfigPath : file path of the json file that was generated
             *                           by the training function
             */
            Classification( const FeatureList& featureList, std::string groupsConfigPath );
            virtual ~Classification();
    };
}
