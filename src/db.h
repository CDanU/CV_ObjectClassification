#pragma once

#include "JsonFileTree.h"
#include <iostream>

// =============================================================================

class DB
{
    public:
        class Table;
        CV::JsonFileTree source;

        void  clear();
        void  save();
        void  open( std::string file );
        Table table( std::string key );

        void close();

        DB();
        ~DB();
};

// =============================================================================

class DB::Table
{
    protected:
        DB * db = 0;
        std::string _key;

    public:
        class Entry;
        boost::property_tree::ptree source;

        void                 clear();
        Entry                entry( std::string key );
        bool                 find( std::string key );
        std::string          key();
        std::vector< Entry > list();
        void                 save();

        Table( DB* db, std::string key );
        ~Table();
};

// =============================================================================

class DB::Table::Entry
{
    protected:
        DB::Table * tb = 0;
        std::string _key;

    public:
        std::string key();
        void        add();
        void        remove();

        template< typename T >
        T get();

        template< typename T >
        void save( T obj );

        Entry( DB::Table* tb, std::string key );
        ~Entry();
};

// =============================================================================

/* DB Test */
static void db_test( DB& db )
{
    std::cout << "DB Test...\n";

    try
    {
        db.open( "storrage.json" );
        // db.clear(); // reset db

        auto table = db.table( "Picture" );
        auto entry = table.entry( "name" );

        entry.save( "FirstPic" );
        entry = table.entry( "url" );
        entry = table.entry( "enabled" );
        entry.save( true );

        table.save();
    }
    catch( const std::exception& ex )
    {
        std::cout << ex.what();
        std::cin.ignore();
    }
}
