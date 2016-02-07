#pragma once

#include <string>
#include <map>

#include "db.h"
#include "Feature.h"

namespace Ue5
{
    using FeatureList = std::vector< std::unique_ptr< Ue5::Feature > >;

    class Classification
    {
        private:
            const FeatureList & featureList;
            DB * db;
            std::string picturePath;

        public:
            template< typename T, typename U >
            static void printMapSortedByVal( std::ostream& out, std::map< T, U >& m );

            template< typename T >
            double parse( T& matrix, int row, std::string img );

            void start( std::string imagePath );
            void training();
            void showMatrix();
            void setPicturePath( std::string picturePath );

            Classification( const FeatureList& featureList, std::string groupsConfigPath );
            virtual ~Classification();
    };
}
