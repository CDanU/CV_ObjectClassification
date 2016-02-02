#include "Classification.h"
#include "FileSearch.h"

#include <map>
#include <vector>
#include <iostream>
#include <clocale> // for stod . separator
#include <regex>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

namespace Ue5
{
    using namespace cv;
    using namespace std;

    template< typename T, typename U >
    void Classification::printMapSortedByVal( map< T, U >& m )
    {
        using PairValue = pair< T, U >;

        vector< PairValue > sortedVec( m.begin(), m.end() );

        sort( sortedVec.begin(), sortedVec.end(), []( PairValue& left, PairValue& right ){
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

                if( ( featureGroup == group.second.not_found() ) || !featureGroup->second.empty() ) { continue; }

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

    void Classification::training( string imagesDir )
    {
        const string regexString = "^(.[^_]+).*$";

        map< string, vector< string > > groupFilesMap;

        vector< string > files;
        vector< string > list;

        search( files, imagesDir, ".+[.]((jpe?g)|(png)|(bmp))$" );

        // sort files int groups inside groupFilesMap
        for( auto& file : files )
        {
            list.clear();
            match( list, file, regexString, true, true );
            if( list.size() == 0 ) { continue; }
            // .................................................................

            auto name = list[1];
            transform( name.begin(), name.end(), name.begin(), ::tolower );

            if( groupFilesMap.find( name ) == groupFilesMap.end() ) { groupFilesMap[name] = vector< string >(); }

            groupFilesMap[name].push_back( file );
        }

        // write settings in json file
        auto & root = db->source.getRoot();
        for( auto& e : groupFilesMap )
        {
            if( e.first.empty() ) { continue; }
            // ..................................................................

            ptree groupGroup;

            ptree fileGroup;
            for( auto& file : e.second )
            {
                ptree fileName;
                fileName.put( "", file );
                fileGroup.push_back( std::make_pair( "", fileName ) );
            }

            groupGroup.push_back( std::make_pair( "files", fileGroup ) );


            for( auto& feature : featureList )
            {
                // accumulate features of groups files
                for( auto& fileName : e.second )
                {
                    cv::Mat img = cv::imread( imagesDir + fileName );
                    feature->accumulate( img );
                    cout << "Group: " << e.first << ", Feature '" << feature->getFilterName() << "' applied to " << fileName << endl;
                }

                // add accumulated values to ptree
                ptree featureGroup;
                for( auto featureVal : feature->getNormedAccumulate() )
                {
                    ptree featureValEntry;
                    featureValEntry.put( "", featureVal );
                    featureGroup.push_back( std::make_pair( "", featureValEntry ) );
                }

                groupGroup.push_back( std::make_pair( feature->getFilterName(), featureGroup ) );

                feature->clearAccu();
            }

            root.push_back( std::make_pair( e.first, groupGroup ) );
        }

        db->save();
    }

    Classification::Classification( const FeatureList& _featureList, string groupsConfigPath )
        : featureList( _featureList )
    {
        db = new DB();

        db->open( groupsConfigPath );
    }

    Classification::~Classification()
    {
        delete db;
    }
}
