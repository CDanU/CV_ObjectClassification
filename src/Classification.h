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
            const bool deleteDB;

        public:
            template< typename T, typename U >
            static void printMapSortedByVal( std::map< T, U >& m );

            void start( std::string imagePath );
            void training( std::string imagesDir );

            Classification( const FeatureList& featureList, std::string groupsConfigPath );
            virtual ~Classification();
    };
}
