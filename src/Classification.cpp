#include "Classification.h"

#include <map>
#include <vector>
#include <iostream>
#include <clocale> // for stod . separator

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>


namespace Ue5
{
    using namespace cv;
    using namespace std;


    template< typename T, typename U >
    void Classification::printMapSortedByVal( map< T, U >& m )
    {
        vector< pair< T, U > > sortedVec( m.begin(), m.end() );

        sort( sortedVec.begin(), sortedVec.end(), []( auto& left, auto& right ){
            return left.second > right.second;
        } );

        for( auto p : sortedVec )
        {
            cout << "Group: " << p.first << " -> " << p.second << endl;
        }
    }

    void Classification::start( string imagePath )
    {
        Mat img    = imread( imagePath );
        ptree root = db->source.getRoot();
        FeatureValue imageFeature;
        FeatureValue groupFeature;

        map< string, double > groupSimilarityCount;

        for( auto& group : root )
        {
            groupSimilarityCount.insert( make_pair( group.first, 0 ) );

            for( auto& feature : featureList )
            {
                groupFeature.clear();
                imageFeature = feature->calculate( img );

                auto featureGroup = group.second.find( feature->getFilterName() );

                groupFeature.reserve( featureGroup->second.size() );

                for( auto entry : featureGroup->second )
                {
                    groupFeature.push_back( entry.second.get_value( 0.0 ) );
                }

                groupSimilarityCount.at( group.first ) += feature->compare( groupFeature, imageFeature );

                feature->clearAccu();
            }

            groupSimilarityCount.at( group.first ) /= featureList.size();
        }

        printMapSortedByVal( groupSimilarityCount );
    }

    Classification::Classification( const FeatureList& _featureList, string groupsConfigPath )
        : featureList( _featureList ), deleteDB( true )
    {
        const auto featuresCount = featureList.size();

        db = new DB();

        db->open( groupsConfigPath );
    }

    Classification::Classification( const FeatureList& _featureList, DB& _db )
        : featureList( _featureList ), deleteDB( false )
    {
        db = std::addressof( _db );
    }

    Classification::~Classification()
    {
        if( deleteDB ) { delete db; }
    }
}
