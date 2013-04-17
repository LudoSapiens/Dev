/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_TRIG_H
#define CGMATH_TRIG_H

#include <CGMath/StdDefs.h>

#include <CGMath/CGConst.h>
#include <CGMath/Math.h>

NAMESPACE_BEGIN

namespace CGM
{

/*----- constants -----*/

const double DegToRad = 0.017453292519943295769236907684886;
const double RadToDeg = 57.295779513082320876798154814105;

/*----- functions -----*/

//------------------------------------------------------------------------------
//! Converts a number of circles into radians (1 cir = 2*Pi rads).
template< typename T > inline T
cirToRad( const T& cir )
{
   return cir*CGConst<T>::pi2();
}

//------------------------------------------------------------------------------
//! Converts a number of radians into circles (1 cir = 2*Pi rads).
template< typename T > inline T
radToCir( const T& rad )
{
   return rad*CGConst<T>::one_pi2();
}

//------------------------------------------------------------------------------
//! Converts degrees into radians (2*Pi rads = 360 degs).
template< typename T > inline T
degToRad( const T& deg )
{
   return deg*(T)DegToRad;
}

//------------------------------------------------------------------------------
//! Converts radians into degrees (2*Pi rads = 360 degs).
template< typename T > inline T
radToDeg( const T& rad )
{
   return rad*(T)RadToDeg;
}

//------------------------------------------------------------------------------
//! Convert an angle in radians to range [0, 2*Pi).
inline double
stdRad( const double rad )
{
   return rad - CGConst<double>::pi2() * CGM::floor( rad / CGConst<double>::pi2() );
}

//------------------------------------------------------------------------------
//! Convert an angle in radians to range [0, 2*Pi).
inline float
stdRad( const float rad )
{
   return rad - CGConst<float>::pi2() * CGM::floor( rad / CGConst<float>::pi2() );
}

//------------------------------------------------------------------------------
//! Convert an angle in degrees to range [0, 360).
inline double
stdDeg( const double deg )
{
   return deg - 360.0 * CGM::floor( deg / 360.0 );
}

//------------------------------------------------------------------------------
//! Convert an angle in degrees to range [0, 360).
inline float
stdDeg( const float deg )
{
   return deg - 360.0f * CGM::floor( deg / 360.0f );
}

//------------------------------------------------------------------------------
//! Convert an angle in circles to range [0, 1)
inline double
stdCir( const double cir )
{
   return CGM::fract( cir );
}

//------------------------------------------------------------------------------
//! Convert an angle in circles to range 0 - 1
inline float
stdCir( const float cir )
{
   return CGM::fract( cir );
}

//------------------------------------------------------------------------------
//! Clamps an angle in radians into the specified range in radians.
//! The following condition must hold:
//!   min <= max
template< typename T > inline void
clampRad( T& val, const T& min, const T& max )
{
   const T Pi2 = CGConst<T>::pi2();
   if( max-min >= Pi2 )
   {
      return;       // All the circle is in the range
   }

   T adjust = Pi2 * CGM::floor( (val-min) / Pi2 );
   min += adjust;
   max += adjust;

   if( max-min <= 0 )
   {
      val = min;    // A single angle in the range
      return;
   }

   // Following condition now holds:
   // min <= val <= min + Pi2
   if( val <= max )
   {
      // val is within range
      return;
   }

   // Val is more than max
   if( val-max < min+Pi2-val )
   {
      val = max;
   }
   else
   {
      val = min;
   }
}

//------------------------------------------------------------------------------
//! Returns the cosine of the specified radians value.
inline float
cos( const float rad )
{
   return ::cosf(rad);
}

//------------------------------------------------------------------------------
//! Returns the cosine of the specified radians value.
inline double
cos( const double rad )
{
   return ::cos(rad);
}

//------------------------------------------------------------------------------
//! Returns the sine of the specified radians value.
inline float
sin( const float rad )
{
   return ::sinf(rad);
}

//------------------------------------------------------------------------------
//! Returns the sine of the specified radians value.
inline double
sin( const double rad )
{
   return ::sin(rad);
}

//------------------------------------------------------------------------------
//! Returns the tangent of the specified radians value.
inline float
tan( const float rad )
{
   return ::tanf(rad);
}

//------------------------------------------------------------------------------
//! Returns the tangent of the specified radians value.
inline double
tan( const double rad )
{
   return ::tan(rad);
}

//------------------------------------------------------------------------------
//! Returns the arccosine of the specified value.
inline float
acos( const float x )
{
   return ::acosf(x);
}

//------------------------------------------------------------------------------
//! Returns the arccosine of the specified value.
inline double
acos( const double x )
{
   return ::acos(x);
}

//------------------------------------------------------------------------------
//! Returns the arcsine of the specified value.
inline float
asin( const float x )
{
   return ::asinf(x);
}

//------------------------------------------------------------------------------
//! Returns the arcsine of the specified value.
inline double
asin( const double x )
{
   return ::asin(x);
}

//------------------------------------------------------------------------------
//! Returns the arctangent of the specified value.
inline float
atan( const float x )
{
   return ::atanf(x);
}

//------------------------------------------------------------------------------
//! Returns the arctangent of the specified value.
inline double
atan( const double x )
{
   return ::atan(x);
}

//------------------------------------------------------------------------------
//! Returns the arctangent of the specified x and y values.
template< typename T > inline T
atan2( const T& y, const T& x )
{
   T ax = abs( x );
   T ay = abs( y );

   T r, phi, rSqr;
   if( ax > ay )
   {
      r = ay / ax;
      rSqr = r * r;
      phi = (T)-0.05030176425872175099 * ((T)-6.9888366207752135 + r) * ((T)3.14559995508649281e-7 + r)
          * ((T)2.84446368839622429 + (T)0.826399783297673451 * r + rSqr)
          / ((T)1.0 + (T)0.1471039133652469065841349249 * r + (T)0.644464067689154755092299698 * rSqr);
   }
   else
   if( ay != 0 )
   {
      r = ax / ay;
      rSqr = r * r;
      phi = (T)-0.05030176425872175099 * ((T)-6.9888366207752135 + r) * ((T)3.14559995508649281e-7 + r)
          * ((T)2.84446368839622429 + (T)0.826399783297673451 * r + rSqr)
          / ((T)1.0 + (T)0.1471039133652469065841349249 * r + (T)0.644464067689154755092299698 * rSqr);
      phi = CGConst<T>::pi_2() - phi;
   }
   else
   {
      // The standard math library has many more special cases.
      phi = (T)0;
   }
   // Check quadrant
   if( x >= (T)0.0 )
   {
      if( y < (T)0.0 )
      {
         phi = -phi;
      }
   }
   else
   {
      if( y >= (T)0.0 )
      {
         phi = CGConst<T>::pi() - phi;
      }
      else
      {
         phi = -CGConst<T>::pi() + phi;
      }
   }
   return phi;
}


} // namespace CGM

NAMESPACE_END

#endif //CGMATH_TRIG_H
