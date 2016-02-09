#include "Classification.h"
#include "FileSearch.h"
#include "Feature.h"

#include <map>
#include <vector>
#include <iostream>
#include <clocale> // for stod . separator
#include <regex>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

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
    double Classification::parse< Mat >( Mat& matrix, int col, string imagePath )
    {
        const Mat img = imread( imagePath );
        ptree root    = db->source.getRoot();
        // FeatureValue imageFeature;
        FeatureValue groupFeature;
        FeatureMat groupFeatureMat;

        map< string, double > groupSimilarityCount;

        int row    = 0, targetRow = 0;
        double max = 0;


        auto simpleFeatureMap = map< string, FeatureValue >();
        for( auto& f : featureList )
        {
            if( f->getFeatureType() == Feature::Simple )
            {
                simpleFeatureMap.insert( make_pair( f->getFilterName(), f->calculate( img ) ) );
            }
        }

        for( auto& group : root )
        {
            groupSimilarityCount.insert( make_pair( group.first, 0 ) );
            auto & value = groupSimilarityCount.at( group.first );

            for( auto& feature : featureList )
            {
                auto featureJSONGrp = group.second.find( feature->getFilterName() );
                if( ( featureJSONGrp == group.second.not_found() ) || featureJSONGrp->second.empty() )
                { continue; }
                // -------------------------------------------------------------

                const auto fType = feature->getFeatureType();
                if( fType == Feature::Simple )
                {
                    auto imageFeature = simpleFeatureMap.at( feature->getFilterName() );

                    groupFeature.clear();
                    groupFeature.reserve( featureJSONGrp->second.size() );
                    for( auto entry : featureJSONGrp->second )
                    {
                        groupFeature.push_back( entry.second.get_value( 0.0 ) );
                    }

                    value += feature->compare( groupFeature, imageFeature );
                }
                else if( fType == Feature::Descriptor )
                {
                    int descWidth     = featureJSONGrp->second.get( "width", -1 );
                    int descHeight    = featureJSONGrp->second.get( "heigt", -1 );
                    int descType      = featureJSONGrp->second.get( "type", -1 );
                    ptree matsJSONGrp = featureJSONGrp->second.get_child( "mats", ptree() );

                    if( (descWidth < 1) || (descHeight < 1) || (descType == -1) || matsJSONGrp.empty() )
                    {
                        cout << "parse error: " << feature->getFilterName() << endl;
                        continue;
                    }
                    // ---------------------------------------------------------

                    // loead desctiptors into a FeatureMat
                    groupFeatureMat.clear();
                    groupFeatureMat.reserve( matsJSONGrp.size() );
                    for( auto entry : matsJSONGrp )
                    {
                        Mat desc( descWidth, descHeight, descType );
                        const auto itE = desc.end< uchar >();
                        auto it        = desc.begin< uchar >();

                        for( auto val : entry.second )
                        {
                            *it = val.second.get_value( 0 );
                            if( it == itE ) { break; }
                            it++;
                        }

                        if( it != itE ) { continue; }
                        // -----------------------------------------------------

                        groupFeatureMat.push_back( desc );
                    }

                    value += feature->compare( groupFeatureMat, img );
                }

                feature->clearAccu();
            }

            value /= featureList.size();

            if( value > max )
            {
                max       = value;
                targetRow = row;
            }
            ++row;
        }

        // printMapSortedByVal(out, groupSimilarityCount);

        matrix.at< double >( targetRow, col ) += max;
        return max;
    }

    void Classification::training()
    {
        db->clear();

        // filters group name from file name
        const string regexString = "^(.[^_]+).*$";

        map< string, vector< string > > groupFilesMap;

        vector< string > files;
        vector< string > list;

        // adds found files into files vec
        search( files, picturePath, ".+[.]((jpe?g)|(png)|(bmp))$" );

        // sort files into groups inside groupFilesMap
        for( auto& file : files )
        {
            // list contains matched regex pattern groups
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
                const auto fType = feature->getFeatureType();

                // accumulate features of groups files
                for( auto& fileName : e.second )
                {
                    // TODO: move out
                    cv::Mat img = cv::imread( picturePath + fileName );
                    feature->accumulate( img );
                    cout << "Group: " << e.first << ", Feature '" << feature->getFilterName() << "' applied to " << fileName << endl;
                }

                // add accumulated values to ptree
                ptree featureGroup;

                if( fType == Feature::Simple )
                {
                    for( auto featureVal : feature->getNormedAccumulate() )
                    {
                        ptree featureValEntry;
                        featureValEntry.put( "", featureVal );
                        featureGroup.push_back( std::make_pair( "", featureValEntry ) );
                    }
                }
                else if( fType == Feature::Descriptor )
                {
                    ptree fMatEntryWidth, fMatEntryHeight, fMatEntryType, fMatEntryMats;
                    FeatureMat fm = feature->getNormedAccumulateMat();
                    if( fm.empty() ) { continue; }
                    // ----------------------------------------------------------

                    for( auto featureMat : fm )
                    {
                        ptree fMatEntryMatsMat;
                        for( auto itS = featureMat.begin< uchar >(), itE = featureMat.end< uchar >(); itS != itE; itS++ )
                        {
                            ptree fMatEntryMatsMatData;
                            fMatEntryMatsMatData.put( "", *itS );
                            fMatEntryMatsMat.push_back( std::make_pair( "", fMatEntryMatsMatData ) );
                        }

                        fMatEntryMats.push_back( std::make_pair( "", fMatEntryMatsMat ) );
                    }

                    fMatEntryWidth.put( "", fm[0].cols );
                    fMatEntryHeight.put( "", fm[0].rows );
                    fMatEntryType.put( "", fm[0].type() );


                    featureGroup.push_back( std::make_pair( "width", fMatEntryWidth ) );
                    featureGroup.push_back( std::make_pair( "height", fMatEntryHeight ) );
                    featureGroup.push_back( std::make_pair( "type", fMatEntryType ) );
                    featureGroup.push_back( std::make_pair( "mats", fMatEntryMats ) );
                }
                groupGroup.push_back( std::make_pair( feature->getFilterName(), featureGroup ) );

                feature->clearAccu();
            }

            root.push_back( std::make_pair( e.first, groupGroup ) );
        }

        db->save();
    }

    void Classification::showMatrix()
    {
        ofstream matFile( "matrix.txt" );

        if( !matFile.is_open() ) { throw "Unable to open matrix.txt"; }
        if( picturePath.empty() ) { throw "Picture path is empty."; }

        auto & root = db->source.getRoot();
        const size_t maxGroups = root.size();

        // string picturePath = root.get<string>("picturePath"); //  cannot find node

        matFile << "[ Confusion Matrix ]" << endl << endl;
        cout << "Build matrix [" << string( 20, ' ' ) << "] 0%";


        vector< string > colTitles;
        vector< string > rowTitles;
        vector< double > maxValues;
        vector< double > errorRate;

        colTitles.push_back( "Picture / Group" );
        rowTitles.push_back( "Picture / Group" );

        // auto maxTotal = maxFiles > 0 ? (maxFiles < maxGroups ? maxFiles : maxGroups) : maxGroups;

        Mat1d mat( maxGroups + 3, maxGroups, double(0) );

        int col = 0;
        for( auto& group : root )
        {
            colTitles.push_back( group.first );
            rowTitles.push_back( group.first );
            errorRate.push_back( 0 );
            maxValues.push_back( 0 );

            auto filesJSONGrp = group.second.find( "files" );
            for( auto& fileC : filesJSONGrp->second )
            {
                auto file = fileC.second.data();
                if( file.empty() ) { continue; }

                parse< Mat >( mat, col, picturePath + file );
                // cout << " ."  << endl;
            }

            for( int r = 0; r < maxGroups; ++r )
            {
                auto val = mat[r][col];
                maxValues[col] += val;
                if( r != col ) { errorRate[col] += val; }
            }

            maxValues[col] = 100.0 / maxValues[col];

            ++col;

            // progress bar
            // -----------------------
            int progress = int( ( col / double(maxGroups) ) * 100 );
            auto bar     = string( int( (progress / 100.0) * 20 ), '=' );

            cout << "\r"             // go to first char in line
                 << "Build matrix [" << bar << string( 20 - bar.length(), ' ' ) << "] " << progress << "%" << endl;
            // -----------------------
        }

        rowTitles.push_back( "Total" );
        rowTitles.push_back( "Error Rate" );
        rowTitles.push_back( "Mean Rank" );

        // matrix output

        int rows = rowTitles.size();
        int cols = colTitles.size();

        auto length        = 0;
        string cell        = "";
        const string split = " | ";
        string rowTitle    = "";
        string colTitle    = "";

        const double scaleFactor = (255.0 / 100.0);

        for( auto row = 0; row < rows; ++row )
        {
            rowTitle = ( row >= 0 && row < rowTitles.size() ) ? rowTitles[row] : "?";

            for( auto col = 0; col < cols; ++col )
            {
                colTitle = ( col >= 0 && col < colTitles.size() ) ? colTitles[col] : "?";

                auto CELL_LENGTH = colTitle.size();
                if( CELL_LENGTH < 10 ) { CELL_LENGTH = 10; }

                cell = "";

                if( row == 0 )
                {
                    if( col > 0 ) { length += split.length(); }
                    cell    = colTitle;
                    length += CELL_LENGTH;
                }
                else if( row > 0 )
                {
                    if( col == 0 ) { cell = rowTitle; }

                    else if( col > 0 )
                    {
                        auto row_index = row - 1;
                        auto col_index = col - 1;

                        if( rowTitle == "Error Rate" )
                        {
                            auto dval = errorRate[col_index] * maxValues[col_index];
                            dval = round( dval );
                            errorRate[col_index] = dval;
                            mat.at< double >( maxGroups + 2, col_index ) = dval * scaleFactor;
                            cell = to_string( int(dval) );
                        }
                        else if( rowTitle == "Mean Rank" )
                        {
                            cell = to_string( int(0) );
                        }
                        else if( rowTitle == "Total" )
                        {
                            double total = 0;
                            for( auto r = 0; r < maxGroups; ++r )
                            {
                                total += mat.at< double >( r, col_index );
                                mat.at< double >( r, col_index ) = int(mat.at< double >( r, col_index ) * scaleFactor);
                            }

                            cell = to_string( int( round( total ) ) );
                        }
                        else if( row_index < maxGroups )
                        {
                            auto dval = mat.at< double >( row_index, col_index ) * maxValues[col_index];
                            // dval = round( dval );
                            mat.at< double >( row_index, col_index ) = dval;
                            cell = dval == 0 ? "0" : to_string( dval );
                        }
                    }
                }
                auto len = cell.length();

                if( col > 0 ) { matFile << " | "; }
                matFile << cell << (len <= CELL_LENGTH ? string( CELL_LENGTH - len, ' ' ) : "");
            }

            if( ( row == 0) || ( row == maxGroups ) ) { matFile << endl << string( length, '-' ) << endl; }
            else { matFile << endl; }
        }

        double errmax = 0;
        for( auto errr : errorRate )
        {
            errmax += errr;
        }

        errmax /= errorRate.size();

        matFile << endl;
        matFile << "Overall Error Rate      : " << to_string( int( round( errmax ) ) ) << "%" << endl;
        matFile << "Overall Correction Rate : " << to_string( int( round( 100 - errmax ) ) ) << "%" << endl;

        cout << endl << "Matrix complete. See matrix.txt." << endl;
        matFile.close();

        Mat image;
        Mat scal;

        resize( mat, scal, Size( 400, 400 ), 0, 0, 0 );
        scal.convertTo( image, CV_8UC1 );

        imshow( "Matrix", image );           // Show our image inside it.
        waitKey( 0 );           // Wait for a keystroke in the window
        destroyAllWindows();
        waitKey( 1 );
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
