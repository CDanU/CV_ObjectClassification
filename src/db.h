#pragma once

#include "JsonFileTree.h"


class DB
{
    public:
        std::shared_ptr< CV::JsonFileTree > source;

        DB() :
            source( new CV::JsonFileTree() ){}
        ~DB(){ close(); }

        class Table
        {
            DB * db = 0;
            std::string _key;

            public:
                boost::property_tree::ptree source;

                class Entry
                {
                    protected:
                        Table * tb = 0;
                        std::string _key;

                    public:
                        Entry( Table* tb, std::string key ) :
                            tb( tb ), _key( key ){ add(); }
                        ~Entry(){}

                        std::string key()   { return _key; }
                        void        add()   { if( !tb->find( _key ) ) { tb->source.put( _key, "" ); } }
                        void        remove(){ tb->source.erase( _key ); }

                        template< typename T >
                        T get(){ return tb->source.get< T >( _key ); }
                        template< typename T >
                        void save( T obj ){ tb->source.put( _key, obj ); }
                };

                Table( DB* db, std::string key ) :
                    db( db ), _key( key )
                { source = db->source->jumpTo( key ); }

                ~Table(){}

                std::string key(){ return _key; }

                void clear(){ source.clear(); }

                void save()
                {
                    if( source.empty() )
                    { db->source->erase( _key ); }
                    else
                    { db->source->save( source ); }
                }

                Entry entry( std::string key ){ return Entry( this, key ); }

                bool find( std::string key ){ return db->source->find( key, source ); }

                std::vector< Entry > list()
                {
                    std::vector< Entry > l;

                    for( auto& e : source )
                    { l.push_back( entry( e.first ) ); }

                    return l;
                }
        };

        Table table( std::string key ){ return Table( this, key ); }

        void save(){ source->write(); }

        void open( std::string file ){ source->open( file, "rw" ); }

        void close(){ save(); clear(); }
        void clear(){ source->clear(); }
};

// -----------------------------------------------------------------------

DB db;

/* DB Test */
void db_test()
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
