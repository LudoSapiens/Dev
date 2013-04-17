/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_BITS_H
#define BASE_BITS_H

#include <Base/StdDefs.h>

#include <Base/Util/CPU.h>
#include <Base/Util/Union.h>

#include <cmath>

#ifdef setbit
#undef setbit
#endif

NAMESPACE_BEGIN

// Aligns a value to a specific one, ex: align(33, 32) => 64
inline uint32_t alignTo( uint32_t value, uint32_t alignment )
{
   //Idea is to add just shy of a full alignment chunk,
   //then divide, drop fractions, and multiply back
   return ((value + alignment - 1) / alignment) * alignment;
}

// Creates a bitmask of 0 to 32 bits
inline uint32_t bitmask( uint32_t s )
{
   //Trick to avoid x86 "optimization" which gives (1<<32) => 1
   //This is because the shift amount is masked to 5b
   // (ref: ftp://download.intel.com/design/Pentium4/manuals/25366720.pdf, 4-420
   // Section "IA-32 Architecture Compatibility" )
   //We simply split the range in 2 halves (efficient, since no 'if' statement)
   uint32_t s_2 = (s >> 1);
   return (1<<s_2<<(s-s_2)) - 1;
}

// Returns bits[63:32].
inline uint32_t msb( const uint64_t bits )
{
   return (uint32_t)(bits >> 32);
}

// Returns bits[31:0].
inline uint32_t lsb( const uint64_t bits )
{
   return (uint32_t)bits;
}

// Creates a uint64_t with 2 uint32_ts.
inline uint64_t to64( const uint32_t msb, const uint32_t lsb )
{
   uint64_t tmp;
   tmp = msb;
   tmp <<= 32;
   tmp |= lsb;
   return tmp;
}

// Retrieves up to 32b in the specified field
inline uint32_t getbits( const uint32_t bits, const uint32_t start, const uint32_t size )
{
   return (bits >> start) & bitmask(size);
}

// Returns the specified field with some bits replaced
inline uint32_t setbits( const uint32_t oldbits, const uint32_t start, const uint32_t size, const uint32_t newbits )
{
   uint32_t mask = bitmask(size) << start;
   return (oldbits & ~mask) | ((newbits<<start) & mask);
}

// Inverses the specified bit range
inline uint32_t flipbits( const uint32_t bits, const uint32_t start, const uint32_t size )
{
   uint32_t mask = bitmask(size) << start;
   return bits ^ mask;
}

// Retrieves a single bit as a boolean.
inline bool getbit( const uint32_t bits, const uint32_t pos )
{
   return ((bits >> pos) & 0x1) != 0x0;
}

// Returns the specified field with some bit replaced
inline uint32_t setbit( const uint32_t oldbits, const uint32_t pos, const bool v )
{
   uint32_t mask   = (0x1 << pos);
   uint32_t newbit = (v ? 0x1 : 0x0) << pos;
   return (oldbits & ~mask) | newbit;
}


// Retrieves up to 32b in the specified field vector
inline uint32_t getbits( const uint32_t* bits, const uint32_t start, const uint32_t size )
{
   const uint32_t mask = bitmask(5);
   uint32_t hi = start + size;  //highest bit (exclusive!)
   uint32_t lo = (hi & ~mask);  //always aligned to 32
   uint32_t cur_size;  //size of the current chunk
   uint32_t tmp = 0;
   while( lo > start )
   {
      cur_size = hi - lo;
      tmp <<= cur_size;
      tmp |= getbits(bits[lo>>5], lo & mask, cur_size);
      hi = lo;
      lo -= mask + 1;
   }
   cur_size = hi - start;
   tmp <<= cur_size;
   tmp |= getbits(bits[start>>5], start & mask, cur_size);
   return tmp;
}

