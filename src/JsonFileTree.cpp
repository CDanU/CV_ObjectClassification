#include "JsonFileTree.h"

#include <iostream>

namespace Ue5
{
    using namespace std;

    // static ==================================================================

    ptree::data_type & JsonFileTree::data( ptree::value_type& value ){ return value.second.data(); }

    bool JsonFileTree::find( string key, const ptree& node )
    {
        auto it = node.find( key );

        return it != node.not_found();
    }

    bool JsonFileTree::isList( ptree::value_type& value ){ return value.second.size() > 0; }

    // =========================================================================

    void JsonFileTree::clear()
    {
        path = "";
        root.clear();
        node = &root;
    }

    void JsonFileTree::erase( string key ){ root.erase( key ); }

    template< typename T >
    T JsonFileTree::get( string name ){ return node->get< T >( name ); }

    std::string JsonFileTree::getPath(){ return path; }

    ptree & JsonFileTree::getRoot(){ return root; }

    const ptree & JsonFileTree::getNode(){ return *node; }

    const ptree & JsonFileTree::goTo( string path )
    {
        if( this->path.empty() ) { jumpTo( path ); }
        else
        {
            node        = &( node->get_child( path, ptree() ) );
            this->path += "." + path;
        }

        return *node;
    }

    const ptree & JsonFileTree::jumpTo( string path )
    {
        node       = &( root.get_child( path, root ) );
        this->path = path;

        return *node;
    }

    void JsonFileTree::open( string filename, string options )
    {
        setUrl( filename );

        try
        {
            if( ( options == "rw") || ( options == "r") ) { read_json( this->filename, root ); }
            else if( options == "w" )
            {
                write_json( this->filename, root );
            }
            node = &root;
        }
        catch( const boost::property_tree::json_parser_error& ex )
        {
            if( options == "rw" ) { write_json( this->filename, root ); }
            else { throw ex; }
        }
    }

    void JsonFileTree::print( const ptree& tree, int depth )
    {
        unsigned int id = 0;
        bool hasValues  = false;

        for( auto& e : tree )
        {
            hasValues = e.second.size() > 0;
            cout << string( depth, ' ' ) << e.first << id;

            if( !hasValues ) { cout << " : " << e.second.data(); }
            cout << endl;

            if( hasValues ) { print( e.second, depth + 1 ); }
            ++id;
        }
    }

    void JsonFileTree::print(){ printRoot(); }

    void JsonFileTree::printParent(){ print( *node ); }

    void JsonFileTree::printRoot(){ print( root ); }

    void JsonFileTree::replace( const ptree& node ){ root = node; }

    void JsonFileTree::save( const ptree& node )
    { root.put_child( this->path, node ); }

    template< typename T >
    void JsonFileTree::save( ptree::value_type& value, string name, T obj )
    { value.second.put( name, obj ); }

    template< typename T >
    void JsonFileTree::save( ptree::value_type& value, T obj )
    { value.second.put_value( obj ); }

    template< typename T >
    void JsonFileTree::set( string path, T obj )
    { root.put( (this->path.empty() ? path : (this->path + ".") + path), obj ); }

    template< typename T >
    void JsonFileTree::setTo( string path, T obj ){ root.put( path, obj ); }

    void JsonFileTree::setUrl( string filename ){ this->filename = filename; }

    void JsonFileTree::write( string filename )
    {
        if( this->filename.empty() ) { return; }

        if( filename.empty() ) { filename = this->filename; }
        write( filename, root );
    }

    void JsonFileTree::write( string filename, const ptree& node ){ write_json( filename, node ); }

    void JsonFileTree::saveAndWrite( const ptree& node, string filename )
    {
        save( node );
        write( filename );
    }
}
