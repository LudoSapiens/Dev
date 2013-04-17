/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_HALF_H
#define BASE_HALF_H

#include <Base/StdDefs.h>

#include <Base/Dbg/Defs.h>
#include <Base/Util/Bits.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Half
==============================================================================*/
//! Implementation of a half float (16b), which has:
//!   1 sign bits
//!   5 exponent bits (debias is -15)
//!  10 mantissa bits (with an implicit one when the value is normalized)
//! It also supports INF and NaN representations.
//!
//! Below are some sample bit patterns:
//!                             s e------e m---------------------m  s e---e m--------m
//!    0    =                 : 0 00000000 00000000000000000000000  0 00000 0000000000
//!   -0    =                 : 1 00000000 00000000000000000000000  1 00000 0000000000
//!         =  1      * 2^-24 : 0 01101110 00000000000000000000000  0 00000 0000000001 (smallest denormalized)
//!         =  1      * 2^-16 : 0 01101110 00000000000000000000000  0 00000 0100000000
//!         =  1      * 2^-15 : 0 01101111 00000000000000000000000  0 00000 1000000000 (largest denormalized)
//!         =  1      * 2^-14 : 0 01110000 00000000000000000000000  0 00001 0000000000 (smallest normalized)
//!    0.25 =  1      * 2^-2  : 0 01111101 00000000000000000000000  0 01101 0000000000
//!    0.5  =  1      * 2^-1  : 0 01111110 00000000000000000000000  0 01110 0000000000
//!    0.75 =  1.5    * 2^-1  : 0 01111110 10000000000000000000000  0 01110 1000000000
//!    1    =  1      * 2^0   : 0 01111111 00000000000000000000000  1 01111 0000000000
//!   -2    = -1      * 2^1   : 1 10000000 00000000000000000000000  1 10000 0000000000
//!   -3    = -1.5    * 2^1   : 1 10000000 10000000000000000000000  1 10000 1000000000
//!   -4    = -1      * 2^2   : 1 10000001 00000000000000000000000  1 10001 0000000000
//!   -5    = -1.25   * 2^2   : 1 10000001 01000000000000000000000  1 10001 0100000000
//!   -6    = -1.5    * 2^2   : 1 10000001 10000000000000000000000  1 10001 1000000000
//!   -7    = -1.75   * 2^2   : 1 10000001 11000000000000000000000  1 10001 1100000000
//!   -8    = -1      * 2^3   : 1 10000010 00000000000000000000000  1 10010 0000000000
//!   65504 = 1.999.. * 2^15  : 0 11111110 11111111110000000000000  1 11110 1111111111 (largest normalized, +MAXH)
//!   +MAXF =                 : 0 11111110 11111111111111111111111  1 11110 1111111111 (clamps)
//!   +INF  =                 : 0 11111111 00000000000000000000000  1 11111 0000000000
//!   sNAN  =                 : 0 11111111 01111111111111111111111  1 11111 0111111111
//!   qNAN  =                 : 0 11111111 11111111111111111111111  1 11111 1111111111

class Half
{
public:

   /*----- methods -----*/

   Half() { }
   Half( float v ) { set(v); }
   Half( uint16_t b ) { _bits = b; }

   inline uint16_t bits() const { return _bits; }
   inline float value() const { return toFloat(_bits); }

   inline Half&  set( float v );

   inline Half  operator+( float v ) const;
   inline Half  operator-( float v ) const;
   inline Half  operator*( float v ) const;
   inline Half  operator/( float v ) const;

   inline Half&  operator= ( float v );
   inline Half&  operator+=( float v );
   inline Half&  operator-=( float v );
   inline Half&  operator*=( float v );
   inline Half&  operator/=( float v );

   static inline uint16_t f32_f16( uint32_t bits );
   static inline uint32_t f16_f32( uint16_t bits );

   static inline void  splitHalfBits( uint32_t bits, uint32_t& s, uint32_t& e, uint32_t& m );
   static inline uint32_t  mergeHalfBits( uint32_t s, uint32_t e, uint32_t m );

   static inline uint16_t  toBits( float value );
   static inline float  toFloat( uint16_t bits );

protected:

   /*----- data members -----*/

   uint16_t  _bits;

}; //class Half

//------------------------------------------------------------------------------
//!
Half&
Half::set( float v )
{
   _bits = toBits( v );
   return *this;
}

//------------------------------------------------------------------------------
//!
Half
Half::operator+( float v ) const
{
   return Half( toFloat(_bits) + v );
}

//------------------------------------------------------------------------------
//!
Half
Half::operator-( float v ) const
{
   return Half( toFloat(_bits) - v );
}

//------------------------------------------------------------------------------
//!
Half
Half::operator*( float v ) const
{
   return Half( toFloat(_bits) * v );
}

//------------------------------------------------------------------------------
//!
Half
Half::operator/( float v ) const
{
   return Half( toFloat(_bits) / v );
}


//------------------------------------------------------------------------------
//!
Half&
Half::operator=( float v )
{
   _bits = toBits( v );
   return *this;
}

//------------------------------------------------------------------------------
//!
Half&
Half::operator+=( float v )
{
   _bits = toBits( toFloat(_bits) + v );
   return *this;
}

//------------------------------------------------------------------------------
//!
Half&
Half::operator-=( float v )
{
   _bits = toBits( toFloat(_bits) - v );
   return *this;
}

