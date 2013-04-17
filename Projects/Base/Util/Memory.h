/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_MEMORY_H
#define BASE_MEMORY_H

#include <Base/StdDefs.h>

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
template< typename T >
inline void  copyData( const void* src, void* dst )
{
   *(T*)dst = *(const T*)src;
}

//------------------------------------------------------------------------------
//! The following memset_#b() routines replicate the specified pattern (of # bits each)
//! for len number of bytes starting at pointer 'dst'.
//! Requirements: the pattern and destination addresses must be valid for structures
//! of size #.
BASE_DLL_API void  memset_8b  ( void* dst, const void* pattern, size_t len );
BASE_DLL_API void  memset_16b ( void* dst, const void* pattern, size_t len );
BASE_DLL_API void  memset_32b ( void* dst, const void* pattern, size_t len );
BASE_DLL_API void  memset_64b ( void* dst, const void* pattern, size_t len );
BASE_DLL_API void  memset_128b( void* dst, const void* pattern, size_t len );
BASE_DLL_API void  memset_256b( void* dst, const void* pattern, size_t len );

typedef void (*MemsetFunc)( void*, const void*, size_t );

//------------------------------------------------------------------------------
//!
inline MemsetFunc  getMemset( size_t s )
{
   switch( s )
   {
      case  1:  return memset_8b;
      case  2:  return memset_16b;
      case  4:  return memset_32b;
      case  8:  return memset_64b;
      case 16:  return memset_128b;
      case 32:  return memset_256b;
   }
   return nullptr;
}

NAMESPACE_END

#endif //BASE_MEMORY_H