// Replaces the specified range of bits with the specified chunk (up to 32 bits)
inline void setbits( uint32_t* bits, const uint32_t start, const uint32_t size, const uint32_t newbits )
{
   const uint32_t mask = bitmask(5);
   uint32_t last = start + size;
   uint32_t lo = start;         //lowest bit (inclusive!)
   uint32_t hi = (lo + mask) & ~mask;  //highest bit (exclusive!)
   uint32_t cur_size;           //size of the current chunk
   uint32_t bits_left = newbits;
   while( hi < last )
   {
      cur_size = hi - lo;
      bits[lo>>5] = setbits(bits[lo>>5], lo & mask, cur_size, bits_left);
      bits_left >>= cur_size;
      lo = hi;
      hi += mask + 1;
   }
   cur_size = last - lo;
   bits[lo>>5] = setbits(bits[lo>>5], lo & mask, cur_size, bits_left);
}

// Retrieves up to 32b in the specified field vector
inline uint32_t getbits( const uint8_t* bits, const uint32_t start, const uint32_t size )
{
   const uint32_t mask = bitmask(3);
   uint32_t hi = start + size;  //highest bit (exclusive!)
   uint32_t lo = (hi & ~mask);  //always aligned to 8
   uint32_t cur_size;  //size of the current chunk
   uint32_t tmp = 0;
   while( lo > start )
   {
      cur_size = hi - lo;
      tmp <<= cur_size;
      tmp |= getbits((uint32_t)bits[lo>>3], lo & mask, cur_size);
      hi = lo;
      lo -= mask + 1;
   }
   cur_size = hi - start;
   tmp <<= cur_size;
   tmp |= getbits((uint32_t)bits[start>>3], start & mask, cur_size);
   return tmp;
}

// Replaces the specified range of bits with the specified chunk (up to 32 bits)
inline void setbits( uint8_t* bits, const uint32_t start, const uint32_t size, const uint32_t newbits )
{
   const uint32_t mask = bitmask(3);
   uint32_t last = start + size;
   uint32_t lo = start;         //lowest bit (inclusive!)
   uint32_t hi = (lo + mask) & ~mask;  //highest bit (exclusive!)
   uint32_t cur_size;           //size of the current chunk
   uint32_t bits_left = newbits;
   while( hi < last )
   {
      cur_size = hi - lo;
      bits[lo>>3] = setbits(bits[lo>>3], lo & mask, cur_size, bits_left);
      bits_left >>= cur_size;
      lo = hi;
      hi += mask + 1;
   }
   cur_size = last - lo;
   bits[lo>>3] = setbits(bits[lo>>3], lo & mask, cur_size, bits_left);
}

// Retrieves up to 32b in the specified field vector
inline uint32_t getbits( const char* bits, const uint32_t start, const uint32_t size )
{
   return getbits( (const uint8_t*)bits, start, size );
}

// Replaces the specified range of bits with the specified chunk (up to 32 bits)
inline void setbits( char* bits, const uint32_t start, const uint32_t size, const uint32_t newbits )
{
   return setbits( (uint8_t*)bits, start, size, newbits );
}

//-----------------------------------------------------------------------------
//! Returns the number of leading 0s in v.
//! Based on branchless nlz() routine from Hacker's Delight.
inline uint32_t nlz( uint8_t v )
{
   int x = v;
   int y, m, n;

   y = x - 0x10;       // If positions [7:4] are 0,
   m = (y >> 8) & 4;   // add 4 to n and shift x left 4.
   n = m;
   x <<= m;

   y = x - 0x40;       // If positions [7:6] are 0,
   m = (y >> 8) & 2;   // add 2 to n and shift x left 2.
   n += m;
   x <<= m;

   y = x >> 6;          // Set y = 0, 1, 2, or 3.
   m = y & ~(y >> 1);   // Set m = 0, 1, 2, or 2 resp.

   return n + 2 - m;
}

//------------------------------------------------------------------------------
//! Counts the number of leading zero bits.
inline uint32_t nlz( uint32_t bits )
{
   // Bins using a LUT for every 8b chunk.
   static const uint32_t lut[256] = {
      8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
   };

   uint32_t n = lut[ (bits>>24) & 0xFF ];
   if( n == 8 )
   {
      n += lut[ (bits>>16) & 0xFF ];
      if( n == 16 )
      {
         n += lut[ (bits>>8) & 0xFF ];
         if( n == 24 )
         {
            n += lut[ bits & 0xFF ];
         }
      }
   }

   return n;
}