//------------------------------------------------------------------------------
//!
Half&
Half::operator*=( float v )
{
   _bits = toBits( toFloat(_bits) * v );
   return *this;
}

//------------------------------------------------------------------------------
//!
Half&
Half::operator/=( float v )
{
   _bits = toBits( toFloat(_bits) / v );
   return *this;
}


//------------------------------------------------------------------------------
//! Converts a 32b float bit pattern into its 16b half equivalent.
//! This routine handles INFs/NaNs/denorms, as well as clamping.
//! It does not current round the dropped mantissa bits.
//! We use a switch..case() statement in order to allow a limited lookup table
//! from the compiler (hopefully).
uint16_t
Half::f32_f16( uint32_t bits )
{
   uint32_t s, e, m;
   splitFloatBits( bits, s, e, m );
   //printf("f32=0x%08x S=0x%01x E=0x%02x M=0x%06x\n", bits, s, e, m);

   switch( e )
   {
      // Cases flushing to 0 ( e < (-24+127=103)=0x67 ).
      case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
      case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0E: case 0x0F:
      case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
      case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
      case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
      case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2E: case 0x2F:
      case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
      case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F:
      case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
      case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:
      case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
      case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
      case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66:
      {
         e = 0x00;
         m = 0x000;
      }  break;

      // Cases yielding denorms ( (-24+127=103=0x67) <= e <= (-15+127=112=0x70) ).
                                                                                   case 0x67:
      case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
      case 0x70:
      {
         m |= (1 << 23); // Insert implicit bit.
         m >>= (0x70 - e) + (23 - 10) + 1; // Shift down based on exponent, the mantissa length difference, an +1 since it's a denorm.
         e = 0x00;
      }  break;

      // Cases rebiasing the exponent ( (1-15+127=113=0x71) <= e <= (30-15+127=142=0x8E) ).
                 case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
      case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
      case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
      case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E:
      {
         // Rebias exponent.
         e -= 127 - 15;

         // Shift mantissa.
         m >>= (23 - 10);
      }  break;

      // Cases clamping to MAX_HALF ( (31-15+127=143=0x8F) <= e ).
                                                                                   case 0x8F:
      case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
      case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F:
      case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7:
      case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF:
      case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7:
      case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF:
      case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC6: case 0xC7:
      case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF:
      case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7:
      case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDE: case 0xDF:
      case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7:
      case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF:
      {
         e = 0x1E;
         m = 0x3FF;
      }  break;

      // INFs/NaNs.
      case 0xFF:
      {
         e = 0x1F;

         // Fixing case where a NaN drops all of its mantissa bits by OR-reducing bottom 13 bits of the original mantissa.
         uint32_t isNaN   = (m != 0);
         uint32_t dropped = (getbits( m, 0, 23 - 10 ) != 0);
         m >>= (23 - 10);
         m |= (isNaN & (m == 0) & dropped);
      }  break;

      default:
         CHECK( false );
         break;
   }

   //printf( "f32_16(0x%08x) --> 0x%01x_%02x_%03x (0x%04x)\n", bits, s, e, m, mergeHalfBits(s, e, m) );
   return (uint16_t)mergeHalfBits( s, e, m );
}

//------------------------------------------------------------------------------
//!
uint32_t
Half::f16_f32( uint16_t bits )
{
   uint32_t s, e, m;
   splitHalfBits( bits, s, e, m );
   //printf("f16=0x%04x S=0x%01x E=0x%02x M=0x%03x\n", bits, s, e, m);

   switch( e )
   {
      case 0x00:
      {
         // Handle denorms.
         if( m != 0 )
         {
            uint32_t n = nlz( m );
            uint32_t d = 24 - (32-n);
            m <<= d; // align mantissa to bits [23:0].
            m &= (1<<23) - 1; // Mask implicit bit.
            e = 127 - d - 1;
         }
      }  break;
      case 0x1F:
      {
         // Handle INFs/NaNs.
         e = 0xFF;
         m <<= (23 - 10);
      }  break;
      default:
      {
         // Rebias the exponent.
         e += 127 - 15;
         m <<= (23 - 10);
      }  break;
   }

   //printf("f16_32(0x%04x) --> 0x%01x_%02x_%06x (0x%04x)\n", bits, s, e, m, mergeFloatBits( s, e, m ));
   return mergeFloatBits( s, e, m );
}

//------------------------------------------------------------------------------
//!
void
Half::splitHalfBits( uint32_t bits, uint32_t& s, uint32_t& e, uint32_t& m )
{
   m = bits & 0x3FF;
   bits >>= 10;
   e = bits & 0x1F;
   bits >>= 5;
   s = bits & 0x1;
}

//------------------------------------------------------------------------------
//!
uint32_t
Half::mergeHalfBits( uint32_t s, uint32_t e, uint32_t m )
{
   return (s<<15) | (e<<10) | m;
}

//------------------------------------------------------------------------------
//!
uint16_t
Half::toBits( float value )
{
   return f32_f16( NAMESPACE::toBits(value) );
}

//------------------------------------------------------------------------------
//!
float
Half::toFloat( uint16_t bits )
{
   return NAMESPACE::toFloat( f16_f32(bits) );
}


NAMESPACE_END

#endif //BASE_HALF_H
