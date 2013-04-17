/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_VEC4_H
#define CGMATH_VEC4_H

#include <CGMath/StdDefs.h>

#include <CGMath/Vec3.h>

#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Vec4
==============================================================================*/

//! 4D vector class.

template< typename T >
class Vec4
{

public:

   /*----- methods -----*/

   inline static Vec4 zero();

   inline static       Vec4&  as(       T* ptr );
   inline static const Vec4&  as( const T* ptr );

   /*----- methods -----*/

   template< typename S > Vec4( const Vec4<S>& vec )
   {
      _e[0] = (T)vec.x;
      _e[1] = (T)vec.y;
      _e[2] = (T)vec.z;
      _e[3] = (T)vec.w;
   }

   template< typename S > explicit Vec4( const Vec2<S>& vec0, const Vec2<S>& vec1 )
   {
      _e[0] = (T)vec0.x;
      _e[1] = (T)vec0.y;
      _e[2] = (T)vec1.x;
      _e[3] = (T)vec1.y;
   }

   template< typename S > explicit Vec4( const Vec2<S>& vec, const T& z = 0, const T& w = 0 )
   {
      _e[0] = (T)vec.x;
      _e[1] = (T)vec.y;
      _e[2] = z;
      _e[3] = w;
   }

   template< typename S > explicit Vec4( const Vec3<S>& vec, const T& w = 0 )
   {
      _e[0] = (T)vec.x;
      _e[1] = (T)vec.y;
      _e[2] = (T)vec.z;
      _e[3] = w;
   }

   Vec4() {}
   Vec4( const Vec4<T>& vec );
   Vec4( const T& e0, const T& e1, const T& e2, const T& e3 );
   Vec4( const T& e );
   Vec4( const T* ptr );

   ~Vec4() {}

   T* ptr();
   const T* ptr() const;

   T length() const;
   T sqrLength() const;
   T dot( const Vec4<T>& vec ) const;

   Vec4 max( const Vec4<T>& vec ) const;
   Vec4 min( const Vec4<T>& vec ) const;

   T max() const;
   T min() const;

   Vec4& clampMax( const T& v );
   Vec4& clampMin( const T& v );
   Vec4& clamp( const T& min, const T& max );

   Vec4& maxLength( const T& v );
   Vec4& minLength( const T& v );

   Vec4  cross( const Vec4<T>& vec ) const;

   Vec4  projectOnto( const Vec4<T>& vec ) const;
   void  decompose( const Vec4<T>& vec, Vec4<T>& parallel, Vec4<T>& perpendicular ) const;

   Vec4& inverse();
   Vec4& negate();
   Vec4& normalize();
   Vec4& rescale( const T len );
   Vec4& premultiply();

   Vec4  getInversed() const;
   Vec4  getNormalized() const;
   Vec4  getRescaled( const T len ) const;
   Vec4  getPremultiplied() const;

   Vec4 operator+( const Vec4<T>& rhs ) const;
   Vec4 operator-( const Vec4<T>& rhs ) const;
   Vec4 operator-() const;
   Vec4 operator*( const T& rhs ) const;
   Vec4 operator*( const Vec4<T>& rhs ) const;
   Vec4 operator/( const T& rhs ) const;
   Vec4 operator/( const Vec4<T>& rhs ) const;

   Vec4& operator+=( const Vec4<T>& rhs );
   Vec4& operator-=( const Vec4<T>& rhs );
   Vec4& operator*=( const T& rhs );
   Vec4& operator*=( const Vec4<T>& rhs );
   Vec4& operator/=( const T& rhs );
   Vec4& operator/=( const Vec4<T>& rhs );
   Vec4& operator=( const Vec4<T>& rsh );

   bool operator==( const Vec4<T>& rhs ) const;
   bool operator!=( const Vec4<T>& rhs ) const;
   bool equal( const Vec4<T>& rhs, const T threshold = (T)CGM::EqualityThreshold ) const;

