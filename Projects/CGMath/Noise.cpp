/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <CGMath/Noise.h>

#include <CGMath/CGConst.h>
#include <CGMath/Geom.h>
#include <CGMath/Math.h>

#include <Base/Dbg/Defs.h>
#include <Base/IO/TextStream.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//=============================================================================
// Perlin Noise.
//=============================================================================

typedef uint  Idx_t;

#ifdef _P
#undef _P
#endif
//------------------------------------------------------------------------------
//! A static table containing value [0, 255] in random order.
//! We copy them a second time to avoid handling overflows in the code.
static Idx_t  _P[512] = {
   // These are the 256 values used by Perlin in his reference implementation.
   151,160,137, 91, 90, 15,131, 13,201, 95, 96, 53,194,233,  7,225,
   140, 36,103, 30, 69,142,  8, 99, 37,240, 21, 10, 23,190,  6,148,
   247,120,234, 75,  0, 26,197, 62, 94,252,219,203,117, 35, 11, 32,
    57,177, 33, 88,237,149, 56, 87,174, 20,125,136,171,168, 68,175,
    74,165, 71,134,139, 48, 27,166, 77,146,158,231, 83,111,229,122,
    60,211,133,230,220,105, 92, 41, 55, 46,245, 40,244,102,143, 54,
    65, 25, 63,161,  1,216, 80, 73,209, 76,132,187,208, 89, 18,169,
   200,196,135,130,116,188,159, 86,164,100,109,198,173,186,  3, 64,
    52,217,226,250,124,123,  5,202, 38,147,118,126,255, 82, 85,212,
   207,206, 59,227, 47, 16, 58, 17,182,189, 28, 42,223,183,170,213,
   119,248,152,  2, 44,154,163, 70,221,153,101,155,167, 43,172,  9,
   129, 22, 39,253, 19, 98,108,110, 79,113,224,232,178,185,112,104,
   218,246, 97,228,251, 34,242,193,238,210,144, 12,191,179,162,241,
    81, 51,145,235,249, 14,239,107, 49,192,214, 31,181,199,106,157,
   184, 84,204,176,115,121, 50, 45,127,  4,150,254,138,236,205, 93,
   222,114, 67, 29, 24, 72,243,141,128,195, 78, 66,215, 61,156,180,
   // Copy of the 256 values above.
   151,160,137, 91, 90, 15,131, 13,201, 95, 96, 53,194,233,  7,225,
   140, 36,103, 30, 69,142,  8, 99, 37,240, 21, 10, 23,190,  6,148,
   247,120,234, 75,  0, 26,197, 62, 94,252,219,203,117, 35, 11, 32,
    57,177, 33, 88,237,149, 56, 87,174, 20,125,136,171,168, 68,175,
    74,165, 71,134,139, 48, 27,166, 77,146,158,231, 83,111,229,122,
    60,211,133,230,220,105, 92, 41, 55, 46,245, 40,244,102,143, 54,
    65, 25, 63,161,  1,216, 80, 73,209, 76,132,187,208, 89, 18,169,
   200,196,135,130,116,188,159, 86,164,100,109,198,173,186,  3, 64,
    52,217,226,250,124,123,  5,202, 38,147,118,126,255, 82, 85,212,
   207,206, 59,227, 47, 16, 58, 17,182,189, 28, 42,223,183,170,213,
   119,248,152,  2, 44,154,163, 70,221,153,101,155,167, 43,172,  9,
   129, 22, 39,253, 19, 98,108,110, 79,113,224,232,178,185,112,104,
   218,246, 97,228,251, 34,242,193,238,210,144, 12,191,179,162,241,
    81, 51,145,235,249, 14,239,107, 49,192,214, 31,181,199,106,157,
   184, 84,204,176,115,121, 50, 45,127,  4,150,254,138,236,205, 93,
   222,114, 67, 29, 24, 72,243,141,128,195, 78, 66,215, 61,156,180
};

