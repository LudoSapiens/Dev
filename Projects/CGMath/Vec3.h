/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_VEC3_H
#define CGMATH_VEC3_H

#include <CGMath/StdDefs.h>

#include <CGMath/Vec2.h>

#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Vec3
==============================================================================*/

//! 3D vector class.

template< typename T >
class Vec3
{

public:


   /*----- static methods -----*/

   inline static Vec3 zero();
   inline static Vec3 perpendicular( const Vec3<T>& dir );

   inline static       Vec3&  as(       T* ptr );
   inline static const Vec3&  as( const T* ptr );

   /*----- methods -----*/

   template< typename S > Vec3( const Vec3<S>& vec )
   {
      _e[0] = (T)vec.x;
      _e[1] = (T)vec.y;
      _e[2] = (T)vec.z;
   }

   template< typename S > explicit Vec3( const Vec2<S>& vec, const T& z = 0 )
   {
      _e[0] = (T)vec.x;
      _e[1] = (T)vec.y;
      _e[2] = z;
   }

   Vec3(){}
   Vec3( const Vec3<T>& vec );
   Vec3( const T& e0, const T& e1, const T& e2 );
   Vec3( const T& e );
   Vec3( const T* ptr );

   ~Vec3(){}

   T* ptr();
   const T* ptr() const;

   T length() const;
   T sqrLength() const;
   T dot( const Vec3<T>& vec ) const;

   Vec3 max( const Vec3<T>& vec ) const;
   Vec3 min( const Vec3<T>& vec ) const;

   T max() const;
   T min() const;

   uint maxComponent() const;
   uint minComponent() const;
   void secAxes( int& xAxis, int& yAxis ) const;
   void secAxes( int& xAxis, int& yAxis, int& zAxis ) const;

   Vec3& clampMax( const T& v );
   Vec3& clampMin( const T& v );
   Vec3& clamp( const T& min, const T& max );

   Vec3& maxLength( const T& v );
   Vec3& minLength( const T& v );

   Vec3  cross( const Vec3<T>& vec ) const;

   Vec3  projectOnto( const Vec3<T>& vec ) const;
   void  decompose( const Vec3<T>& vec, Vec3<T>& parallel, Vec3<T>& perpendicular ) const;

   Vec3& inverse();
   Vec3& negate();
   Vec3& normalize();
   Vec3& normalizeSafe();
   Vec3& rescale( const T len );

   Vec3  getNormalized() const;
   Vec3  getRescaled( const T len ) const;
   Vec3  getInversed() const;
   Vec3  getAxisAlignedDirection() const;

   void  toThetaPhi( T& theta, T& phi ) const;
   void  fromThetaPhi( const T theta, const T phi, const T radius = (T)1.0 );

   Vec3 operator+( const Vec3<T>& rhs ) const;
   Vec3 operator-( const Vec3<T>& rhs ) const;
   Vec3 operator-() const;
   Vec3 operator*( const T& rhs ) const;
   Vec3 operator*( const Vec3<T>& rhs ) const;
   Vec3 operator/( const T& rhs ) const;
   Vec3 operator/( const Vec3<T>& rhs ) const;

   Vec3& operator+=( const Vec3<T>& rhs );
   Vec3& operator-=( const Vec3<T>& rhs );
   Vec3& operator*=( const T& rhs );
   Vec3& operator*=( const Vec3<T>& rhs );
   Vec3& operator/=( const T& rhs );
   Vec3& operator/=( const Vec3<T>& rhs );
   Vec3& operator=( const Vec3<T>& rsh );

   bool operator==( const Vec3<T>& rhs ) const;
   bool operator!=( const Vec3<T>& rhs ) const;
   bool equal( const Vec3<T>& rhs, const T threshold = (T)CGM::EqualityThreshold ) const;

   T& operator()( int idx );
   const T& operator()( int idx ) const;
   Vec2<T> operator()( int xAxis, int yAxis ) const;
   Vec3<T> operator()( int xAxis, int yAxis, int zAxis ) const;

   /*----- data members -----*/

   union {
      struct {
         T x;
         T y;
         T z;
      };
      T _e[3];
   };
};

//=============================================================================
// FUNCTIONS
//=============================================================================

