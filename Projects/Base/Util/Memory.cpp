/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Util/Memory.h>

#include <Base/Util/Union.h>

#include <cstdio>

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
void  memset_8b( void* dst, const void* pattern, size_t len )
{
   uint8_t* cur = (uint8_t*)dst;
   uint8_t* end = cur + len;
   while( cur < end )
   {
      copyData<uint8_t>( pattern, cur );
      ++cur;
   }
}

//------------------------------------------------------------------------------
//!
void  memset_16b( void* dst, const void* pattern, size_t len )
{
   size_t    lenA = (len >> 1);
   uint16_t* curA = (uint16_t*)dst;
   uint16_t* endA = curA + lenA;
   while( curA < endA )
   {
      copyData<uint16_t>( pattern, curA );
      ++curA;
   }
   uint8_t* cur = (uint8_t*)curA;
   uint8_t* end = (uint8_t*)dst + len;
   while( cur < end )
   {
      copyData<uint8_t>( pattern, cur );
      ++cur;
   }
}

//------------------------------------------------------------------------------
//!
void  memset_32b( void* dst, const void* pattern, size_t len )
{
   size_t    lenA = (len >> 2);
   uint32_t* curA = (uint32_t*)dst;
   uint32_t* endA = curA + lenA;
   while( curA < endA )
   {
      copyData<uint32_t>( pattern, curA );
      ++curA;
   }
   uint8_t* cur = (uint8_t*)curA;
   uint8_t* end = (uint8_t*)dst + len;
   while( cur < end )
   {
      copyData<uint8_t>( pattern, cur );
      ++cur;
   }
}

//------------------------------------------------------------------------------
//!
void  memset_64b( void* dst, const void* pattern, size_t len )
{
   size_t    lenA = (len >> 3);
   uint64_t* curA = (uint64_t*)dst;
   uint64_t* endA = curA + lenA;
   while( curA < endA )
   {
      copyData<uint64_t>( pattern, curA );
      ++curA;
   }
   uint8_t* cur = (uint8_t*)curA;
   uint8_t* end = (uint8_t*)dst + len;
   while( cur < end )
   {
      copyData<uint8_t>( pattern, cur );
      ++cur;
   }
}

//------------------------------------------------------------------------------
//!
void  memset_128b( void* dst, const void* pattern, size_t len )
{
   size_t    lenA = (len >> 4);
   U128_t* curA = (U128_t*)dst;
   U128_t* endA = curA + lenA;
   while( curA < endA )
   {
      copyData<U128_t>( pattern, curA );
      ++curA;
   }
   uint8_t* cur = (uint8_t*)curA;
   uint8_t* end = (uint8_t*)dst + len;
   while( cur < end )
   {
      copyData<uint8_t>( pattern, cur );
      ++cur;
   }
}

//------------------------------------------------------------------------------
//!
void  memset_256b( void* dst, const void* pattern, size_t len )
{
   size_t    lenA = (len >> 5);
   U256_t* curA = (U256_t*)dst;
   U256_t* endA = curA + lenA;
   while( curA < endA )
   {
      copyData<U256_t>( pattern, curA );
      ++curA;
   }
   uint8_t* cur = (uint8_t*)curA;
   uint8_t* end = (uint8_t*)dst + len;
   while( cur < end )
   {
      copyData<uint8_t>( pattern, cur );
      ++cur;
   }
}


NAMESPACE_END
