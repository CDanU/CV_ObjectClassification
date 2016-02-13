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
            template< typename T, typename U >
            static void printMapSortedByVal( std::ostream& out, std::map< T, U >& m );

            void start( std::string imagePath );
            void training();
            void showMatrix();
            void setPicturePath( std::string picturePath );

            Classification( const FeatureList& featureList, std::string groupsConfigPath );
            virtual ~Classification();
    };
}