//=============================================================================
// FLOATING-POINT ROUTINES
//=============================================================================

//------------------------------------------------------------------------------
//! Converts a 32b binary representation into a float.
inline float toFloat( const uint32_t bits )
{
   U32_t u;
   u.u32 = bits;
   return u.f;
}

//------------------------------------------------------------------------------
//! Converts a 32b float into its bit representation.
inline uint32_t toBits( const float value )
{
   U32_t u;
   u.f = value;
   return u.u32;
}

//------------------------------------------------------------------------------
//!
inline uint32_t toMonotonousBits( const float value )
{
   U32_t u;
   u.f = value;
   uint32_t mask = -int(u.u32 >> 31) | 0x80000000;
   return u.u32 ^ mask;
}

//------------------------------------------------------------------------------
//!
inline float fromMonotonousBits( const uint32_t value )
{
   U32_t u;
   u.u32 = value;
   uint32_t mask = ((u.u32 >> 31) - 1) | 0x80000000;
   u.u32 ^= mask;
   return u.f;
}

//------------------------------------------------------------------------------
//!
inline uint32_t*  toMonotonousBits( size_t n, float* values )
{
   uint32_t* cur = (uint32_t*)values;
   uint32_t* end = cur + n;
   while( cur != end )
   {
      *cur ^= -(int)(*cur >> 31) | 0x80000000;
      ++cur;
   }
   return (uint32_t*)values;
}

//------------------------------------------------------------------------------
//!
inline float*  fromMonotonousBits( size_t n, uint32_t* values )
{
   uint32_t* cur = values;
   uint32_t* end = cur + n;
   while( cur != end )
   {
      *cur ^= ((*cur >> 31) - 1) | 0x80000000;
      ++cur;
   }
   return (float*)values;
}

//------------------------------------------------------------------------------
//!
inline void splitFloatBits( uint32_t bits, uint32_t& s, uint32_t& e, uint32_t& m )
{
   m = bits & 0x7FFFFF;
   bits >>= 23;
   e = bits & 0xFF;
   bits >>= 8;
   s = bits & 0x1;
}

//------------------------------------------------------------------------------
//!
inline uint32_t mergeFloatBits( uint32_t s, uint32_t e, uint32_t m )
{
   return (s<<31) | (e<<23) | m;
}

//------------------------------------------------------------------------------
//! Splits a float into its sign, exponent, and mantissa bit patterns.
inline void toBits( float value, uint32_t& s, uint32_t& e, uint32_t& m )
{
   splitFloatBits( toBits(value), s, e, m );
}

//------------------------------------------------------------------------------
//! Creates a float using sign, exponent, and mantissa bit patterns.
inline float toFloat( uint32_t s, const uint32_t e, const uint32_t m )
{
   return toFloat( mergeFloatBits(s, e, m) );
}

//-----------------------------------------------------------------------------
//! Copies the sign of y into x, and returns the result.
inline float copySign( float x, float y )
{
   uint32_t bit31 = (1 << 31);
   U32_t u, v;
   u.f = x;
   v.f = y;
   u.u32 = (u.u32 & ~bit31) | (v.u32 & bit31);
   return u.f;
}

//-----------------------------------------------------------------------------
//! Copies the sign of y into x, and returns the result.
inline double copySign( double x, double y )
{
   uint64_t bit63 = 1;
   bit63 <<= 63;
   U64_t u, v;
   u.f64 = x;
   v.f64 = y;
   u.u64 = (u.u64 & ~bit63) | (v.u64 & bit63);
   return u.f64;
}

//------------------------------------------------------------------------------
//!
/*==============================================================================
  CLASS RoundingOffsetTable
==============================================================================*/
class RoundingOffsetTable
{
public:

   inline RoundingOffsetTable();

   inline float  operator[]( size_t i ) const;

   inline float  operator()( int exp ) const;


protected:
   float  _roundingOffsets[256];
}; //class RoundingOffsetTable