//------------------------------------------------------------------------------
//! Performs both a conversion of a hash into one of 16 hard-coded directions
//! and does the dot product of that vector with the specified vector which is
//! actually its opposite direction (yeah, Perlin optimized that much!).
//! Here is a rundown of the temporary variables that Perlin used in his code:
//!   hash   vector    u v    final
//!   [ 0] ( 1, 1, 0)  x y    x + y
//!   [ 1] (-1, 1, 0)  x y   -x + y
//!   [ 2] ( 1,-1, 0)  x y    x +-y
//!   [ 3] (-1,-1, 0)  x y   -x +-y
//!   [ 4] ( 1, 0, 1)  x z    x + z
//!   [ 5] (-1, 0, 1)  x z   -x + z
//!   [ 6] ( 1, 0,-1)  x z    x +-z
//!   [ 7] (-1, 0,-1)  x z   -x +-z
//!   [ 8] ( 0, 1, 1)  y z    y + z
//!   [ 9] ( 0,-1, 1)  y z   -y + z
//!   [10] ( 0, 1,-1)  y z    y +-z
//!   [11] ( 0,-1,-1)  y z   -y +-z
//!   [12] ( 1, 1, 0)  y x    y + x
//!   [13] ( 0,-1, 1)  y z   -y + z
//!   [14] (-1, 1, 0)  y x    y +-x
//!   [15] ( 0,-1,-1)  y z   -y +-z
inline float grad( Idx_t hash, float x, float y, float z )
{
#if 0
   hash &= 0x0F;
   float u = (hash < 8) ? x : y;
   float v = (hash < 4) ? y : (hash==12||hash==14) ? x : z;
   return ((hash&0x01)==0 ? u : -u) + ((hash&0x02)==0 ? v : -v);
#else
   switch( hash & 0x0F )
   {                             // Vector doing a dot product.
      case 0x00: return  x + y;  // ( 1, 1, 0)
      case 0x01: return -x + y;  // (-1, 1, 0)
      case 0x02: return  x - y;  // ( 1,-1, 0)
      case 0x03: return -x - y;  // (-1,-1, 0)
      case 0x04: return  x + z;  // ( 1, 0, 1)
      case 0x05: return -x + z;  // (-1, 0, 1)
      case 0x06: return  x - z;  // ( 1, 0,-1)
      case 0x07: return -x - z;  // (-1, 0,-1)
      case 0x08: return  y + z;  // ( 0, 1, 1)
      case 0x09: return -y + z;  // ( 0,-1, 1)
      case 0x0A: return  y - z;  // ( 0, 1,-1)
      case 0x0B: return -y - z;  // ( 0,-1,-1)
      case 0x0C: return  y + x;  // ( 1, 1, 0)
      case 0x0D: return -y + z;  // ( 0,-1, 1)
      case 0x0E: return  y - x;  // (-1, 1, 0)
      case 0x0F: return -y - z;  // ( 0,-1,-1)
      default  : CHECK( false );
                 return CGConstf::NaN();
   }
#endif
}

UNNAMESPACE_END

NAMESPACE_BEGIN

