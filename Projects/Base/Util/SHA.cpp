/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Util/SHA.h>

#include <Base/IO/TextStream.h>

#include <Base/Util/CPU.h>
#include <Base/Util/Union.h>

#include <cmath>

#define PUT_BYTE_BY_BYTE 0

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

template< typename T >
inline void alignTo16b( T& t )
{
   t += 1;
   t >>= 1;
   t <<= 1;
}

template< typename T >
inline void alignTo32b( T& t )
{
   t += 3;
   t >>= 2;
   t <<= 2;
}

template< typename T >
inline void alignTo64b( T& t )
{
   t += 7;
   t >>= 3;
   t <<= 3;
}

//template< typename T >
//T alignTo( T t, T a )
//{
//   const T a_1 = a - 1;
//   return (t + a_1) & a_1;
//}

UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
SHA1::SHA1():
   _numBlocks( 0 ),
   _curSizeB( 0 )
{
   initBlock();
}

//------------------------------------------------------------------------------
//!
void
SHA1::put( const uint8_t* data, size_t n )
{
#if PUT_BYTE_BY_BYTE
   for( size_t i = 0; i < n; ++i )
   {
      put( data[i] );
   }
#else
   // Determine alignment.
   size_t a = _curSizeB & 0x03;
   size_t b = 4 - a; // Distance from right of 32b word.
   // Compute number of bytes to pad.
   size_t p = std::min( b & 0x03, n );
   // Compute an offset within the 32b word (happens when clamping to n < 4).
   size_t o = (b - p) * 8;
   // Pad the previous word.
   switch( p )
   {
      case 0:
      {  // Perfectly aligned, nothing to do.
      }  break;
      case 1:
      {  // Need to pad 1 byte.
         _W[_curSizeB>>2] |= (data[0]) << o;
         _curSizeB += 1;
         data      += 1;
         n         -= 1;
      }  break;
      case 2:
      {  // Need to pad 2 bytes.
         _W[_curSizeB>>2] |= ((data[0] << 8) | (data[1])) << o;
         _curSizeB += 2;
         data      += 2;
         n         -= 2;
      }  break;
      case 3:
      {  // Need to pad 3 bytes.
         _W[_curSizeB>>2] |= ((data[0] << 16) | (data[1] << 8) | (data[2])) << o;
         _curSizeB += 3;
         data      += 3;
         n         -= 3;
      }  break;
      default:
         CHECK( false );
         break;
   }
   checkBlock();

   // Copy most bytes 4 at a time.
   while( n >= 4 )
   {
      _W[_curSizeB>>2] = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3]);
      _curSizeB += 4;
      data      += 4;
      n         -= 4;
      checkBlock();
   }

   // Copy remaining bytes.
   switch( n )
   {
      case 0:
      {  // Perfectly aligned, nothing to do.
      }  break;
      case 1:
      {  // Need to write 1 byte.
         _W[_curSizeB>>2] |= (data[0] << 24);
      }  break;
      case 2:
      {  // Need to write 2 bytes.
         _W[_curSizeB>>2] |= (data[0] << 24) | (data[1] << 16);
      }  break;
      case 3:
      {  // Need to write 3 bytes.
         _W[_curSizeB>>2] |= (data[0] << 24) | (data[1] << 16) | (data[2] << 8);
      }  break;
      default:
         CHECK( false );
         break;
   }
   _curSizeB += n;
   checkBlock();
#endif
}

//------------------------------------------------------------------------------
//!
void
SHA1::put( uint8_t v )
{
#if PUT_BYTE_BY_BYTE
   put( &v, 1 );
#else
   // Determine alignment.
   size_t a = _curSizeB & 0x03;
   size_t o = 24 - a * 8;
   _W[_curSizeB>>2] |= (v << o);
   ++_curSizeB;
   checkBlock();
#endif
}

//------------------------------------------------------------------------------
//!
void
SHA1::put( uint16_t v )
{
#if PUT_BYTE_BY_BYTE
   U16_t u;
   u.u16 = v;
   put( u.u8[0] );
   put( u.u8[1] );
#else
   alignTo16b( _curSizeB );
   checkBlock();
   size_t o = ((~_curSizeB >> 1) & 0x01) * 16; // Alignment of 0 means put in upper bits to match SHA1 byte order.
   _W[_curSizeB>>2] |= (v << o);
   _curSizeB += 2;
   checkBlock();
#endif
}

//------------------------------------------------------------------------------
//!
void
SHA1::put( uint32_t v )
{
#if PUT_BYTE_BY_BYTE
   U32_t u;
   u.u32 = v;
   put( u.u8[0] );
   put( u.u8[1] );
   put( u.u8[2] );
   put( u.u8[3] );
#else
   alignTo32b( _curSizeB );
   checkBlock();
   _W[_curSizeB>>2] = v;
   _curSizeB += 4;
   checkBlock();
#endif
}