static const RoundingOffsetTable  roundingOffset; //!< An object which looks like a function.

//------------------------------------------------------------------------------
//! Initializes the offset table.
inline RoundingOffsetTable::RoundingOffsetTable()
{
   for( uint i = 0; i < 256; ++i )
   {
      _roundingOffsets[i] = powf( 2.0f, (float)(23 + i - 127) );
   }
}

//------------------------------------------------------------------------------
//!
inline float  RoundingOffsetTable::operator[]( size_t i ) const
{
   return _roundingOffsets[i];
}

//------------------------------------------------------------------------------
//! Returns the rounding offset to use in the roundPrecision routine.
inline float RoundingOffsetTable::operator()( int exp ) const
{
   uint e = exp + 127;
   if( e > 255 ) e = 255;
   return _roundingOffsets[e];
}

//------------------------------------------------------------------------------
//!
inline float roundWithOffset( float val, float off )
{
   off = copySign( off, val );
   // The code below needs to not be optimized out of existence by the compiler.
   val += off;
   val -= off;
   return val;
}

//------------------------------------------------------------------------------
//!
inline float roundTo( float val, int exp )
{
   return roundWithOffset( val, roundingOffset(exp) );
}


//=============================================================================
// ENDIAN-SAFE ROUTINES
//=============================================================================

//------------------------------------------------------------------------------
//! Reads 2 uint8_ts into a single uint16_t such that:
//!   result[15:0] = { bytes[1][7:0], bytes[0][7:0] }.
//! (this code can be optimized on some architectures)
inline uint16_t  read16( const uint8_t* bytes )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   // Little-endian optimization.
   return *(const uint16_t*)bytes;
#else
   // Architecture-safe method.
   return ((uint16_t)(bytes[0])     ) |
          ((uint16_t)(bytes[1]) << 8);
#endif
}

//------------------------------------------------------------------------------
//! Reads 4 uint8_ts into a single uint32_t such that:
//!   result[31:0] = { bytes[3][7:0], bytes[2][7:0], bytes[1][7:0], bytes[0][7:0] }
//! (this code can be optimized on some architectures)
inline uint32_t  read32( const uint8_t* bytes )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   // Little-endian optimization.
   return ((uint32_t)read16(bytes    )      ) |
          ((uint32_t)read16(bytes + 2) << 16);
#else
   // Architecture-safe method.
   return ((uint32_t)(bytes[0])      ) |
          ((uint32_t)(bytes[1]) <<  8) |
          ((uint32_t)(bytes[2]) << 16) |
          ((uint32_t)(bytes[3]) << 24);
#endif
}

//------------------------------------------------------------------------------
//! Reads 8 uint8_ts into a single uint64_t such that:
//!   result[63:0] = { bytes[7][7:0], bytes[6][7:0], bytes[5][7:0], bytes[4][7:0],
//!                    bytes[3][7:0], bytes[2][7:0], bytes[1][7:0], bytes[0][7:0] }
inline uint64_t  read64( const uint8_t* bytes )
{
   // Architecture-safe method.
   return to64( read32(bytes+4), read32(bytes) );
}

//------------------------------------------------------------------------------
//! Reads 2 uint16_ts into a single uint32_t such that:
//!   result[31:0] = { words[1][15:0], words[0][15:0] }
//! (this code can be optimized on some architectures)
inline uint32_t  read32( const uint16_t* words )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE) && defined(_MSC_VER)
   // Little-endian optimization.
   return *reinterpret_cast<const uint32_t*>(words);
#else
   // Architecture-safe method.
   return ((uint32_t)(words[0])      ) |
          ((uint32_t)(words[1]) << 16);
#endif
}

//------------------------------------------------------------------------------
//! Reads 4 uint16_ts into a single uint64_t such that:
//!   result[63:0] = { words[3][15:0], words[2][15:0], words[1][15:0], words[0][15:0] }
inline uint64_t  read64( const uint16_t* words )
{
   // Architecture-safe method.
   return to64( read32(words+2), read32(words) );
}


NAMESPACE_END

#endif //BASE_BITS_H
