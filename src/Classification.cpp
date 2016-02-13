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
    struct ParseReturn
    {
        int    id  = 0;
        double max = 0;
    };

    ParseReturn parse( const ptree& root, const FeatureList& featureList, const map< string, FeatureValue >& simpleFeatureMap, const Mat& img )
    {
        // FeatureValue imageFeature;
        FeatureValue groupFeature;
        FeatureMat groupFeatureMat;

        // map< string, double > groupSimilarityCount;

        ParseReturn ret;

        int id       = 0;
        double value = 0;

        for( auto& group : root )
        {
            value = 0;

            // groupSimilarityCount.insert( make_pair( group.first, 0 ) );
            // auto & value = groupSimilarityCount.at( group.first );

            for( auto& feature : featureList )
            {
                auto featureJSONGrp = group.second.find( feature->getFilterName() );
                if( ( featureJSONGrp == group.second.not_found() ) || featureJSONGrp->second.empty() )
                { continue; }
                // -------------------------------------------------------------

                const auto fType = feature->getFeatureType();
                if( fType == Feature::Simple )
                {
                    auto & imageFeature = simpleFeatureMap.at( feature->getFilterName() );

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
                        break;
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
                            *it = uchar( val.second.get_value( 0 ) );
                            if( it == itE ) { break; }
                            it++;
                        }

                        if( it != itE ) { break; }
                        // -----------------------------------------------------

                        groupFeatureMat.push_back( desc );
                    }

                    value += feature->compare( groupFeatureMat, img );
                }

                feature->clearAccu();
            }

            value /= featureList.size();

            if( value > ret.max )
            {
                ret.max = value;
                ret.id  = id;
            }

            ++id;
        }

        // printMapSortedByVal(out, groupSimilarityCount);

        return ret;
    }

    const string TestImagesFile = "testimages.txt";

    map< string, string > getTestImages( const ptree& root, string picturePath )
    {
        const string regexString = "^(.[^_]+).*$";

        map< string, string > group_files;
        vector< string > list;

        ifstream in_stream;
        in_stream.open( picturePath + TestImagesFile );

        std::string file;
        while( in_stream.is_open() && !in_stream.eof() )
        {
            in_stream >> file;

            list.clear();
            match( list, file, regexString, true, true );
            if( list.size() == 0 ) { continue; }

            auto name = list[1];
            group_files.insert( make_pair( file, name ) );
        }

        if( group_files.size() == 0 )
        {
            for( auto& group : root )
            {
                auto filesJSONGrp = group.second.find( "files" );
                for( auto& fileC : filesJSONGrp->second )
                {
                    auto file = fileC.second.data();
                    if( file.empty() ) { continue; }
                    group_files.insert( make_pair( file, group.first ) );
                }
            }
        }

        return group_files;
    }

    void Classification::training()
    {
        jsonfileTree.clear();
        auto & root = jsonfileTree.getRoot();

        // filters group name from file name
        const string regexString = "^(.[^_]+).*$";

        map< string, vector< string > > groupFilesMap;

        vector< string > files;
        vector< string > list;

        // adds found files into files vec
        search( files, picturePath, ".+[.]((jpe?g)|(png)|(bmp))$" );

        auto testImages = getTestImages( root, picturePath );

        // sort files into groups inside groupFilesMap
        for( auto& file : files )
        {
            // list contains matched regex pattern groups
            list.clear();
            match( list, file, regexString, true, true );
            if( list.size() == 0 ) { continue; }

            auto name = list[1];

            // dont add image to training when found in testimages list
            if( testImages.size() > 0 )
            {
                auto found = testImages.find( file );
                if( found != testImages.end() )
                {
                    // make list smaller, when image was found (to be faster)
                    testImages.erase( found );
                    continue;
                }
            }

            // .................................................................

            transform( name.begin(), name.end(), name.begin(), ::tolower );

            if( groupFilesMap.find( name ) == groupFilesMap.end() ) { groupFilesMap[name] = vector< string >(); }

            groupFilesMap[name].push_back( file );
        }

        // write settings in json file
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

        jsonfileTree.write();
    }

    void progressBar( int current, int total, int limit = 35 )
    {
        int progress = total > 0 ? int( ( current / double(total) ) * 100 ) : 0;
        auto bar     = string( int( (progress / 100.0) * limit ), '=' );

        cout << "\r[" << bar << string( limit - bar.length(), ' ' ) << "] " << progress << "%" << flush;
    }

    void Classification::showMatrix()
    {
        ofstream matFile( "matrix.txt" );

        if( !matFile.is_open() ) { throw runtime_error( "Unable to open matrix.txt" ); }
        if( picturePath.empty() ) { throw runtime_error("Picture path is empty."); }

        auto & root         = jsonfileTree.getRoot();
        const int maxGroups = int( root.size() );

        auto testImages = getTestImages( root, picturePath );
        if( testImages.size() <= 0 ) { throw runtime_error( "No Test Images found!" ); }

        // string picturePath = root.get<string>("picturePath"); //  cannot find node

        matFile << "[ Confusion Matrix ]" << endl << endl;
        cout << "Build matrix..." << endl;

        vector< string > colTitles;
        vector< string > rowTitles;
        vector< double > maxValues;
        vector< double > errorRate;

        colTitles.push_back( "Picture / Group" );
        rowTitles.push_back( "Picture / Group" );

        // auto maxTotal = maxFiles > 0 ? (maxFiles < maxGroups ? maxFiles : maxGroups) : maxGroups;

        Mat1d mat( maxGroups + 3, maxGroups, double(0) );

        int count_tesimages = int( testImages.size() );

        for( auto& group : root )
        {
            colTitles.push_back( group.first );
            rowTitles.push_back( group.first );
            errorRate.push_back( 0 );
            maxValues.push_back( 0 );
        }

        count_tesimages = int( testImages.size() );

        int id      = 0;
        int current = 0;
        Mat img;
        auto simpleFeatureMap = map< string, FeatureValue >();

        progressBar( current, count_tesimages );

        for( auto& fileC : testImages )
        {
            // get group id
            id = -1;
            if( root.size() > 0 ) { id = 0; }
            for( auto& group : root )
            {
                if( group.first == fileC.second ) { break; }
                ++id;
            }

            if( ( id < 0) || ( id >= int( root.size() ) ) ) { continue; }

            img = imread( picturePath + fileC.first );

            // apply all features to image
            simpleFeatureMap.clear();
            for( auto& f : featureList )
            {
                if( f->getFeatureType() == Feature::Simple )
                {
                    simpleFeatureMap.insert( make_pair( f->getFilterName(), f->calculate( img ) ) );
                }
            }

            auto ret = parse( root, featureList, simpleFeatureMap, img );
            if( ret.max > 0 ) { mat.at< double >( ret.id, id ) += ret.max; }

            ++current;

            progressBar( current, count_tesimages );
        }

        double v   = 0;
        double val = 0;

        id = 0;
        for( auto& group : root )
        {
            v   = 0;
            val = 0;

            for( int r = 0; r < maxGroups; ++r )
            {
                v    = mat[r][id];
                val += v;
                if( r != id ) { errorRate[id] += v; }
            }

            if( val > 0 ) { maxValues[id] = 100.0 / val; }
            ++id;
        }

        rowTitles.push_back( "Total" );
        rowTitles.push_back( "Error Rate" );
        rowTitles.push_back( "Mean Rank" );

        // matrix output

        int rows = int( rowTitles.size() );
        int cols = int( colTitles.size() );

        size_t length      = 0;
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

        if( errorRate.size() > 0 ) { errmax /= errorRate.size(); }

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
        jsonfileTree.open( groupsConfigPath, "rw" );
    }

    Classification::~Classification()
    {}

    void Classification::setPicturePath( std::string picturePath )
    {
        this->picturePath = picturePath;
    }
}
