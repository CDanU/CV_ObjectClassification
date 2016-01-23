#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace Ue5
{
    using boost::property_tree::ptree;

    /*! A generalized template class for file operations */
    class JsonFileTree
    {
        protected:
            std::string filename;
            std::string path;
            ptree root;
            const ptree * node = 0;
            const ptree & empty_ptree();

            void print( const ptree& tree, int depth = 0 );

        public:
            static ptree::data_type & data( ptree::value_type& value );

            static bool find( std::string key, const ptree& node );

            static bool isList( ptree::value_type& value );

            static void write( std::string filename, const ptree& node );

            void clear();

            void erase( std::string key );

            std::string getPath();

            template< typename T >
            T get( std::string name );

            ptree & getRoot();

            const ptree & getNode();

            const ptree & goTo( std::string path );

            const ptree & jumpTo( std::string path );

            void open( std::string filename, std::string options = "r" );

            void print();

            void printParent();

            void printRoot();

            void replace( const ptree& node );
            void save( const ptree& node );

            template< typename T >
            static void save( ptree::value_type& value, std::string name, T obj );

            template< typename T >
            static void save( ptree::value_type& value, T obj );

            template< typename T >
            void set( std::string path, T obj );

            template< typename T >
            void setTo( std::string path, T obj );

            void setUrl( std::string filename );
            void write( std::string filename = "" );

            void saveAndWrite( const ptree& node, std::string filename = "" );
    };
}
