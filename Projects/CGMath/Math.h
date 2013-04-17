/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_MATH_H
#define CGMATH_MATH_H

#include <CGMath/StdDefs.h>

#include <cmath>
#include <cstdlib>

NAMESPACE_BEGIN

namespace CGM
{

/*----- constants -----*/

const double ln2      = 0.69314718055994530941723212145818;
const double inv_ln2  = 1.4426950408889634073599246810019;

const float EqualityThreshold = 1e-6f;
const float EqualityThresholdSqr = EqualityThreshold * EqualityThreshold;

// Need to undefine log2 that is defined in GCC's math.h.
#ifdef log2
#undef log2
#endif

/*----- functions -----*/

//=============================================================================
// Value swapping/clamping routines.
//=============================================================================

//------------------------------------------------------------------------------
//! Swaps a and b.
template< typename T > inline void
swap( T& a, T& b )
{
   T tmp = a;
   a = b;
   b = tmp;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
max( const T& a, const T& b )
{
   return b > a ? b : a;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
min( const T& a, const T& b )
{
   return b < a ? b : a;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
clampMin( T& val, const T& min )
{
   if( val < min )
   {
      val = min;
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
clampMax( T& val, const T& max )
{
   if( val > max )
   {
      val = max;
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
clamp( const T& val, const T& minVal, const T& maxVal )
{
#if 1
   return min( max( val, minVal ), maxVal );
#else
   if( val < minVal )
   {
      return minVal;
   }
   else
   if( val > maxVal )
   {
      return maxVal;
   }
   return val;
#endif
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
clampEq( T& val, const T& min, const T& max )
{
   if( val < min )
   {
      val = min;
   }
   else
   if( val > max )
   {
      val = max;
   }
}

//------------------------------------------------------------------------------
//! Returns whether or not the specified value is in the range [min, max].
template< typename T > inline bool
inRange( const T& val, const T& min, const T& max )
{
   return min <= val && val <= max;
}


//=============================================================================
// Sign-related routines.
//=============================================================================

//------------------------------------------------------------------------------
//! Returns the absolute value of the specified value.
template< typename T > inline T
abs( const T& val )
{
   return ( val < 0 ) ? -val : val;
}

//------------------------------------------------------------------------------
//! Returns the absolute value of the specified int value.
inline int
abs( const int val )
{
   return ::abs(val);
}

//------------------------------------------------------------------------------
//! Returns the absolute value of the specified float value.
inline float
abs( const float val )
{
   return ::fabsf(val);
}

//------------------------------------------------------------------------------
//! Returns the absolute value of the specified double value.
inline double
abs( const double val )
{
   return ::fabs(val);
}

//------------------------------------------------------------------------------
//! Returns the value of v with the sign of s.
template< typename T > inline T
copySign( const T& v, const T& s )
{
   return ( s < T(0) ) ? -abs(v) : abs(v);
}

//------------------------------------------------------------------------------
//! Returns the value of v with the sign of s.
inline float
copySign( const float v, const float s )
{
#if defined(_MSC_VER)
   return (float)_copysign(v, s);
#else
   return copysignf(v, s);
#endif
}

//------------------------------------------------------------------------------
//! Returns the value of v with the sign of s.
inline double
copySign( const double v, const double s )
{
#if defined(_MSC_VER)
   return _copysign(v, s);
#else
   return copysign(v, s);
#endif
}

//------------------------------------------------------------------------------
//! Returns -1 if val < 0, 0 if val == 0, or +1 if val > 0.
template< typename T > inline T
sign( const T& val )
{
   return val == T(0) ? T(0) : copySign( T(1), val );
}

//------------------------------------------------------------------------------
//! Converts a potentially non-power-of-2 value into the smallest power-of-2
//! value containing it.
inline uint
nextPow2( uint val )
{
   // Convert a pow2 to a non-pow2
   --val;
   // Push high order bit down until bit 0.
   val |= (val >>  1); //0x80000000 --> 0xC0000000
   val |= (val >>  2); //0xC0000000 --> 0xF0000000
   val |= (val >>  4); //0xF0000000 --> 0xFF000000
   val |= (val >>  8); //0xFFFF0000 --> 0xFFFF0000
   val |= (val >> 16); //0xFFFFFFFF --> 0xFFFFFFFF
   // Convert back to pow2
   ++val;
   return val;
}

//------------------------------------------------------------------------------
//! Returns whether or not the specified values are equal within a certain
//! error margin.
inline bool
equal( float a, float b, float threshold = (float)EqualityThreshold )
{
   return CGM::abs(a - b) <= threshold;
}

//------------------------------------------------------------------------------
//! 
inline bool
equal( double a, double b, double threshold = (double)EqualityThreshold )
{
   return CGM::abs(a - b) <= threshold;
}


//=============================================================================
// Integer/fraction-related routines.
//=============================================================================

//------------------------------------------------------------------------------
//! Returns the largest integer value x such that x <= val.
template< typename T > inline T
floor( const T& val )
{
   return ::floor(val);
}

//------------------------------------------------------------------------------
//! Returns the smallest integer value x such that x >= val.
template< typename T > inline T
ceil( T val )
{
   return ::ceil(val);
}

//------------------------------------------------------------------------------
//! Returns the integer value x such that abs(x - val) <= 0.5.
//! Half-way cases round away from zero.
template< typename T > inline T
round( const T& val )
{
#if _MSC_VER >= 1600
   return copySign( floor( abs(val)+T(0.5) ), val );
#else
   return ::round(val);
#endif
}

//------------------------------------------------------------------------------
//! Returns the largest integer value x such that x <= val.
template< typename T > inline int
floori( T val )
{
   return (int)::floor(val);
}

//------------------------------------------------------------------------------
//! Returns the largest integer value x such that x >= val.
template< typename T > inline int
ceili( const T val )
{
   return (int)::ceil(val);
}

//------------------------------------------------------------------------------
//! Returns the integer value x such that abs(x - val) <= 0.5.
//! Half-way cases round away from zero.
template< typename T > inline int
roundi( const T& val )
{
   return ::lround(val);
}

//------------------------------------------------------------------------------
//! Splits the specified value into an integer portion (stored where iptr points
//! to) and a fractional part (returned).
inline float
mod( const float val, float* iptr )
{
   return ::modff(val, iptr);
}

//------------------------------------------------------------------------------
//! Splits the specified value into an integer portion (stored where iptr points
//! to) and a fractional part (returned).
inline double
mod( double x, double* iptr )
{
   return ::modf(x, iptr);
}

//------------------------------------------------------------------------------
//! Returns the signed remainder of performing x/y.
inline float
fmod( const float x, const float y )
{
   return ::fmodf(x, y);
}

//------------------------------------------------------------------------------
//! Returns the signed remainder of performing x/y.
inline double
fmod( const double x, const double y )
{
   return ::fmod(x, y);
}

//------------------------------------------------------------------------------
//! A true modulo operation, never giving negative values.
inline int
modulo( int value, int divisor )
{
   return ((value % divisor) + divisor) % divisor;
}

//------------------------------------------------------------------------------
//! Returns val - floor(val).
template< typename T > inline T
fract( const T& val )
{
   return val - CGM::floor(val);
}

//------------------------------------------------------------------------------
//! Splits the val into an integer and fractional parts such that
//!   i <= val
//!   i+f = val (except for some precision corner cases)
//! This routine is meant to be a shorthand of calling the following:
//!   i = floor(val)
//!   f = val - i
//! and can potentially be faster (and more accurate) if i's type is different to f's.
template< typename T, typename S > inline void
splitIntFrac( const T& val, S& i, T& f )
{
   T tmp = CGM::floor(val);
   i = (S)tmp;
   f = val - tmp;
}

inline float pow( const float b, const float e );  // forward declaration.

//------------------------------------------------------------------------------
//! Snaps precision (round-to-nearest-even) up to the specified number of fractional bits.
//! Only valid for 0 to 32 fractional bits.
inline float
fixedPrec( float v, uint nFracBits = 24 )
{
   static const float _fixedPrecOffset[] =
   {
      //CGM::pow( 2.0f, 24.0f ),
      CGM::pow( 2.0f, 23.0f ), // 0 fractional bits.
      CGM::pow( 2.0f, 22.0f ),
      CGM::pow( 2.0f, 21.0f ),
      CGM::pow( 2.0f, 20.0f ),
      CGM::pow( 2.0f, 19.0f ),
      CGM::pow( 2.0f, 18.0f ),
      CGM::pow( 2.0f, 17.0f ),
      CGM::pow( 2.0f, 16.0f ),
      CGM::pow( 2.0f, 15.0f ),
      CGM::pow( 2.0f, 14.0f ),
      CGM::pow( 2.0f, 13.0f ), // 10 fractional bits.
      CGM::pow( 2.0f, 12.0f ),
      CGM::pow( 2.0f, 11.0f ),
      CGM::pow( 2.0f, 10.0f ),
      CGM::pow( 2.0f,  9.0f ),
      CGM::pow( 2.0f,  8.0f ),
      CGM::pow( 2.0f,  7.0f ),
      CGM::pow( 2.0f,  6.0f ),
      CGM::pow( 2.0f,  5.0f ),
      CGM::pow( 2.0f,  4.0f ),
      CGM::pow( 2.0f,  3.0f ), // 20 fractional bits.
      CGM::pow( 2.0f,  2.0f ),
      CGM::pow( 2.0f,  1.0f ),
      CGM::pow( 2.0f,  0.0f ),
      CGM::pow( 2.0f, -1.0f ),
      CGM::pow( 2.0f, -2.0f ),
      CGM::pow( 2.0f, -3.0f ),
      CGM::pow( 2.0f, -4.0f ),
      CGM::pow( 2.0f, -5.0f ),
      CGM::pow( 2.0f, -6.0f ),
      CGM::pow( 2.0f, -7.0f ), // 30 fractional bits.
      CGM::pow( 2.0f, -8.0f ),
      CGM::pow( 2.0f, -9.0f ), // 32 fractional bits.
   };

   const float fpo = CGM::copySign( _fixedPrecOffset[nFracBits], v );
   v += fpo;
   v -= fpo;
   return v;
}


//=============================================================================
// Exponential-related routines.
//=============================================================================

//------------------------------------------------------------------------------
//! Returns the logarithm base-2 of the specified value.
inline float
log2( const float val )
{
   return ::logf(val) * (float)inv_ln2;
}

//------------------------------------------------------------------------------
//! Returns the logarithm base-2 of the specified value.
inline double
log2( const double val )
{
   return ::log(val) * inv_ln2;
}

//------------------------------------------------------------------------------
//! Returns the natural logarithm (base-e) of the specified value.
inline float
ln( const float val )
{
   return ::logf(val);
}

//------------------------------------------------------------------------------
//! Returns the natural logarithm (base-e) of the specified value.
inline double
ln( const double val )
{
   return ::log(val);
}

//------------------------------------------------------------------------------
//! Returns the logarithm base-10 of the specified value.
inline float
log10( const float val )
{
   return ::log10f(val);
}

//------------------------------------------------------------------------------
//! Returns the logarithm base-10 of the specified value.
inline double
log10( const double val )
{
   return ::log10(val);
}

//------------------------------------------------------------------------------
//! Returns the square root of the specified value.
inline float
sqrt( float val )
{
   return ::sqrtf(val);
}

//------------------------------------------------------------------------------
//! Returns the square root of the specified value.
inline double
sqrt( double val )
{
   return ::sqrt(val);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
invSqrt( const T& v )
{
   return T(1) / sqrt(v);
}

//------------------------------------------------------------------------------
//! Returns b^e.
inline float
pow( const float b, const float e )
{
   return ::powf(b, e);
}

//------------------------------------------------------------------------------
//! Returns b^e.
inline double
pow( const double b, const double e )
{
   return ::pow(b, e);
}

//------------------------------------------------------------------------------
//! Returns 2^e.
inline float
pow2( const float e )
{
#if defined(_MSC_VER)
   return ::powf(2.0f, e);
#else
   return ::exp2(e);
#endif
}

//------------------------------------------------------------------------------
//! Returns 2^e.
inline double
pow2( const double e )
{
#if defined(_MSC_VER)
   return ::pow(2.0, e);
#else
   return ::exp2(e);
#endif
}



} // namespace CGM

NAMESPACE_END

#endif //CGMATH_MATH_H
