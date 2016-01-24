#pragma once

#include "JsonFileTree.h"

std::shared_ptr< CV::JsonFileTree > Storage( new CV::JsonFileTree() );
boost::property_tree::ptree db_ptr_table;

std::string db_key_table;
std::string db_key_entry;

void db_clear()
{
    db_ptr_table.clear();
    db_key_table = "";
    db_key_entry = "";
    Storage->clear();
}

void db_save()
{
    Storage->write();
}

void db_close()
{
    db_save();
    db_clear();
}

void db_open( std::string file )
{
    Storage->open( file, "rw" );
}

void db_table( std::string table )
{
    db_key_table = table;
    db_ptr_table = Storage->jumpTo( table );
}

void db_table_save()
{
    if( db_key_table != "" ) { Storage->save( db_ptr_table ); }
    if( db_ptr_table.empty() ) { Storage->erase( db_key_table ); }
}

void db_table_delete( std::string key = db_key_table )
{
    db_key_table = key;
    db_ptr_table.clear();
}

bool db_has_entry( std::string key )
{
    return Storage->find( key, db_ptr_table );
}

template< typename T >
void db_entry_save( T obj )
{
    db_ptr_table.put( db_key_entry, obj );
}

template< typename T >
T db_entry_get( std::string key )
{
    return db_ptr_table.get< T >( key );
}

template< typename T >
T db_entry( std::string key, T obj = T() )
{
    db_key_entry = key;
    if( !db_has_entry( key ) ) { db_entry_save( obj ); }
    return db_entry_get< T >( key );
}

void db_entry_delete( std::string key = db_key_entry )
{
    db_ptr_table.erase( key );
}

/* DB Test */
void db_test()
{
    std::cout << "DB Test...\n";

    try
    {
        db_open( "storrage.json" );
        // db_clear(); // reset db

        db_table( "Picture" );
        db_entry< std::string >( "name" );
        db_entry_save( "FirstPic" );
        db_entry< std::string >( "url" );
        db_entry< bool >( "enabled" );
        db_entry_save( true );
        db_table_save();
    }
    catch( const std::exception& ex )
    {
        std::cout << ex.what();
        std::cin.ignore();
    }
}
