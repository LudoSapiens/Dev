/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/ADT/ConstString.h>

#include <Base/MT/Lock.h>
#include <Base/ADT/HashTable.h>

#include <cstring>
#include <cstdlib>

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

struct strHash
{
   size_t operator()( const char* const& t )
   {
      size_t len = strlen(t);
      size_t h = len;
      if( len != 0 )
      {
         const uint8_t* data = (const uint8_t*)t;
         size_t tmp;
         uint rem = (len & 0x3);
         for( len >>= 2; len > 0; --len )
         {
            h    +=  read16( data );
            tmp   = (read16( data+2 ) << 11) ^ h;
            h     = (h << 16) ^ tmp;
            data += 2*sizeof(uint16_t);
            h    += (h >> 11);
         }
         // Special case.
         switch( rem )
         {
            case 3:
               h += read16( data );
               h ^= (h << 16);
               h ^= data[sizeof(uint16_t)] << 18;
               h += (h >> 11);
               break;
            case 2:
               h += read16( data );
               h ^= (h << 11);
               h += (h >> 17);
               break;
            case 1:
               h += *data;
               h ^= (h << 10);
               h += (h >>  1);
               break;
         }
         // Force "avalanching" of final 127 bits.
         h ^= (h <<  3);
         h += (h >>  5);
         h ^= (h <<  4);
         h += (h >> 17);
         h ^= (h << 25);
         h += (h >>  6);
      }
      // else do nothing, and return 0.
      return h;
   }
};

//------------------------------------------------------------------------------
//!
struct strEqual
{
   bool operator()( const char* const& x, const char* const& y )
   {
      return strcmp( x, y ) == 0;
   }
};

//------------------------------------------------------------------------------
//!
RCString _nullStr;

template< typename K, typename D, typename H, typename C >
class StringHashTable:
   public HashTable<K,D,H,C>
{
public:
   StringHashTable( uint s ): HashTable<K,D,H,C>(s) { /*StdErr << ">> RCStringHashTable()" << nl;*/ }
#ifdef _DEBUG
   ~StringHashTable()
   {
      // StdErr << ">> ~RCStringHashTable()" << nl;
      size_t c = HashTable<K,D,H,C>::count();
      if( c != 0 )
      {
         const char* pre = "ERROR: ";
         fprintf( stderr, "%s" FMT_SIZE_T " unreleased ConstStrings:\n", pre, c );
         printStrings( pre );
      }
   }
#endif
   void print()
   {
      fprintf( stderr, "%d ConstStrings:\n", (int)HashTable<K,D,H,C>::count() );
      printStrings();
   }
   void printStrings( const char* pre = "" )
   {
      for( typename HashTable<K,D,H,C>::Iterator cur = HashTable<K,D,H,C>::begin();
           cur != HashTable<K,D,H,C>::end();
           ++cur )
      {
         fprintf( stderr, "%s%3d x '%s'\n", pre, (*cur).second->count(), (*cur).first );
      }
   }
};
typedef StringHashTable<const char*,RCString*,strHash,strEqual>  RCStringHashTable;

RCStringHashTable  _hash(1024);

Lock _lock;

UNNAMESPACE_END

NAMESPACE_BEGIN
/*==============================================================================
   CLASS RCString
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCString*
RCString::create( const char* str )
{
   // Null string.
   if( !str || str[0] == 0 )
   {
      _nullStr.addReference();
      return &_nullStr;
   }

   {
      LockGuard lock( _lock );
      RCStringHashTable::Iterator it = _hash.find( str );
      RCString* data;
      if( it == _hash.end() )
      {
         int size = (int)strlen(str);
         data = new( size ) RCString( str, size );
         _hash.add( data->cstr(), data );
      }
      else
      {
         data = (*it).second;
      }
      data->addReference();
      return data;
   }
}

//------------------------------------------------------------------------------
//!
RCString::RCString( const char* str, int size ):
   _size( size ), _count(0)
{
   memcpy( _str, str, size );
   _str[size] = 0;
}

//------------------------------------------------------------------------------
//!
void
RCString::remove()
{
   // Remove from hash.
   {
      LockGuard lock( _lock );
      _hash.erase( _str );
   }

   // delete string.
   delete this;
}

//------------------------------------------------------------------------------
//!
void*
RCString::operator new( size_t size, int strlen )
{
   return malloc( size+strlen );
}

//------------------------------------------------------------------------------
//!
void
RCString::operator delete( void* p )
{
   free(p);
}


/*==============================================================================
   CLASS ConstString
==============================================================================*/

//-----------------------------------------------------------------------------
//!
void
ConstString::printAll()
{
   _hash.print();
}

NAMESPACE_END
