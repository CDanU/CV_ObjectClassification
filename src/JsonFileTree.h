#pragma once
#ifndef __CV_T_FILE_TREE_H_INCLUDED__
#define __CV_T_FILE_TREE_H_INCLUDED__

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace CV
{
    /*! A generalized template class for file operations */
    class JsonFileTree
    {
        protected:
            std::string filename;
            std::string path;
            boost::property_tree::ptree root;
            const boost::property_tree::ptree * node = 0;
            const boost::property_tree::ptree & empty_ptree();

            void print( const boost::property_tree::ptree& tree, int depth = 0 );

        public:
            static boost::property_tree::ptree::data_type & data( boost::property_tree::ptree::value_type& value );

            static bool find( std::string key, const boost::property_tree::ptree& node );

            static bool isList( boost::property_tree::ptree::value_type& value );

            static void write( std::string filename, const boost::property_tree::ptree& node );

            void clear();

            void erase( std::string key );

            std::string getPath();

            template< typename T >
            T get( std::string name );

            const boost::property_tree::ptree & getRoot();

            const boost::property_tree::ptree & getNode();

            const boost::property_tree::ptree & goTo( std::string path );

            const boost::property_tree::ptree & jumpTo( std::string path );

            void open( std::string filename, std::string options = "r" );

            void print();

            void printParent();

            void printRoot();

            void save( const boost::property_tree::ptree& node );

            template< typename T >
            static void save( boost::property_tree::ptree::value_type& value, std::string name, T obj );

            template< typename T >
            static void save( boost::property_tree::ptree::value_type& value, T obj );


            template< typename T >
            void set( std::string path, T obj );

            template< typename T >
            void setTo( std::string path, T obj );

            void setUrl( std::string filename );
            void write( std::string filename = "" );

            void saveAndWrite( const boost::property_tree::ptree& node, std::string filename = "" );
    };
}
#endif
