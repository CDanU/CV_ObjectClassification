#pragma once

#include "Feature.h"
#include "db.h"

namespace Ue5
{
    class Training
    {
        private:
            DB & db;
            std::string picturesDir;
            std::vector< std::unique_ptr< Feature > > & featureList;

        public:
            Training( std::string dir, std::vector< std::unique_ptr< Feature > >& featureList, DB& db );
            ~Training();
            void start();
    };
}
