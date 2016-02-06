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
    void Classification::printMapSortedByVal( ostream& out, map< T, U >& m )
    {
        using PairValue = pair< T, U >;

        vector< PairValue > sortedVec( m.begin(), m.end() );

        sort( sortedVec.begin(), sortedVec.end(), []( PairValue& left, PairValue& right ){
            return left.second > right.second;
        } );

        for( auto p : sortedVec )
        {
            out << "Group: " << p.first << " -> " << p.second;
        }
    }

    void Classification::start( string imagePath )
    {
        // TODO
    }

    template< >
    double Classification::parse< Mat >( Mat& matrix, int row, string imagePath )
    {
        Mat img    = imread( imagePath );
        ptree root = db->source.getRoot();
        FeatureValue imageFeature;
        FeatureValue groupFeature;

        map< string, double > groupSimilarityCount;

        double max = 0;

        int col = 0;
        for( auto& group : root )
        {
            groupSimilarityCount.insert( make_pair( group.first, 0 ) );

            auto & value = groupSimilarityCount.at( group.first );

            for( auto& feature : featureList )
            {
                groupFeature.clear();
                imageFeature = feature->calculate( img );

                auto featureGroup = group.second.find( feature->getFilterName() );

                if( ( featureGroup == group.second.not_found() ) || featureGroup->second.empty() ) { continue; }

                groupFeature.reserve( featureGroup->second.size() );

                for( auto entry : featureGroup->second )
                {
                    groupFeature.push_back( entry.second.get_value( 0.0 ) );
                }

                auto v = feature->compare( groupFeature, imageFeature );

                value += v;

                feature->clearAccu();
            }

            value /= featureList.size();

            if( value > max ) { max = value; }

            matrix.at< double >( row, col ) = value;
            ++col;
        }

        // printMapSortedByVal(out, groupSimilarityCount);

        return max;
    }

    void Classification::training()
    {
        const string regexString = "^(.[^_]+).*$";

        map< string, vector< string > > groupFilesMap;

        vector< string > files;
        vector< string > list;

        search( files, picturePath, ".+[.]((jpe?g)|(png)|(bmp))$" );

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
        // root.put("picturePath", picturePath); // utf8 problem

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
                    cv::Mat img = cv::imread( picturePath + fileName );
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

    void Classification::showMatrix( size_t maxFiles )
    {
        ofstream matFile( "matrix.txt" );

        if( !matFile.is_open() ) { throw "Unable to open matrix.txt"; }

        matFile << "[ Confusion Matrix ]" << endl << endl;
        auto & root = db->source.getRoot();

        // string picturePath = root.get<string>("picturePath"); //  cannot find node

        if( picturePath.empty() ) { throw "Picture path is empty."; }

        cout << "Build matrix [" << string( 20, ' ' ) << "] 0%";

        vector< string > colTitle;
        vector< string > rowTitle;
        vector< double > maxValues;
        vector< double > errorRate;

        colTitle.push_back( "Picture / Group" );

        auto maxGroups = root.size();
        auto maxTotal  = maxFiles > 0 ? (maxFiles < maxGroups ? maxFiles : maxGroups) : maxGroups;

        Mat1d mat( maxTotal + 3, maxGroups + 1, double(0) );

        int r = 0;
        for( auto& group : root )
        {
            colTitle.push_back( group.first );

            for( auto& feature : group.second )
            {
                if( r >= maxTotal ) { break; }

                if( feature.first != "files" ) { continue; }

                for( auto& fileC : feature.second )
                {
                    auto file = fileC.second.data();
                    if( file.empty() ) { continue; }

                    rowTitle.push_back( file );
                    maxValues.push_back( parse< Mat >( mat, r, picturePath + file ) );
                    break;
                }

                break;
            }

            if( r <= maxTotal )
            {
                // progress bar
                // -----------------------
                int progress = int( ( r / double(maxTotal) ) * 100 );
                cout << "\r";                 // go to first char in line
                auto bar = string( int( (progress / 100.f) * 20 ), '=' );
                cout << "Build matrix [" << bar << string( 20 - bar.length(), ' ' ) << "] " << progress << "%";
                // -----------------------
            }

            ++r;
        }

        rowTitle.push_back( "Error Rate" );
        rowTitle.push_back( "Mean Rank" );

        // matrix output

        int rows = mat.rows - 1;
        int cols = mat.cols - 1;

        const int CELL_LENGTH = 20;
        auto length           = 0;
        string cell           = "";

        for( auto row = -1; row < rows; ++row )
        {
            for( auto col = -1; col < cols; ++col )
            {
                if( row == -1 )
                {
                    cell = (col + 1) < colTitle.size() ? colTitle[col + 1] : "?";
                    string msg = (col > -1 ? " | " : "") + cell + string( CELL_LENGTH - cell.length(), ' ' );
                    length += msg.length();
                    matFile << msg;
                }
                else if( row >= 0 )
                {
                    if( col == -1 )
                    {
                        cell = row < rowTitle.size() ? rowTitle[row] : "?";
                        matFile << cell + string( CELL_LENGTH - cell.length(), ' ' );
                    }
                    else if( col >= 0 )
                    {
                        auto val = 0;
                        cell = "?";

                        if( rowTitle[row] == "Error Rate" )
                        {
                            for( auto err_row = 0; err_row < maxValues.size(); ++err_row )
                            {
                                auto err_val = mat.at< double >( err_row, col );

                                if( err_val == maxValues[err_row] ) { continue; }
                                val += int( round( err_val * 100 ) );
                            }

                            cell = to_string( val );
                        }
                        else if( rowTitle[row] == "Mean Rank" )
                        {
                            // TODO
                        }
                        else if( row < maxValues.size() )
                        {
                            auto dval = mat.at< double >( row, col );
                            val  = int( round( dval * 100 ) );
                            cell = to_string( val );
                            if( dval == maxValues[row] ) { cell += " MAX"; }
                        }

                        matFile << " | " + cell + string( CELL_LENGTH - cell.length(), ' ' );
                    }
                }
            }

            if( ( row == -1) || ( row == (maxTotal - 1) ) ) { matFile << endl << string( length, '-' ) << endl; }
            else { matFile << endl; }
        }

        cout << endl << "Matrix complete. See matrix.txt." << endl;
        matFile.close();
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

    void Classification::setPicturePath( std::string picturePath )
    {
        this->picturePath = picturePath;
    }
}
