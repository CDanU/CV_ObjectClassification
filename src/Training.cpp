#include "Training.h"
#include "FileSearch.h"

#include <iostream>
#include <regex>
#include <vector>
#include <map>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace std;

namespace Ue5
{
    static vector< string > match( string search, string regex )
    {
        vector< string > list;
        smatch match;

        if( regex_match( search, match, ::regex( regex ) ) )
        {
            for( size_t i = 0; i < match.size(); ++i )
            {
                ssub_match sub_match = match[i];
                string piece         = sub_match.str();
                list.push_back( piece );
            }
        }
        return list;
    }

// =============================================================================

    void Training::start()
    {
        const string regexString = "^(.[^_]+).*$";

        map< string, vector< string > > groupFilesMap;

        vector< string > files;
        fileSearch( files, picturesDir, "*.jpg" );

        // sort files int groups inside groupFilesMap
        for( auto& file : files )
        {
            auto list = match( file, regexString );
            if( list.size() == 0 ) { continue; }
            // .................................................................

            auto name = list[1];
            transform( name.begin(), name.end(), name.begin(), ::tolower );

            if( groupFilesMap.find( name ) == groupFilesMap.end() ) { groupFilesMap[name] = vector< string >(); }

            groupFilesMap[name].push_back( file );
        }

        // write settings in json file
        auto & root = db.source.getRoot();
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
                    cv::Mat img = cv::imread( picturesDir + fileName );
                    feature->accumulate( img );
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

        db.save();
    }

// =============================================================================

    Training::Training( string dir, vector< unique_ptr< Feature > >& featureList, DB& db )
        : picturesDir( dir ), featureList( featureList ), db( db )
    {}

    Training::~Training(){}
}
