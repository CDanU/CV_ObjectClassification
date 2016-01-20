#include "db.h"

using namespace std;

#define TABLE DB::Table
#define ENTRY DB::Table::Entry

// =============================================================================

void DB::clear(){ source->clear(); }

void DB::open( string file ){ source->open( file, "rw" ); }

void DB::save(){ source->write(); }

TABLE DB::table( string key ){ return Table( this, key ); }

void DB::close()
{
    save();
    clear();
}

DB::DB() :
    source( new CV::JsonFileTree() )
{}

DB::~DB(){ close(); }

// =============================================================================

void TABLE::clear(){ source.clear(); }

ENTRY TABLE::entry( string key ){ return Entry( this, key ); }

bool TABLE::find( string key ){ return db->source->find( key, source ); }

string TABLE::key(){ return _key; }

vector< ENTRY > TABLE::list()
{
    vector< ENTRY > l;
    for( auto& e : source )
    {
        l.push_back( entry( e.first ) );
    }

    return l;
}

void TABLE::save()
{
    source.empty() ? db->source->erase( _key ) : db->source->save( source );
}

TABLE::Table( DB * db, string key ) :
    db( db ), _key( key )
{
    source = db->source->jumpTo( key );
}

TABLE::~Table(){}

// =============================================================================

void ENTRY::add()
{
    if( tb->find( _key ) ) { return; }

    tb->source.put( _key, "" );
}

template< typename T >
T ENTRY::get(){ return tb->source.get< T >( _key ); }

string ENTRY::key(){ return _key; }

void ENTRY::remove(){ tb->source.erase( _key ); }

template< typename T >
void ENTRY::save( T obj ){ tb->source.put( _key, obj ); }

ENTRY::Entry( TABLE * tb, string key ) :
    tb( tb ), _key( key )
{
    add();
}

ENTRY::~Entry(){}