namespace CGM
{

//------------------------------------------------------------------------------
//!
float  cellNoise1( int x, int y, int z )
{
#if 0
   // LibNoise implementation.
   uint hash =  x*1619 + y*31337 + z*6971 + 1013;
   hash &= 0x7FFFFFFF;
   hash = (hash >> 13) ^ hash;
   hash = (hash * (hash * hash * 60493 + 19990303) + 1376312589);
   //return 1.0f - (hash & 0x7FFFFFFF)/1073741824.0f; // Returns a 32b precision number in range (-1, 1).
   return (hash & 0x7FFFFFFF)/2147483648.0f;          // Returns a 31b precision number in range [0, 1).
   //return hash/4294967296.0f;                       // Returns a 32b precision number in range [0, 1).
   //return (hash & 0x00FFFFFF)/16777216.0f;          // Returns a 24b precision number in range [0, 1).
   //return (hash & 0xFFFF)/65536.0f;                 // Returns a 16b precision number in range [0, 1).
#else
   // Reuse the _P that is used for Perlin noise.
   // This technique has the advantage of being implementable in GPU by:
   //   - Reusing the noise table
   //   - Converting the shifts into adds/mulls/fract.
   //   - Using wrap texture mode to take care of _P[v & 0xFF].
   // We only implemented 16 bits of precision (note: Perlin only handles 8).
   Idx_t a, b;
   a = _P[ (  x) & 0xFF ];
   a = _P[ (a+y) & 0xFF ];
   a = _P[ (a+z) & 0xFF ];
   x >>= 8; y >>= 8; z >>= 8;
   b = _P[ (a+x) & 0xFF ];
   b = _P[ (b+y) & 0xFF ];
   b = _P[ (b+z) & 0xFF ];
   // Use 'a' as the upper bits, since the LSBs of x,y,z have more entropy.
   return ((a<<8) + b)/65536.0f;  // Returns a 16b precision number in range [0, 1).

#endif
}

//------------------------------------------------------------------------------
//!
Vec2f  cellNoise2( int x, int y, int z )
{
   // See cellNoise1() for comments.
   Vec2f tmp;

   Idx_t a, b;
   a = _P[ (  x) & 0xFF ];
   a = _P[ (a+y) & 0xFF ];
   a = _P[ (a+z) & 0xFF ];
   x >>= 8; y >>= 8; z >>= 8;
   b = _P[ (a+x) & 0xFF ];
   b = _P[ (b+y) & 0xFF ];
   b = _P[ (b+z) & 0xFF ];
   // Use 'a' as the upper bits, since the LSBs of x,y,z have more entropy.
   tmp.x = ((a<<8) + b)/65536.0f; // Convert into a 16b precision number in range [0, 1).

   // Since h is already random, use it as a seed for the next random.
   a = _P[a];
   b = _P[b];
   tmp.y = ((a<<8) + b)/65536.0f; // Convert into a 16b precision number in range [0, 1).

   return tmp;
}

//------------------------------------------------------------------------------
//!
Vec3f  cellNoise3( int x, int y, int z )
{
   // See cellNoise1() for comments.
   Vec3f tmp;

   Idx_t a, b;
   a = _P[ (  x) & 0xFF ];
   a = _P[ (a+y) & 0xFF ];
   a = _P[ (a+z) & 0xFF ];
   x >>= 8; y >>= 8; z >>= 8;
   b = _P[ (a+x) & 0xFF ];
   b = _P[ (b+y) & 0xFF ];
   b = _P[ (b+z) & 0xFF ];
   // Use 'a' as the upper bits, since the LSBs of x,y,z have more entropy.
   tmp.x = ((a<<8) + b)/65536.0f; // Convert into a 16b precision number in range [0, 1).

   // Since h is already random, use it as a seed for the next random.
   a = _P[a];
   b = _P[b];
   tmp.y = ((a<<8) + b)/65536.0f; // Convert into a 16b precision number in range [0, 1).

   a = _P[a];
   b = _P[b];
   tmp.z = ((a<<8) + b)/65536.0f; // Convert into a 16b precision number in range [0, 1).

   return tmp;
}

//------------------------------------------------------------------------------
//!
float  perlinNoise1( const Vec3f& p )
{
   // Convert the {x,y,z} position into an limited range {xi,yi,zi} index with {u, v, w} factors.
   float xf = CGM::floor( p.x );
   float yf = CGM::floor( p.y );
   float zf = CGM::floor( p.z );
   Idx_t xi = (int)xf & 0xFF;
   Idx_t yi = (int)yf & 0xFF;
   Idx_t zi = (int)zf & 0xFF;
   float x = p.x - xf;
   float y = p.y - yf;
   float z = p.z - zf;
   float u = CGM::smoothRampC2( x );
   float v = CGM::smoothRampC2( y );
   float w = CGM::smoothRampC2( z );

   // Retrieve pseudo-random corner indices.
   Idx_t A  = (_P[xi  ] + yi); //& 0xFF;
   Idx_t AA = (_P[A   ] + zi); //& 0xFF;
   Idx_t AB = (_P[A+1 ] + zi); //& 0xFF;
   Idx_t B  = (_P[xi+1] + yi); //& 0xFF;
   Idx_t BA = (_P[B   ] + zi); //& 0xFF;
   Idx_t BB = (_P[B+1 ] + zi); //& 0xFF;

   // Compute the scalars for each corner of the cube.
   //     ZYX
   float _000 = grad( _P[AA  ] /*& 0xFF*/, x  , y  , z   );
   float _001 = grad( _P[BA  ] /*& 0xFF*/, x-1, y  , z   );
   float _010 = grad( _P[AB  ] /*& 0xFF*/, x  , y-1, z   );
   float _011 = grad( _P[BB  ] /*& 0xFF*/, x-1, y-1, z   );
   float _100 = grad( _P[AA+1] /*& 0xFF*/, x  , y  , z-1 );
   float _101 = grad( _P[BA+1] /*& 0xFF*/, x-1, y  , z-1 );
   float _110 = grad( _P[AB+1] /*& 0xFF*/, x  , y-1, z-1 );
   float _111 = grad( _P[BB+1] /*& 0xFF*/, x-1, y-1, z-1 );

   //StdErr << "Z=0 " << _000 << "," << _001 << "," << _010 << "," << _011 << nl;
   //StdErr << "Z=1 " << _100 << "," << _101 << "," << _110 << "," << _111 << nl;
   //StdErr << "uvw=" << u << "," << v << "," << w << nl;

   // Trilinearly interpolate the corners using the computed factors.
   return CGM::trilinear( _000, _001 - _000,
                          _010, _011 - _010,
                          _100, _101 - _100,
                          _110, _111 - _110,
                          u, v, w );
}

//------------------------------------------------------------------------------
//!
Vec2f  perlinNoise2( const Vec3f& p )
{
   Vec2f tmp;

   // Convert the {x,y,z} position into an limited range {xi,yi,zi} index with {u, v, w} factors.
   float xf = CGM::floor( p.x );
   float yf = CGM::floor( p.y );
   float zf = CGM::floor( p.z );
   Idx_t xi = (int)xf & 0xFF;
   Idx_t yi = (int)yf & 0xFF;
   Idx_t zi = (int)zf & 0xFF;
   float x = p.x - xf;
   float y = p.y - yf;
   float z = p.z - zf;
   float u = CGM::smoothRampC2( x );
   float v = CGM::smoothRampC2( y );
   float w = CGM::smoothRampC2( z );

   // Retrieve pseudo-random corner indices.
   Idx_t A  = (_P[xi  ] + yi); //& 0xFF;
   Idx_t AA = (_P[A   ] + zi); //& 0xFF;
   Idx_t AB = (_P[A+1 ] + zi); //& 0xFF;
   Idx_t B  = (_P[xi+1] + yi); //& 0xFF;
   Idx_t BA = (_P[B   ] + zi); //& 0xFF;
   Idx_t BB = (_P[B+1 ] + zi); //& 0xFF;

   // Compute the scalars for each corner of the cube.
   //     ZYX
   float _000 = grad( _P[AA  ] /*& 0xFF*/, x  , y  , z   );
   float _001 = grad( _P[BA  ] /*& 0xFF*/, x-1, y  , z   );
   float _010 = grad( _P[AB  ] /*& 0xFF*/, x  , y-1, z   );
   float _011 = grad( _P[BB  ] /*& 0xFF*/, x-1, y-1, z   );
   float _100 = grad( _P[AA+1] /*& 0xFF*/, x  , y  , z-1 );
   float _101 = grad( _P[BA+1] /*& 0xFF*/, x-1, y  , z-1 );
   float _110 = grad( _P[AB+1] /*& 0xFF*/, x  , y-1, z-1 );
   float _111 = grad( _P[BB+1] /*& 0xFF*/, x-1, y-1, z-1 );

   //StdErr << "Z=0 " << _000 << "," << _001 << "," << _010 << "," << _011 << nl;
   //StdErr << "Z=1 " << _100 << "," << _101 << "," << _110 << "," << _111 << nl;
   //StdErr << "uvw=" << u << "," << v << "," << w << nl;

   // Trilinearly interpolate the corners using the computed factors.
   tmp.x = CGM::trilinear( _000, _001 - _000,
                           _010, _011 - _010,
                           _100, _101 - _100,
                           _110, _111 - _110,
                           u, v, w );

   // Retrieve pseudo-random corner indices.
   A  = (_P[(xi  +13)     ] + yi); //& 0xFF;
   AA = (_P[(A   +13)&0xFF] + zi); //& 0xFF;
   AB = (_P[(A+1 +13)&0xFF] + zi); //& 0xFF;
   B  = (_P[(xi+1+13)&0xFF] + yi); //& 0xFF;
   BA = (_P[(B   +13)&0xFF] + zi); //& 0xFF;
   BB = (_P[(B+1 +13)&0xFF] + zi); //& 0xFF;

   // Compute the scalars for each corner of the cube.
   //     ZYX
   _000 = grad( _P[AA  ] /*& 0xFF*/, x  , y  , z   );
   _001 = grad( _P[BA  ] /*& 0xFF*/, x-1, y  , z   );
   _010 = grad( _P[AB  ] /*& 0xFF*/, x  , y-1, z   );
   _011 = grad( _P[BB  ] /*& 0xFF*/, x-1, y-1, z   );
   _100 = grad( _P[AA+1] /*& 0xFF*/, x  , y  , z-1 );
   _101 = grad( _P[BA+1] /*& 0xFF*/, x-1, y  , z-1 );
   _110 = grad( _P[AB+1] /*& 0xFF*/, x  , y-1, z-1 );
   _111 = grad( _P[BB+1] /*& 0xFF*/, x-1, y-1, z-1 );

   // Trilinearly interpolate the corners using the computed factors.
   tmp.y = CGM::trilinear( _000, _001 - _000,
                           _010, _011 - _010,
                           _100, _101 - _100,
                           _110, _111 - _110,
                           u, v, w );

   return tmp;
}

//------------------------------------------------------------------------------
//!
Vec3f  perlinNoise3( const Vec3f& p )
{
   Vec3f tmp;

   // Convert the {x,y,z} position into an limited range {xi,yi,zi} index with {u, v, w} factors.
   float xf = CGM::floor( p.x );
   float yf = CGM::floor( p.y );
   float zf = CGM::floor( p.z );
   Idx_t xi = (int)xf & 0xFF;
   Idx_t yi = (int)yf & 0xFF;
   Idx_t zi = (int)zf & 0xFF;
   float x = p.x - xf;
   float y = p.y - yf;
   float z = p.z - zf;
   float u = CGM::smoothRampC2( x );
   float v = CGM::smoothRampC2( y );
   float w = CGM::smoothRampC2( z );

   // Retrieve pseudo-random corner indices.
   Idx_t A  = (_P[xi  ] + yi); //& 0xFF;
   Idx_t AA = (_P[A   ] + zi); //& 0xFF;
   Idx_t AB = (_P[A+1 ] + zi); //& 0xFF;
   Idx_t B  = (_P[xi+1] + yi); //& 0xFF;
   Idx_t BA = (_P[B   ] + zi); //& 0xFF;
   Idx_t BB = (_P[B+1 ] + zi); //& 0xFF;

   // Compute the scalars for each corner of the cube.
   //     ZYX
   float _000 = grad( _P[AA  ] /*& 0xFF*/, x  , y  , z   );
   float _001 = grad( _P[BA  ] /*& 0xFF*/, x-1, y  , z   );
   float _010 = grad( _P[AB  ] /*& 0xFF*/, x  , y-1, z   );
   float _011 = grad( _P[BB  ] /*& 0xFF*/, x-1, y-1, z   );
   float _100 = grad( _P[AA+1] /*& 0xFF*/, x  , y  , z-1 );
   float _101 = grad( _P[BA+1] /*& 0xFF*/, x-1, y  , z-1 );
   float _110 = grad( _P[AB+1] /*& 0xFF*/, x  , y-1, z-1 );
   float _111 = grad( _P[BB+1] /*& 0xFF*/, x-1, y-1, z-1 );

   //StdErr << "Z=0 " << _000 << "," << _001 << "," << _010 << "," << _011 << nl;
   //StdErr << "Z=1 " << _100 << "," << _101 << "," << _110 << "," << _111 << nl;
   //StdErr << "uvw=" << u << "," << v << "," << w << nl;

   // Trilinearly interpolate the corners using the computed factors.
   tmp.x = CGM::trilinear( _000, _001 - _000,
                           _010, _011 - _010,
                           _100, _101 - _100,
                           _110, _111 - _110,
                           u, v, w );

   // Retrieve pseudo-random corner indices.
   A  = (_P[(xi  +13)     ] + yi); //& 0xFF;
   AA = (_P[(A   +13)&0xFF] + zi); //& 0xFF;
   AB = (_P[(A+1 +13)&0xFF] + zi); //& 0xFF;
   B  = (_P[(xi+1+13)&0xFF] + yi); //& 0xFF;
   BA = (_P[(B   +13)&0xFF] + zi); //& 0xFF;
   BB = (_P[(B+1 +13)&0xFF] + zi); //& 0xFF;

   // Compute the scalars for each corner of the cube.
   //     ZYX
   _000 = grad( _P[AA  ] /*& 0xFF*/, x  , y  , z   );
   _001 = grad( _P[BA  ] /*& 0xFF*/, x-1, y  , z   );
   _010 = grad( _P[AB  ] /*& 0xFF*/, x  , y-1, z   );
   _011 = grad( _P[BB  ] /*& 0xFF*/, x-1, y-1, z   );
   _100 = grad( _P[AA+1] /*& 0xFF*/, x  , y  , z-1 );
   _101 = grad( _P[BA+1] /*& 0xFF*/, x-1, y  , z-1 );
   _110 = grad( _P[AB+1] /*& 0xFF*/, x  , y-1, z-1 );
   _111 = grad( _P[BB+1] /*& 0xFF*/, x-1, y-1, z-1 );

   // Trilinearly interpolate the corners using the computed factors.
   tmp.y = CGM::trilinear( _000, _001 - _000,
                           _010, _011 - _010,
                           _100, _101 - _100,
                           _110, _111 - _110,
                           u, v, w );

   // Retrieve pseudo-random corner indices.
   A  = (_P[(xi  +27)     ] + yi); //& 0xFF;
   AA = (_P[(A   +27)&0xFF] + zi); //& 0xFF;
   AB = (_P[(A+1 +27)&0xFF] + zi); //& 0xFF;
   B  = (_P[(xi+1+27)&0xFF] + yi); //& 0xFF;
   BA = (_P[(B   +27)&0xFF] + zi); //& 0xFF;
   BB = (_P[(B+1 +27)&0xFF] + zi); //& 0xFF;

   // Compute the scalars for each corner of the cube.
   //     ZYX
   _000 = grad( _P[AA  ] /*& 0xFF*/, x  , y  , z   );
   _001 = grad( _P[BA  ] /*& 0xFF*/, x-1, y  , z   );
   _010 = grad( _P[AB  ] /*& 0xFF*/, x  , y-1, z   );
   _011 = grad( _P[BB  ] /*& 0xFF*/, x-1, y-1, z   );
   _100 = grad( _P[AA+1] /*& 0xFF*/, x  , y  , z-1 );
   _101 = grad( _P[BA+1] /*& 0xFF*/, x-1, y  , z-1 );
   _110 = grad( _P[AB+1] /*& 0xFF*/, x  , y-1, z-1 );
   _111 = grad( _P[BB+1] /*& 0xFF*/, x-1, y-1, z-1 );

   // Trilinearly interpolate the corners using the computed factors.
   tmp.z = CGM::trilinear( _000, _001 - _000,
                           _010, _011 - _010,
                           _100, _101 - _100,
                           _110, _111 - _110,
                           u, v, w );

   return tmp;
}

//------------------------------------------------------------------------------
//!
float  voronoiNoise1( const Vec3f& p, const float jitter, Vec3f& p1 )
{
   Vec3f cellCenter = Vec3f( CGM::floor(p.x) + 0.5f, CGM::floor(p.y) + 0.5f, CGM::floor(p.z) + 0.5f );
   float f1 = CGConstf::infinity();
   Vec3f d;
   for( d.z = -1.0f; d.z <= 1.0f; ++d.z )
   {
      for( d.y = -1.0f; d.y <= 1.0f; ++d.y )
      {
         for( d.x = -1.0f; d.x <= 1.0f; ++d.x )
         {
            Vec3f testCenter = cellCenter + d;
            Vec3f testPos    = testCenter + (cellNoise1(testCenter) - 0.5f) * jitter;
            Vec3f offset     = testPos - p;
            float dist = CGM::sqrLength( offset );
            if( dist < f1 )
            {
               f1 = dist;
               p1 = testPos;
            }
         }
      }
   }
   return CGM::sqrt( f1 );
}

//------------------------------------------------------------------------------
//!
Vec2f  voronoiNoise2( const Vec3f& p, const float jitter, Vec3f& p1, Vec3f& p2 )
{
   Vec3f cellCenter = Vec3f( CGM::floor(p.x) + 0.5f, CGM::floor(p.y) + 0.5f, CGM::floor(p.z) + 0.5f );
   float f1 = CGConstf::infinity();
   float f2 = CGConstf::infinity();
   Vec3f d;
   for( d.z = -1.0f; d.z <= 1.0f; ++d.z )
   {
      for( d.y = -1.0f; d.y <= 1.0f; ++d.y )
      {
         for( d.x = -1.0f; d.x <= 1.0f; ++d.x )
         {
            Vec3f testCenter = cellCenter + d;
            Vec3f testPos    = testCenter + (cellNoise1(testCenter) - 0.5f) * jitter;
            Vec3f offset     = testPos - p;
            float dist = CGM::sqrLength( offset );
            if( dist < f1 )
            {
               f2 = f1;
               p2 = p1;
               f1 = dist;
               p1 = testPos;
            }
            else
            if( dist < f2 )
            {
               f2 = dist;
               p2 = testPos;
            }
         }
      }
   }
   return Vec2f( CGM::sqrt(f1), CGM::sqrt(f2) );
}

//------------------------------------------------------------------------------
//!
float  fBmNoise1(
   const Vec3f& p,
   const float  width,
   const uint   octaves,
   const float  lacunarity,
   const float  gain
)
{
   float sum = 0.0f;
   Vec3f pp = p;
   float amp = 1.0f;
   float fw  = width;

   for( uint o = 0; o < octaves; ++o )
   {
      sum += filteredPerlinNoise1( pp, fw ) * amp;
      amp *= gain;
      pp  *= lacunarity;
      fw  *= lacunarity;
   }

   return sum;
}

} // namespace CGM

NAMESPACE_END