namespace CGM {

//------------------------------------------------------------------------------
//!
template< typename T >
inline T dot( const Vec3<T>& a, const Vec3<T>& b )
{
   return a.x*b.x + a.y*b.y + a.z*b.z;
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec3<T> cross( const Vec3<T>& a, const Vec3<T>& b )
{
   return Vec3<T>(
      a.y*b.z - a.z*b.y,
      a.z*b.x - a.x*b.z,
      a.x*b.y - a.y*b.x
   );
}

//------------------------------------------------------------------------------
//! 
template< typename T > inline Vec3<T>
normalize( const Vec3<T>& v )
{
   const T tmp = (T)1 / length(v);
   return Vec3<T>( v.x*tmp, v.y*tmp, v.z*tmp );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline T sqrLength( const Vec3<T>& v )
{
   return CGM::dot(v, v);
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline T length( const Vec3<T>& v )
{
   return CGM::sqrt( CGM::sqrLength(v) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline bool equal( const Vec3<T>& a, const Vec3<T>& b, const T threshold = (T)EqualityThreshold )
{
   return CGM::equal(a.x, b.x, threshold) &&
          CGM::equal(a.y, b.y, threshold) &&
          CGM::equal(a.z, b.z, threshold);
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec3<T> max( const Vec3<T>& a, const Vec3<T>& b )
{
   return Vec3<T>( CGM::max(a.x, b.x), CGM::max(a.y, b.y), CGM::max(a.z, b.z) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec3<T> min( const Vec3<T>& a, const Vec3<T>& b )
{
   return Vec3<T>( CGM::min(a.x, b.x), CGM::min(a.y, b.y), CGM::min(a.z, b.z) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec3<T> abs( const Vec3<T>& vec )
{
   return Vec3<T>( CGM::abs(vec.x), CGM::abs(vec.y), CGM::abs(vec.z) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampMax( Vec3<T>& vec, const T& max )
{
   CGM::clampMax( vec.x, max );
   CGM::clampMax( vec.y, max );
   CGM::clampMax( vec.z, max );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampMin( Vec3<T>& vec, const T& min )
{
   CGM::clampMin( vec.x, min );
   CGM::clampMin( vec.y, min );
   CGM::clampMin( vec.z, min );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec3<T> clamp( const Vec3<T>& vec, const T& min, const T& max )
{
   return Vec3<T>(
      CGM::clamp( vec.x, min, max ),
      CGM::clamp( vec.y, min, max ),
      CGM::clamp( vec.z, min, max )
   );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampEq( Vec3<T>& vec, const T& min, const T& max )
{
   CGM::clampEq( vec.x, min, max );
   CGM::clampEq( vec.y, min, max );
   CGM::clampEq( vec.z, min, max );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampMax( Vec3<T>& vec, const Vec3<T>& max )
{
   CGM::clampMax( vec.x, max.x );
   CGM::clampMax( vec.y, max.y );
   CGM::clampMax( vec.z, max.z );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampMin( Vec3<T>& vec, const Vec3<T>& min )
{
   CGM::clampMin( vec.x, min.x );
   CGM::clampMin( vec.y, min.y );
   CGM::clampMin( vec.z, min.z );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec3<T> clamp( const Vec3<T>& vec, const Vec3<T>& min, const Vec3<T>& max )
{
   return Vec3<T>(
      CGM::clamp( vec.x, min.x, max.x ),
      CGM::clamp( vec.y, min.y, max.y ),
      CGM::clamp( vec.z, min.z, max.z )
   );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampEq( Vec3<T>& vec, const Vec3<T>& min, const Vec3<T>& max )
{
   CGM::clampEq( vec.x, min.x, max.x );
   CGM::clampEq( vec.y, min.y, max.y );
   CGM::clampEq( vec.z, min.z, max.z );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec3<T> floor( const Vec3<T>& vec )
{
   return Vec3<T>( CGM::floor(vec.x), CGM::floor(vec.y), CGM::floor(vec.z) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec3<T> ceil( const Vec3<T>& vec )
{
   return Vec3<T>( CGM::ceil(vec.x), CGM::ceil(vec.y), CGM::ceil(vec.z) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec3<T> round( const Vec3<T>& vec )
{
   return Vec3<T>( CGM::round(vec.x), CGM::round(vec.y), CGM::round(vec.z) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec3<T> fract( const Vec3<T>& vec )
{
   return Vec3<T>( CGM::fract(vec.x), CGM::fract(vec.y), CGM::fract(vec.z) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec3<T> fixedPrec( const Vec3<T>& vec, uint nFracBits = 24 )
{
   return Vec3<T>( CGM::fixedPrec(vec.x, nFracBits),
                   CGM::fixedPrec(vec.y, nFracBits),
                   CGM::fixedPrec(vec.z, nFracBits) );
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
template< typename T > inline Vec3<T>
Vec3<T>::zero()
{
   return Vec3( T(0), T(0), T(0) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::perpendicular( const Vec3<T>& dir )
{
   Vec3<T> tmp( zero() );

   // Find the dimension with the smallest length and use the corresponding
   // orthonormal axis (either (1, 0, 0), (0, 1, 0), or (0, 0, 1)).
   if( CGM::abs(dir.x) < CGM::abs(dir.y) )
   {
      if( CGM::abs(dir.x) < CGM::abs(dir.z) )
      {
         //X is smallest, so use +X
         tmp.x = T(1);
      }
      else
      {
         //Z is smallest, so use +Z
         tmp.z = T(1);
      }
   }
   else
   {
      if( CGM::abs(dir.y) < CGM::abs(dir.z) )
      {
         //Y is smallest, so use +Y
         tmp.y = T(1);
      }
      else
      {
         //Z is smallest, so use +Z
         tmp.z = T(1);
      }
   }
   // Generate a perpendicular vector
   return dir.cross(tmp);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::as( T* ptr )
{
   return reinterpret_cast<Vec3<T>&>( *ptr );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec3<T>&
Vec3<T>::as( const T* ptr )
{
   return reinterpret_cast<const Vec3<T>&>( *ptr );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Vec3<T>::Vec3( const Vec3<T>& vec )
{
   _e[0] = vec._e[0];
   _e[1] = vec._e[1];
   _e[2] = vec._e[2];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Vec3<T>::Vec3( const T& e0, const T& e1, const T& e2 )
{
   _e[0] = e0;
   _e[1] = e1;
   _e[2] = e2;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Vec3<T>::Vec3( const T& e )
{
   _e[0] = e;
   _e[1] = e;
   _e[2] = e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Vec3<T>::Vec3( const T* ptr )
{
   _e[0] = ptr[0];
   _e[1] = ptr[1];
   _e[2] = ptr[2];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T*
Vec3<T>::ptr()
{
   return _e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T*
Vec3<T>::ptr() const
{
   return _e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec3<T>::length() const
{
   return CGM::length( *this );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec3<T>::sqrLength() const
{
   return CGM::dot( *this, *this );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec3<T>::dot( const Vec3<T>& vec ) const
{
   return CGM::dot( *this, vec );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::max( const Vec3<T>& vec ) const
{
   return CGM::max( *this, vec );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::min( const Vec3<T>& vec ) const
{
   return CGM::min( *this, vec );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec3<T>::max() const
{
   return CGM::max( x, CGM::max( y, z ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec3<T>::min() const
{
   return CGM::min( x, CGM::min( y, z ) );
}

//------------------------------------------------------------------------------
//! 
template< typename T > inline uint
Vec3<T>::maxComponent() const
{
   if( CGM::abs(x) > CGM::abs(y) )
   {
      return CGM::abs(x) > CGM::abs(z) ? 0 : 2;
   }
   else
   {
      return CGM::abs(y) > CGM::abs(z) ? 1 : 2;
   }
}

//------------------------------------------------------------------------------
//! 
template< typename T > inline uint
Vec3<T>::minComponent() const
{
   if( CGM::abs(x) < CGM::abs(y) )
   {
      return CGM::abs(x) < CGM::abs(z) ? 0 : 2;
   }
   else
   {
      return CGM::abs(y) < CGM::abs(z) ? 1 : 2;
   }
}

//------------------------------------------------------------------------------
//! 
template< typename T > inline void 
Vec3<T>::secAxes( int& ax, int& ay ) const
{
   int az;
   secAxes( ax, ay, az );
}

//------------------------------------------------------------------------------
//! 
template< typename T > inline void 
Vec3<T>::secAxes( int& ax, int& ay, int& az ) const
{
   if( CGM::abs(x) > CGM::abs(y) )
   {
      if( CGM::abs(x) > CGM::abs(z) )
      {
         ax = 1;
         ay = 2;
         az = 0;
      }
      else
      {
         ax = 0;
         ay = 1;
         az = 2;
      }
   }
   else
   {
      if( CGM::abs(y) > CGM::abs(z) )
      {
         ax = 2;
         ay = 0;
         az = 1;
      }
      else
      {
         ax = 0;
         ay = 1;
         az = 2;
      }
   }
   if( _e[az] < 0.0f ) CGM::swap( ax, ay );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::clampMax( const T& max )
{
   CGM::clampMax( *this, max );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::clampMin( const T& min )
{
   CGM::clampMin( *this, min );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::clamp( const T& min, const T& max )
{
   _e[0] = CGM::clamp( _e[0], min, max );
   _e[1] = CGM::clamp( _e[1], min, max );
   _e[2] = CGM::clamp( _e[2], min, max );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::maxLength( const T& maxLen )
{
   const T maxLenSqr = maxLen * maxLen;
   T lenSqr = sqrLength();
   if( lenSqr > maxLenSqr ) (*this) *= maxLen / CGM::sqrt( lenSqr );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::minLength( const T& minLen )
{
   const T minLenSqr = minLen * minLen;
   T lenSqr = sqrLength();
   if( lenSqr < minLenSqr ) (*this) *= minLen / CGM::sqrt( lenSqr );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::cross( const Vec3<T>& vec ) const
{
   return Vec3<T>(
      _e[1]*vec._e[2] - _e[2]*vec._e[1],
      _e[2]*vec._e[0] - _e[0]*vec._e[2],
      _e[0]*vec._e[1] - _e[1]*vec._e[0]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::projectOnto( const Vec3<T>& vec ) const
{
   return vec * (vec.dot(*this) / vec.sqrLength());
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Vec3<T>::decompose
( const Vec3<T>& vec, Vec3<T>& parallel, Vec3<T>& perpendicular ) const
{
   parallel = projectOnto(vec);
   perpendicular = (*this) - parallel;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::inverse()
{
   _e[0] = T(1) / _e[0];
   _e[1] = T(1) / _e[1];
   _e[2] = T(1) / _e[2];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::negate()
{
   _e[0] = -_e[0];
   _e[1] = -_e[1];
   _e[2] = -_e[2];
   return *this;
}

//------------------------------------------------------------------------------
//! Normalizes the Vec3 in place, making it unit length.
template< typename T > inline Vec3<T>&
Vec3<T>::normalize()
{
   const T tmp = CGM::invSqrt( sqrLength() );
   _e[0] *= tmp;
   _e[1] *= tmp;
   _e[2] *= tmp;
   return *this;
}

//------------------------------------------------------------------------------
//! Normalizes the Vec3 in place, making it unit length.
template< typename T > inline Vec3<T>&
Vec3<T>::normalizeSafe()
{
   const T sqLen = sqrLength();
   if( sqLen != T(0) )
   {
      const T tmp = CGM::invSqrt( sqLen );
      _e[0] *= tmp;
      _e[1] *= tmp;
      _e[2] *= tmp;
   }
   return *this;
}

//------------------------------------------------------------------------------
//! Normalizes the Vec3 in place, making it the specified length.
template< typename T > inline Vec3<T>&
Vec3<T>::rescale( const T len )
{
   const T tmp = len / length();
   _e[0] *= tmp;
   _e[1] *= tmp;
   _e[2] *= tmp;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::getInversed() const
{
   return Vec3<T>( T(1) / _e[0], T(1) / _e[1], T(1) / _e[2] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::getNormalized() const
{
   return CGM::normalize( *this );
}

//------------------------------------------------------------------------------
//! Returns the vector rescaled to be of the specified length.
template< typename T > inline Vec3<T>
Vec3<T>::getRescaled( const T len ) const
{
   const T tmp = len / length();
   return (*this) * tmp;
}

//------------------------------------------------------------------------------
//! Returns one of the 6 axis-aligned vectors, depending on which gives a larger
//! projection.
template< typename T > inline Vec3<T>
Vec3<T>::getAxisAlignedDirection() const
{
   uint m = maxComponent();
   Vec3<T> tmp = Vec3<T>( (T)0 );
   tmp(m) = CGM::copySign( (T)1, _e[m] );
   return tmp;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Vec3<T>::toThetaPhi( T& theta, T& phi ) const
{
   T r = length();
   theta = CGM::atan2(x, z);
   phi   = CGM::acos(y/r);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Vec3<T>::fromThetaPhi( const T theta, const T phi, const T radius )
{
   T rSinPhi = radius * CGM::sin(phi);
   x = rSinPhi * CGM::sin(theta);
   y = radius  * CGM::cos(phi);
   z = rSinPhi * CGM::cos(theta);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::operator+( const Vec3<T>& rhs ) const
{
   return Vec3<T>(
      _e[0] + rhs._e[0],
      _e[1] + rhs._e[1],
      _e[2] + rhs._e[2]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::operator-( const Vec3<T>& rhs ) const
{
   return Vec3<T>(
      _e[0] - rhs._e[0],
      _e[1] - rhs._e[1],
      _e[2] - rhs._e[2]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::operator-() const
{
   return Vec3<T>( -_e[0], -_e[1], -_e[2] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::operator*( const T& rhs ) const
{
   return Vec3<T>( _e[0] * rhs, _e[1] * rhs, _e[2] * rhs );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::operator*( const Vec3<T>& rhs ) const
{
   return Vec3<T>(
      _e[0] * rhs._e[0],
      _e[1] * rhs._e[1],
      _e[2] * rhs._e[2]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::operator/( const T& rhs ) const
{
   return Vec3<T>( _e[0] / rhs, _e[1] / rhs, _e[2] / rhs );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::operator/( const Vec3<T>& rhs ) const
{
   return Vec3<T>( _e[0] / rhs._e[0], _e[1] / rhs._e[1], _e[2] / rhs._e[2] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::operator+=( const Vec3<T>& rhs )
{
   _e[0] += rhs._e[0];
   _e[1] += rhs._e[1];
   _e[2] += rhs._e[2];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::operator-=( const Vec3<T>& rhs )
{
   _e[0] -= rhs._e[0];
   _e[1] -= rhs._e[1];
   _e[2] -= rhs._e[2];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::operator*=( const T& rhs )
{
   _e[0] *= rhs;
   _e[1] *= rhs;
   _e[2] *= rhs;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::operator*=( const Vec3<T>& rhs )
{
   _e[0] *= rhs._e[0];
   _e[1] *= rhs._e[1];
   _e[2] *= rhs._e[2];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::operator/=( const T& rhs )
{
   _e[0] /= rhs;
   _e[1] /= rhs;
   _e[2] /= rhs;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::operator/=( const Vec3<T>& rhs )
{
   _e[0] /= rhs._e[0];
   _e[1] /= rhs._e[1];
   _e[2] /= rhs._e[2];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Vec3<T>::operator=( const Vec3<T>& rhs )
{
   _e[0] = rhs._e[0];
   _e[1] = rhs._e[1];
   _e[2] = rhs._e[2];
   return *this;
}


//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Vec3<T>::operator==( const Vec3<T>& rhs ) const
{
   return
      _e[0] == rhs._e[0] &&
      _e[1] == rhs._e[1] &&
      _e[2] == rhs._e[2];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Vec3<T>::operator!=( const Vec3<T>& rhs ) const
{
   return
      _e[0] != rhs._e[0] ||
      _e[1] != rhs._e[1] ||
      _e[2] != rhs._e[2];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Vec3<T>::equal( const Vec3<T>& vec, const T threshold ) const
{
   return CGM::equal( *this, vec, threshold );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Vec3<T>::operator()( int idx )
{
  return _e[idx];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T&
Vec3<T>::operator()( int idx ) const
{
  return _e[idx];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec3<T>::operator()( int xAxis, int yAxis ) const
{
  return Vec2<T>( _e[xAxis], _e[yAxis] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Vec3<T>::operator()( int xAxis, int yAxis, int zAxis ) const
{
  return Vec3<T>( _e[xAxis], _e[yAxis], _e[zAxis] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
operator*( const T& val, const Vec3<T>& vec )
{
   return Vec3<T>( vec(0) * val, vec(1) * val, vec(2) * val );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<( TextStream& stream, const Vec3<T>& vec )
{
   return stream << "(" << vec(0) << "," << vec(1) << "," << vec(2) << ")";
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator>>( TextStream& stream, Vec3<T>& vec )
{
   char c;
   return stream >> c >> vec(0) >> c >> vec(1) >> c >> vec(2) >> c;
}

/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef Vec3< int >    Vec3i;
typedef Vec3< float >  Vec3f;
typedef Vec3< double > Vec3d;

NAMESPACE_END

#endif
