#include "Training.h"
#include "FileSearch.h"

#include <iostream>
#include <regex>
#include <vector>
#include <map>

namespace Ue5
{
    std::vector< std::string > match( std::string search, std::string regex )
    {
        std::vector< std::string > list;
        std::regex base_regex( "([a-z]+)\\.txt" );
        std::smatch match;

        if( std::regex_match( search, match, std::regex( regex ) ) )
        {
            // std::cout << search << '\n';
            for( size_t i = 0; i < match.size(); ++i )
            {
                std::ssub_match sub_match = match[i];
                std::string piece         = sub_match.str();
                // std::cout << "  submatch " << i << ": " << piece << '\n';
                list.push_back( piece );
            }
        }
        return list;
    }

    Training::Training( std::string dir, std::vector< std::unique_ptr< Feature > >& featureList, DB& db ) :
        picturesDir( dir ), featureList( featureList ), db( db )
    {}

    Training::~Training(){}

    void Training::start()
    {
        std::vector< std::string > files;

        if( !files.empty() ) { files.clear(); }
        fileSearch( files, picturesDir, "*.jpg" );

        std::cout << "List Files:\n";
        unsigned int count = 0;

        std::map< std::string, std::vector< std::string > > map;

        for( auto& file : files )
        {
            auto list = match( file, "^(.[^_]+).*$" );

            if( list.size() > 1 )
            {
                auto name = list[1];
                std::transform( name.begin(), name.end(), name.begin(), tolower );
                if( map.find( name ) == map.end() ) { map[name] = std::vector< std::string >(); }
                map[name].push_back( file );
                // std::cout << count << ": " << name << std::endl;
            }
            count++;
        }

        // auto& root = db.source.getRoot();
        ptree root;

        for( auto& e : map )
        {
            if( e.first.empty() ) { continue; }
            std::cout << e.first << std::endl;

            auto table      = db.table( e.first );
            auto entry      = table.entry( "files" );
            std::string all = "";
            std::cout << "\t" << db.source.getPath() << std::endl;

            for( auto& file : e.second )
            {
                all += file + "; ";
                // std::cout << "\t" << file << std::endl;
            }

            entry.save( all );
            root.put_child( e.first, table.source );
            std::cout << root.size() << std::endl;
            // table.save();
            // db.source.save(table.source);
        }

        // bugfix
        // db.source.replace(root);

        std::cin.ignore();
    }
}


// auto table = db.table("Group1");
// auto entry = table.entry("name");

// entry.save("blabla");
// entry = table.entry("feature_values");
// entry.save("0.0");
// entry = table.entry("pictures");
// entry.save("C://...");

// table.save();
