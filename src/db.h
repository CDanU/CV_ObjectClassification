#pragma once

#include "JsonFileTree.h"

// =============================================================================

namespace Ue5
{
    class DB
    {
        public:
            class Table;
            JsonFileTree source;

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
            T get(){ return tb->source.get< T >( _key ); }

            template< typename T >
            void save( T obj ){ tb->source.put( _key, obj ); }

            Entry( DB::Table* tb, std::string key );
            ~Entry();
    };

// =============================================================================

    /* DB Test */
    void db_test( DB& db );
}
