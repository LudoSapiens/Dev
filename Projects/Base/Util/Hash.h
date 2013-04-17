/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_HASH_H
#define BASE_HASH_H

#include <Base/StdDefs.h>

#include <Base/ADT/ConstString.h>
#include <Base/ADT/String.h>
#include <Base/Util/Bits.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS NoHash
==============================================================================*/
template< typename T >
class NoHash
{
public:
   size_t operator()( const T& t ) const;
}; //class NoHash

// Generic version.
template< typename T >
inline size_t  NoHash<T>::operator()( const T& t ) const
{
   return (size_t)t;
}

// Specializations below.


/*==============================================================================
  CLASS StdHash
==============================================================================*/
template< typename T >
class StdHash
{
public:
   size_t operator()( const T& t ) const;
}; //class StdHash


// Specializations below.

//------------------------------------------------------------------------------
//!
template<>
inline size_t  StdHash<void*>::operator()( void* const& t ) const
{
   size_t h = reinterpret_cast<const size_t>(t);
   h = ~h + (h << 15); // h = (h << 15) - h - 1
   h =  h ^ (h >> 12);
   h =  h + (h <<  2);
   h =  h ^ (h >>  4);
   h =  h * 2057;      // h = (h + (h << 3) + (h << 11)) = h + 8h + 2048h = 2057h
   h =  h ^ (h >> 16);
   return h;
}

#if !defined(_MSC_VER)
//------------------------------------------------------------------------------
//! Taken from http://www.concentric.net/~Ttwang/tech/inthash.htm,
//! but others exist (e.g. http://www.partow.net/programming/hashfunctions/index.html).
template<>
inline size_t  StdHash<char32_t>::operator()( const char32_t& t ) const
{
   size_t h = t;
   h = ~h + (h << 15); // h = (h << 15) - h - 1
   h =  h ^ (h >> 12);
   h =  h + (h <<  2);
   h =  h ^ (h >>  4);
   h =  h * 2057;      // h = (h + (h << 3) + (h << 11)) = h + 8h + 2048h = 2057h
   h =  h ^ (h >> 16);
   return h;
}
#endif

//------------------------------------------------------------------------------
//! Taken from http://www.concentric.net/~Ttwang/tech/inthash.htm,
//! but others exist (e.g. http://www.partow.net/programming/hashfunctions/index.html).
template<>
inline size_t  StdHash<uint>::operator()( const uint& t ) const
{
   size_t h = t;
   h = ~h + (h << 15); // h = (h << 15) - h - 1
   h =  h ^ (h >> 12);
   h =  h + (h <<  2);
   h =  h ^ (h >>  4);
   h =  h * 2057;      // h = (h + (h << 3) + (h << 11)) = h + 8h + 2048h = 2057h
   h =  h ^ (h >> 16);
   return h;
}

//------------------------------------------------------------------------------
//!
template<>
inline size_t  StdHash<ConstString>::operator()( const ConstString& str ) const
{
   size_t h = reinterpret_cast<const size_t>( str.rcstring() );
   h = ~h + (h << 15); // h = (h << 15) - h - 1
   h =  h ^ (h >> 12);
   h =  h + (h <<  2);
   h =  h ^ (h >>  4);
   h =  h * 2057;      // h = (h + (h << 3) + (h << 11)) = h + 8h + 2048h = 2057h
   h =  h ^ (h >> 16);
   return h;
}

//------------------------------------------------------------------------------
//! Implements Hsieh's hash algorithm (http://www.azillionmonkeys.com/qed/hash.html).
template<>
inline size_t  StdHash<String>::operator()( const String& t ) const
{
   size_t len = t.size();
   size_t h = len;
   if( len != 0 )
   {
      const uint8_t* data = (const uint8_t*)t.cstr();
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


NAMESPACE_END

#endif //BASE_HASH_H