   T& operator()( int idx );
   const T& operator()( int idx ) const;
   Vec2<T> operator()( int xAxis, int yAxis ) const;
   Vec3<T> operator()( int xAxis, int yAxis, int zAxis ) const;
   Vec4<T> operator()( int xAxis, int yAxis, int zAxis, int wAxis ) const;

   /*----- data members -----*/

   union {
      struct {
         T x;
         T y;
         T z;
         T w;
      };
      T _e[4];
   };
};


//=============================================================================
// FUNCTIONS
//=============================================================================

namespace CGM {

//------------------------------------------------------------------------------
//!
template< typename T >
inline T dot( const Vec4<T>& a, const Vec4<T>& b )
{
   return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec4<T> cross( const Vec4<T>& a, const Vec4<T>& b )
{
   return Vec4<T>(
      a.y*b.z - a.z*b.y,
      a.z*b.x - a.x*b.z,
      a.x*b.y - a.y*b.x,
      0
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
normalize( const Vec4<T>& v )
{
   const T tmp = (T)1 / length(v);
   return Vec4<T>( v.x*tmp, v.y*tmp, v.z*tmp, v.w*tmp );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline T sqrLength( const Vec4<T>& v )
{
   return CGM::dot(v, v);
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline T length( const Vec4<T>& v )
{
   return CGM::sqrt( CGM::sqrLength(v) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline bool equal( const Vec4<T>& a, const Vec4<T>& b, const T threshold = (T)EqualityThreshold )
{
   return CGM::equal(a.x, b.x, threshold) &&
          CGM::equal(a.y, b.y, threshold) &&
          CGM::equal(a.z, b.z, threshold) &&
          CGM::equal(a.w, b.w, threshold);
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec4<T> max( const Vec4<T>& a, const Vec4<T>& b )
{
   return Vec4<T>( CGM::max(a.x, b.x), CGM::max(a.y, b.y), CGM::max(a.z, b.z), CGM::max(a.w, b.w) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec4<T> min( const Vec4<T>& a, const Vec4<T>& b )
{
   return Vec4<T>( CGM::min(a.x, b.x), CGM::min(a.y, b.y), CGM::min(a.z, b.z), CGM::min(a.w, b.w) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec4<T> abs( const Vec4<T>& vec )
{
   return Vec4<T>( CGM::abs(vec.x), CGM::abs(vec.y), CGM::abs(vec.z), CGM::abs(vec.w) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampMax( Vec4<T>& vec, const T& max )
{
   CGM::clampMax( vec.x, max );
   CGM::clampMax( vec.y, max );
   CGM::clampMax( vec.z, max );
   CGM::clampMax( vec.w, max );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampMin( Vec4<T>& vec, const T& min )
{
   CGM::clampMin( vec.x, min );
   CGM::clampMin( vec.y, min );
   CGM::clampMin( vec.z, min );
   CGM::clampMin( vec.w, min );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec4<T> clamp( const Vec4<T>& vec, const T& min, const T& max )
{
   return Vec4<T>(
      CGM::clamp( vec.x, min, max ),
      CGM::clamp( vec.y, min, max ),
      CGM::clamp( vec.z, min, max ),
      CGM::clamp( vec.w, min, max )
   );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampEq( Vec4<T>& vec, const T& min, const T& max )
{
   CGM::clampEq( vec.x, min, max );
   CGM::clampEq( vec.y, min, max );
   CGM::clampEq( vec.z, min, max );
   CGM::clampEq( vec.w, min, max );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampMax( Vec4<T>& vec, const Vec4<T>& max )
{
   CGM::clampMax( vec.x, max.x );
   CGM::clampMax( vec.y, max.y );
   CGM::clampMax( vec.z, max.z );
   CGM::clampMax( vec.w, max.w );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampMin( Vec4<T>& vec, const Vec4<T>& min )
{
   CGM::clampMin( vec.x, min.x );
   CGM::clampMin( vec.y, min.y );
   CGM::clampMin( vec.z, min.z );
   CGM::clampMin( vec.w, min.w );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec4<T> clamp( const Vec4<T>& vec, const Vec4<T>& min, const Vec4<T>& max )
{
   return Vec4<T>(
      CGM::clamp( vec.x, min.x, max.x ),
      CGM::clamp( vec.y, min.y, max.y ),
      CGM::clamp( vec.z, min.z, max.z ),
      CGM::clamp( vec.w, min.w, max.w )
   );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampEq( Vec4<T>& vec, const Vec4<T>& min, const Vec4<T>& max )
{
   CGM::clampEq( vec.x, min.x, max.x );
   CGM::clampEq( vec.y, min.y, max.y );
   CGM::clampEq( vec.z, min.z, max.z );
   CGM::clampEq( vec.w, min.w, max.w );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec4<T> floor( const Vec4<T>& vec )
{
   return Vec4<T>( CGM::floor(vec.x), CGM::floor(vec.y), CGM::floor(vec.z), CGM::floor(vec.w) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec4<T> ceil( const Vec4<T>& vec )
{
   return Vec4<T>( CGM::ceil(vec.x), CGM::ceil(vec.y), CGM::ceil(vec.z), CGM::ceil(vec.w) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec4<T> round( const Vec4<T>& vec )
{
   return Vec4<T>( CGM::round(vec.x), CGM::round(vec.y), CGM::round(vec.z), CGM::round(vec.w) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec4<T> fract( const Vec4<T>& vec )
{
   return Vec4<T>( CGM::fract(vec.x), CGM::fract(vec.y), CGM::fract(vec.z), CGM::fract(vec.w) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec4<T> fixedPrec( const Vec4<T>& vec, uint nFracBits = 24 )
{
   return Vec4<T>( CGM::fixedPrec(vec.x, nFracBits),
                   CGM::fixedPrec(vec.y, nFracBits),
                   CGM::fixedPrec(vec.z, nFracBits),
                   CGM::fixedPrec(vec.w, nFracBits) );
}

} // namespace CGM

using CGM::dot;
using CGM::cross;
using CGM::normalize;
using CGM::length;
using CGM::sqrLength;
using CGM::equal;

//=============================================================================
// METHODS
//=============================================================================

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::zero()
{
   return Vec4( 0, 0, 0, 0 );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::as( T* ptr )
{
   return reinterpret_cast<Vec4<T>&>( *ptr );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec4<T>&
Vec4<T>::as( const T* ptr )
{
   return reinterpret_cast<const Vec4<T>&>( *ptr );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Vec4<T>::Vec4( const Vec4<T>& vec )
{
   _e[0] = vec._e[0];
   _e[1] = vec._e[1];
   _e[2] = vec._e[2];
   _e[3] = vec._e[3];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Vec4<T>::Vec4( const T& e0, const T& e1, const T& e2, const T& e3 )
{
   _e[0] = e0;
   _e[1] = e1;
   _e[2] = e2;
   _e[3] = e3;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Vec4<T>::Vec4( const T& e )
{
   _e[0] = e;
   _e[1] = e;
   _e[2] = e;
   _e[3] = e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Vec4<T>::Vec4( const T* ptr )
{
   _e[0] = ptr[0];
   _e[1] = ptr[1];
   _e[2] = ptr[2];
   _e[3] = ptr[3];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T*
Vec4<T>::ptr()
{
   return _e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T*
Vec4<T>::ptr() const
{
   return _e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec4<T>::length() const
{
   return CGM::length( *this );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec4<T>::sqrLength() const
{
   return CGM::dot( *this, *this );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec4<T>::dot( const Vec4<T>& vec ) const
{
   return CGM::dot( *this, vec );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::max( const Vec4<T>& vec ) const
{
   return CGM::max( *this, vec );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::min( const Vec4<T>& vec ) const
{
   return CGM::min( *this, vec );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec4<T>::max() const
{
   return CGM::max( x, CGM::max( y, CGM::max( z, w ) ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec4<T>::min() const
{
   return CGM::min( x, CGM::min( y, CGM::min( z, w ) ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::clampMax( const T& max )
{
   CGM::clampMax( *this, max );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::clampMin( const T& min )
{
   CGM::clampMin( *this, min );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::clamp( const T& min, const T& max )
{
   _e[0] = CGM::clamp( _e[0], min, max );
   _e[1] = CGM::clamp( _e[1], min, max );
   _e[2] = CGM::clamp( _e[2], min, max );
   _e[3] = CGM::clamp( _e[3], min, max );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::maxLength( const T& maxLen )
{
   const T maxLenSqr = maxLen * maxLen;
   T lenSqr = sqrLength();
   if( lenSqr > maxLenSqr )
   {
      (*this) *= maxLen / CGM::sqrt( lenSqr );
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::minLength( const T& minLen )
{
   const T minLenSqr = minLen * minLen;
   T lenSqr = sqrLength();
   if( lenSqr < minLenSqr )
   {
      (*this) *= minLen / CGM::sqrt( lenSqr );
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::cross( const Vec4<T>& vec ) const
{
   return CGM::cross( *this, vec );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::projectOnto( const Vec4<T>& vec ) const
{
   return vec * (vec.dot(*this) / vec.sqrLength());
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Vec4<T>::decompose( const Vec4<T>& vec, Vec4<T>& parallel, Vec4<T>& perpendicular ) const
{
   parallel = projectOnto(vec);
   perpendicular = (*this) - parallel;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::inverse()
{
   _e[0] = T(1) / _e[0];
   _e[1] = T(1) / _e[1];
   _e[2] = T(1) / _e[2];
   _e[3] = T(1) / _e[3];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::negate()
{
   _e[0] = -_e[0];
   _e[1] = -_e[1];
   _e[2] = -_e[2];
   _e[3] = -_e[3];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::normalize()
{
   T tmp = (T)1 / length();
   _e[0] *= tmp;
   _e[1] *= tmp;
   _e[2] *= tmp;
   _e[3] *= tmp;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::rescale( const T len )
{
   T tmp = len / length();
   _e[0] *= tmp;
   _e[1] *= tmp;
   _e[2] *= tmp;
   _e[3] *= tmp;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::premultiply()
{
   _e[0] *= _e[3];
   _e[1] *= _e[3];
   _e[2] *= _e[3];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::getInversed() const
{
   return Vec3<T>( T(1) / _e[0], T(1) / _e[1], T(1) / _e[2], T(1) / _e[3] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::getNormalized() const
{
   return CGM::normalize( *this );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::getRescaled( const T len ) const
{
   T tmp = len / length();
   return (*this) * tmp;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::getPremultiplied() const
{
   return Vec4<T>( _e[0]*_e[3], _e[1]*_e[3], _e[2]*_e[3], _e[3] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::operator+( const Vec4<T>& rhs ) const
{
   return Vec4<T>(
      _e[0] + rhs._e[0],
      _e[1] + rhs._e[1],
      _e[2] + rhs._e[2],
      _e[3] + rhs._e[3]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::operator-( const Vec4<T>& rhs ) const
{
   return Vec4<T>(
      _e[0] - rhs._e[0],
      _e[1] - rhs._e[1],
      _e[2] - rhs._e[2],
      _e[3] - rhs._e[3]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::operator-() const
{
   return Vec4<T>( -_e[0], -_e[1], -_e[2], -_e[3] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::operator*( const T& rhs ) const
{
   return Vec4<T>(
      _e[0] * rhs,
      _e[1] * rhs,
      _e[2] * rhs,
      _e[3] * rhs
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::operator*( const Vec4<T>& rhs ) const
{
   return Vec4<T>(
      _e[0] * rhs._e[0],
      _e[1] * rhs._e[1],
      _e[2] * rhs._e[2],
      _e[3] * rhs._e[3]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::operator/( const T& rhs ) const
{
   return Vec4<T>(
      _e[0] / rhs,
      _e[1] / rhs,
      _e[2] / rhs,
      _e[3] / rhs
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::operator/( const Vec4<T>& rhs ) const
{
   return Vec4<T>(
      _e[0] / rhs._e[0],
      _e[1] / rhs._e[1],
      _e[2] / rhs._e[2],
      _e[3] / rhs._e[3]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::operator+=( const Vec4<T>& rhs )
{
   _e[0] += rhs._e[0];
   _e[1] += rhs._e[1];
   _e[2] += rhs._e[2];
   _e[3] += rhs._e[3];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::operator-=( const Vec4<T>& rhs )
{
   _e[0] -= rhs._e[0];
   _e[1] -= rhs._e[1];
   _e[2] -= rhs._e[2];
   _e[3] -= rhs._e[3];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::operator*=( const T& rhs )
{
   _e[0] *= rhs;
   _e[1] *= rhs;
   _e[2] *= rhs;
   _e[3] *= rhs;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::operator*=( const Vec4<T>& rhs )
{
   _e[0] *= rhs._e[0];
   _e[1] *= rhs._e[1];
   _e[2] *= rhs._e[2];
   _e[3] *= rhs._e[3];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::operator/=( const T& rhs )
{
   _e[0] /= rhs;
   _e[1] /= rhs;
   _e[2] /= rhs;
   _e[3] /= rhs;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::operator/=( const Vec4<T>& rhs )
{
   _e[0] /= rhs._e[0];
   _e[1] /= rhs._e[1];
   _e[2] /= rhs._e[2];
   _e[3] /= rhs._e[3];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>&
Vec4<T>::operator=( const Vec4<T>& rhs )
{
   _e[0] = rhs._e[0];
   _e[1] = rhs._e[1];
   _e[2] = rhs._e[2];
   _e[3] = rhs._e[3];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Vec4<T>::operator==( const Vec4<T>& rhs ) const
{
   return
      _e[0] == rhs._e[0] &&
      _e[1] == rhs._e[1] &&
      _e[2] == rhs._e[2] &&
      _e[3] == rhs._e[3];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Vec4<T>::operator!=( const Vec4<T>& rhs ) const
{
   return
      _e[0] != rhs._e[0] ||
      _e[1] != rhs._e[1] ||
      _e[2] != rhs._e[2] ||
      _e[3] != rhs._e[3];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Vec4<T>::equal( const Vec4<T>& vec, const T threshold ) const
{
   return CGM::equal( *this, vec, threshold );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Vec4<T>::operator()( int idx )
{
  return _e[idx];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T&
Vec4<T>::operator()( int idx ) const
{
  return _e[idx];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec4<T>::operator()( int xAxis, int yAxis ) const
{
  return Vec2<T>( _e[xAxis], _e[yAxis] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec4<T>::operator()( int xAxis, int yAxis, int zAxis ) const
{
  return Vec3<T>( _e[xAxis], _e[yAxis], _e[zAxis] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Vec4<T>::operator()( int xAxis, int yAxis, int zAxis, int wAxis ) const
{
  return Vec4<T>( _e[xAxis], _e[yAxis], _e[zAxis], _e[wAxis] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
operator*( const T& val, const Vec4<T>& vec )
{
   return Vec4<T>(
      vec(0) * val,
      vec(1) * val,
      vec(2) * val,
      vec(3) * val
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<( TextStream& stream, const Vec4<T>& vec )
{
   return stream << "("
                 << vec(0) << ","
                 << vec(1) << ","
                 << vec(2) << ","
                 << vec(3) << ")";
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator>>( TextStream& stream, Vec4<T>& vec )
{
   char c;
   return stream >> c
                 >> vec(0) >> c
                 >> vec(1) >> c
                 >> vec(2) >> c
                 >> vec(3) >> c;
}

/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef Vec4< uchar >  Vec4uc;
typedef Vec4< int >    Vec4i;
typedef Vec4< float >  Vec4f;
typedef Vec4< double > Vec4d;

NAMESPACE_END

#endif
