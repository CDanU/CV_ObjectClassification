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

            const boost::property_tree::ptree & empty_ptree()
            {
                static boost::property_tree::ptree t;

                return t;
            }

            void print( const boost::property_tree::ptree& tree, int depth = 0 )
            {
                unsigned int id = 0;
                bool hasValues  = false;

                for( auto& e : tree )
                {
                    hasValues = e.second.size() > 0;
                    std::cout << std::string( depth, ' ' ).c_str() << e.first << id;

                    if( !hasValues ) { std::cout << " : " << e.second.data(); }
                    std::cout << std::endl;

                    if( hasValues ) { print( e.second, depth + 1 ); }
                    ++id;
                }
            }

        public:
            std::string getPath(){ return this->path; }

            void clear()
            {
                path = "";
                root = boost::property_tree::ptree();
                node = &root;
            }

            template< typename T >
            T get( std::string name ){ return node->get< T >( name ); }

            void setUrl( std::string filename ){ this->filename = filename; }

            void open( std::string filename, std::string options = "r" )
            {
                setUrl( filename );

                try
                {
                    if( ( options == "rw") || ( options == "r") ) { read_json( this->filename, root ); }
                    else if( options == "w" ) { write_json( this->filename, root ); }
                    node = &root;
                }
                catch( const boost::property_tree::json_parser_error& ex )
                {
                    if( options == "rw" ) { write_json( this->filename, root ); }
                    else { throw ex; }
                }
            }

            void save( const boost::property_tree::ptree& node ){ root.put_child( this->path, node ); }

            static void write( std::string filename, const boost::property_tree::ptree& node ){ write_json( filename, node ); }

            static boost::property_tree::ptree::data_type & data( boost::property_tree::ptree::value_type& value ){ return value.second.data(); }

            static bool isList( boost::property_tree::ptree::value_type& value ){ return value.second.size() > 0; }

            template< typename T >
            static void save( boost::property_tree::ptree::value_type& value, std::string name, T obj ){ value.second.put( name, obj ); }

            template< typename T >
            static void save( boost::property_tree::ptree::value_type& value, T obj ){ value.second.put_value( obj ); }

            void erase( std::string key )
            {
                root.erase( key );
            }

            void write( std::string filename = "" )
            {
                if( this->filename.empty() ) { return; }
                if( filename.empty() ) { filename = this->filename; }
                write( filename, root );
            }

            void saveAndWrite( const boost::property_tree::ptree& node, std::string filename = "" ){ save( node ); write( filename ); }

            static bool find( std::string key, const boost::property_tree::ptree& node ){ auto it = node.find( key ); return it != node.not_found(); }

            const boost::property_tree::ptree & jumpTo( std::string path )
            {
                node       = &( root.get_child( path, empty_ptree() ) );
                this->path = path;
                return *node;
            }

            const boost::property_tree::ptree & goTo( std::string path )
            {
                if( this->path.empty() ) { jumpTo( path ); }
                else { node = &( node->get_child( path, empty_ptree() ) ); this->path += "." + path; }
                return *node;
            }

            template< typename T >
            void setTo( std::string path, T obj ){ root.put( path, obj ); }

            template< typename T >
            void set( std::string path, T obj ){ root.put( (this->path.empty() ? path : (this->path + ".") + path), obj ); }

            const boost::property_tree::ptree & getRoot(){ return root; }
            const boost::property_tree::ptree & getNode(){ return *node; }

            void printRoot()  { print( root ); }
            void printParent(){ print( *node ); }
            void print()      { printRoot(); }
    };
}
#endif