//------------------------------------------------------------------------------
//!
void
SHA1::put( uint64_t v )
{
   U64_t u;
   u.u64 = v;
#if PUT_BYTE_BY_BYTE
   put( u.u8[0] );
   put( u.u8[1] );
   put( u.u8[2] );
   put( u.u8[3] );
   put( u.u8[4] );
   put( u.u8[5] );
   put( u.u8[6] );
   put( u.u8[7] );
#else
   alignTo64b( _curSizeB );
   checkBlock();
   _W[_curSizeB>>2] = u.u32[0];
   _curSizeB += 4;
   checkBlock();
   _W[_curSizeB>>2] = u.u32[1];
   _curSizeB += 4;
   checkBlock();
#endif
}

//------------------------------------------------------------------------------
//!
void
SHA1::put( float v )
{
   U32_t u;
   u.f32 = v;
#if PUT_BYTE_BY_BYTE
   put( u.u8[0] );
   put( u.u8[1] );
   put( u.u8[2] );
   put( u.u8[3] );
#else
   alignTo32b( _curSizeB );
   checkBlock();
   _W[_curSizeB>>2] = u.u32;
   _curSizeB += 4;
   checkBlock();
#endif
}

//------------------------------------------------------------------------------
//!
void
SHA1::put( double v )
{
   U64_t u;
   u.f64 = v;
#if PUT_BYTE_BY_BYTE
   put( u.u8[0] );
   put( u.u8[1] );
   put( u.u8[2] );
   put( u.u8[3] );
   put( u.u8[4] );
   put( u.u8[5] );
   put( u.u8[6] );
   put( u.u8[7] );
#else
   alignTo64b( _curSizeB );
   checkBlock();
   _W[_curSizeB>>2] = u.u32[0];
   ++_curSizeB;
   checkBlock();
   _W[_curSizeB>>2] = u.u32[1];
   ++_curSizeB;
   checkBlock();
#endif
}

//------------------------------------------------------------------------------
//!
void
SHA1::begin()
{
   _digest.reset();
   initBlock();
   _numBlocks = 0;
}

//------------------------------------------------------------------------------
//!
const SHA1::Digest&
SHA1::end()
{
   uint64_t totSize = (size_t)(_numBlocks) * 512;
   totSize += (size_t)(_curSizeB) * 8;

   // Append a 1.
   put( (uint8_t)0x80 );

   if( _curSizeB >= 56 )
   {
      // Storing the last 64b (8B) won't fit in the current block.
      _curSizeB = 64;
      runBlock();
      initBlock();
   }

   // End last block with total size stored in the last 64b.
   _W[14] = (totSize >> 32) & 0xFFFFFFFF;
   _W[15] = (totSize      ) & 0xFFFFFFFF;
   _curSizeB = 64;
   runBlock();

   return _digest;
}

//------------------------------------------------------------------------------
//! Initializes _W[0] to _W[15] to zero.
void
SHA1::initBlock()
{
   _curSizeB = 0;
   // We initialize to zero in order to avoid branches in the put() routines.
   memset( _W, 0, sizeof(_W[0])*16 );
}

//------------------------------------------------------------------------------
//! Runs the block already set in _W[0] to _W[15], with padding applied.
void
SHA1::runBlock()
{
   // Initialize rest of the message schedule.
   for( uint i = 16; i < 80; ++i )
   {
      _W[i] = ROTL( 1, _W[i-3] ^ _W[i-8] ^ _W[i-14] ^ _W[i-16] );
   }

   // Initialize working variables.
   // TODO: Compare speed with local variables as opposed to instance variables.
   _a = _digest._H[0];
   _b = _digest._H[1];
   _c = _digest._H[2];
   _d = _digest._H[3];
   _e = _digest._H[4];

   // Process using a different function for every chunk of 20 words.
   uint32_t tmp;
   for( uint i = 0; i < 20; ++i )
   {
      tmp = ROTL( 5, _a ) + Ch( _b, _c, _d ) + _e + K_00_19 + _W[i];
      _e = _d;
      _d = _c;
      _c = ROTL( 30, _b );
      _b = _a;
      _a = tmp;
   }
   for( uint i = 20; i < 40; ++i )
   {
      tmp = ROTL( 5, _a ) + Parity( _b, _c, _d ) + _e + K_20_39 + _W[i];
      _e = _d;
      _d = _c;
      _c = ROTL( 30, _b );
      _b = _a;
      _a = tmp;
   }
   for( uint i = 40; i < 60; ++i )
   {
      tmp = ROTL( 5, _a ) + Maj( _b, _c, _d ) + _e + K_40_59 + _W[i];
      _e = _d;
      _d = _c;
      _c = ROTL( 30, _b );
      _b = _a;
      _a = tmp;
   }
   for( uint i = 60; i < 80; ++i )
   {
      tmp = ROTL( 5, _a ) + Parity( _b, _c, _d ) + _e + K_60_79 + _W[i];
      _e = _d;
      _d = _c;
      _c = ROTL( 30, _b );
      _b = _a;
      _a = tmp;
   }

   _digest._H[0] += _a;
   _digest._H[1] += _b;
   _digest._H[2] += _c;
   _digest._H[3] += _d;
   _digest._H[4] += _e;

   ++_numBlocks;
}
