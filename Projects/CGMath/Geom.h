/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_GEOM_H
#define CGMATH_GEOM_H

#include <CGMath/StdDefs.h>

#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN

namespace CGM
{

/*----- constants -----*/


/*----- functions -----*/

//------------------------------------------------------------------------------
//! Performs linear interpolation between 2 values specified as a position
//! and a distance to the other position.
template< typename T, typename S > inline T
linear(
   const T& v0, const T& d10,
   const S& u
)
{
   return v0 + d10*u;
}

//------------------------------------------------------------------------------
//! Perform linear interpolation between 2 values.
template< typename T, typename S > inline T
linear2( const T& v0, const T& v1, const S& u )
{
   return v0 + (v1-v0)*u;
}

//------------------------------------------------------------------------------
//! Performs bilinear interpolation between 4 values specified as 2 corners
//! and 2 distances to the other 2 corners.
//! Vertex order:
//!    2-------3
//!    |       |
//!  v |       |
//!    |       |
//!    0-------1
//!        u
template< typename T, typename S > inline T
bilinear(
   const T& v0, const T& d10,
   const T& v2, const T& d32,
   const S& u,
   const S& v
)
{
   T v01 = v0 + d10*u;
   T v23 = v2 + d32*u;
   return v01 + ( v23-v01 )*v;
}

//------------------------------------------------------------------------------
//! Performs trilinear interpolation between 8 values specified as 4 corners
//! and 4 distances to the other 4 corners.
//! Vertex order:
//!      2-------3
//!     /|      /|
//!    / |     / |
//!   6--+----7  |
//!   |  0----|--1
//! v | /     | /
//!   |/      |/ w
//!   4-------5
//!       u
template< typename T, typename S > inline T
trilinear(
   const T& v0, const T& d10,
   const T& v2, const T& d32,
   const T& v4, const T& d54,
   const T& v6, const T& d76,
   const S& u,
   const S& v,
   const S& w
)
{
   T v01 = v0 + d10*u;
   T v23 = v2 + d32*u;
   T v45 = v4 + d54*u;
   T v67 = v6 + d76*u;

   T v02 = v01 + ( v23-v01 )*v;
   T v46 = v45 + ( v67-v45 )*v;

   return v02 + ( v46-v02 )*w;
}

//------------------------------------------------------------------------------
//! Returns -2t^3 + 3t^2 = t^2*(-2t + 3), which is C1-continous around [0, 1] range.
template< typename T > inline T
smoothRampC1( const T& t )
{
   return t*t*(T(-2)*t + T(3));
}

//------------------------------------------------------------------------------
//! Returns 6t^5 - 15t^4 + 10t^3 = t^3*(t*(6t-15) + 10), which is C2-continous around [0, 1] range.
template< typename T > inline T
smoothRampC2( const T& t )
{
   return t*t*t*( t*(T(6)*t - T(15)) + T(10) );
}

//------------------------------------------------------------------------------
//! Performs a Hermite interpolation of a specified value inside the specified bounds.
template< typename T, typename S > inline S
smoothStep(
   const T& minVal,
   const T& maxVal,
   const S& val
)
{
   S d = (val - minVal)/(maxVal - minVal);
   d = clamp( d, T(0.0), T(1.0) );
   d = smoothRampC1( d );
   return d;
}

//------------------------------------------------------------------------------
//! Performs a Hermite interpolation of a specified value inside the specified bounds.
//! This variant uses a C2-continuous curve in the range [minVal, maxVal].
template< typename T, typename S > inline S
smoothStepC2(
   const T& minVal,
   const T& maxVal,
   const S& val
)
{
   S d = (val - minVal)/(maxVal - minVal);
   d = clamp( d, T(0.0), T(1.0) );
   d = smoothRampC2( d );
   return d;
}

//------------------------------------------------------------------------------
//! Evaluates a point on a quadratic bezier spline.
template< typename T, typename S > inline T
evalBezier(
   const T& p0,
   const T& p1,
   const T& p2, 
   const S& t
)
{
   const S t_m = (S)1 - t;
   return (
               p0 * (       t_m  * t_m)
             + p1 * ((S)2 * t    * t_m)
             + p2 * (       t    * t  )
          );
}

//------------------------------------------------------------------------------
//! Evaluates a point on a cubic bezier spline.
template< typename T, typename S > inline T
evalBezier(
   const T& p0,
   const T& p1,
   const T& p2, 
   const T& p3,
   const S& t
)
{
   const S t_m    = (S)1 - t;
   const S t_sq   = t * t;
   const S t_m_sq = t_m * t_m;
   return (
               p0 * (       t_m  * t_m_sq)
             + p1 * ((S)3 * t    * t_m_sq)
             + p2 * ((S)3 * t_sq * t_m   )
             + p3 * (       t_sq * t     )
          );
}

//------------------------------------------------------------------------------
//! Evaluates a point on a Catmull-Rom spline.
//! The curves passes through all 4 specified points, and is C1 (but not C2).
template< typename T, typename S > inline T
evalCatmullRom(
   const T& p0,
   const T& p1,
   const T& p2, 
   const T& p3,
   const S& t
)
{
   const S t_sq = t * t;
   const S t_cb = t_sq * t;
   // Note: sometimes, I swap term order to avoid negating a T.
   return (
               (p1*(S)2)
             + (p2 - p0) * t
             + (p0*(S)2 - p1*(S)5 + p2*(S)4 - p3) * t_sq
             + (p1*(S)3 - p0 - p2*(S)3 + p3) * t_cb
          ) * (S)0.5;
}

//------------------------------------------------------------------------------
//! Evaluates a point on a cubic Hermite spline.
//! The spline starts at p0 and ends at p1.
//! The values m0 and m1 are the starting and ending tangents.
template< typename T, typename S > inline T
evalHermite(
   const T& p0,
   const T& m0,
   const T& p1, 
   const T& m1,
   const S& t
)
{
   const S t_sq   = t * t;
   const S t_cb   = t * t_sq;
   return (         //     t^3         t^2   t^1   t^0
               p0 * ( (S)2*t_cb - (S)3*t_sq     + (S)1)
             + m0 * (      t_cb - (S)2*t_sq + t       )
             + p1 * ((S)-2*t_cb + (S)3*t_sq           )
             + m1 * (      t_cb -      t_sq           )
          );
}


} // namespace CGM

NAMESPACE_END

#endif //CGMATH_GEOM_H
