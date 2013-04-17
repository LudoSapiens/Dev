/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_VEC2_H
#define CGMATH_VEC2_H

#include <CGMath/StdDefs.h>

#include <CGMath/Math.h>
#include <CGMath/Trig.h>

#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Vec2
==============================================================================*/

//! 2D vector class.

template< typename T >
class Vec2
{

public:

   /*----- static methods -----*/

   inline static Vec2 zero();

   inline static       Vec2&  as(       T* ptr );
   inline static const Vec2&  as( const T* ptr );

   /*----- methods -----*/

   template< typename S > Vec2( const Vec2<S>& vec )
   {
      _e[0] = (T)vec(0);
      _e[1] = (T)vec(1);
   }

   Vec2() {}
   Vec2( const Vec2<T>& vec );
   Vec2( const T& e0, const T& e1 );
   Vec2( const T& e );
   Vec2( const T* ptr );

   ~Vec2() {}

   T* ptr();
   const T* ptr() const;

   T length() const;
   T sqrLength() const;
   T dot( const Vec2<T>& vec ) const;
   T pseudoCross( const Vec2<T>& vec ) const;

   Vec2 max( const Vec2<T>& vec ) const;
   Vec2 min( const Vec2<T>& vec ) const;

   T max() const;
   T min() const;

   uint maxComponent() const;
   uint minComponent() const;

   Vec2& clampMax( const T& v );
   Vec2& clampMin( const T& v );
   Vec2& clamp( const T& min, const T& max );

   Vec2& maxLength( const T& v );
   Vec2& minLength( const T& v );

   Vec2  projectOnto( const Vec2<T>& vec ) const;
   void  decompose( const Vec2<T>& vec, Vec2<T>& parallel, Vec2<T>& perpendicular ) const;

   Vec2& inverse();
   Vec2& negate();
   Vec2& normalize();
   Vec2& rescale( const T len );

   Vec2  getInversed() const;
   Vec2  getNormalized() const;
   Vec2  getRescaled( const T len ) const;
   Vec2  getAxisAlignedDirection() const;

   Vec2 operator+( const Vec2<T>& rhs ) const;
   Vec2 operator-( const Vec2<T>& rhs ) const;
   Vec2 operator-() const;
   Vec2 operator*( const T& rhs ) const;
   Vec2 operator*( const Vec2<T>& rhs ) const;
   Vec2 operator/( const T& rhs ) const;
   Vec2 operator/( const Vec2<T>& rhs ) const;

   Vec2& operator+=( const Vec2<T>& rhs );
   Vec2& operator-=( const Vec2<T>& rhs );
   Vec2& operator*=( const T& rhs );
   Vec2& operator*=( const Vec2<T>& rhs );
   Vec2& operator/=( const T& rhs );
   Vec2& operator/=( const Vec2<T>& rhs );
   Vec2& operator=( const Vec2<T>& rsh );

   bool operator==( const Vec2<T>& rhs ) const;
   bool operator!=( const Vec2<T>& rhs ) const;
   bool equal( const Vec2<T>& rhs, const T threshold = (T)CGM::EqualityThreshold ) const;

   T& operator()( int idx );
   const T& operator()( int idx ) const;
   Vec2<T> operator()( int xAxis, int yAxis ) const;

   /*----- data members -----*/

   union {
      struct {
         T x;
         T y;
      };
      T _e[2];
   };
};

//=============================================================================
// FUNCTIONS
//=============================================================================

namespace CGM {

//------------------------------------------------------------------------------
//!
template< typename T >
inline T dot( const Vec2<T>& a, const Vec2<T>& b )
{
   return a.x*b.x + a.y*b.y;
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline T pseudoCross( const Vec2<T>& a, const Vec2<T>& b )
{
   return a.x*b.y - a.y*b.x;
}

//------------------------------------------------------------------------------
//! 
template< typename T > inline Vec2<T>
normalize( const Vec2<T>& v )
{
   const T tmp = (T)1 / length(v);
   return Vec2<T>( v.x*tmp, v.y*tmp );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline T sqrLength( const Vec2<T>& v )
{
   return CGM::dot(v, v);
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline T length( const Vec2<T>& v )
{
   return CGM::sqrt( CGM::sqrLength(v) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline bool equal( const Vec2<T>& a, const Vec2<T>& b, const T threshold = (T)EqualityThreshold )
{
   return CGM::equal(a.x, b.x, threshold) &&
          CGM::equal(a.y, b.y, threshold);
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec2<T> max( const Vec2<T>& a, const Vec2<T>& b )
{
   return Vec2<T>( CGM::max(a.x, b.x), CGM::max(a.y, b.y) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec2<T> min( const Vec2<T>& a, const Vec2<T>& b )
{
   return Vec2<T>( CGM::min(a.x, b.x), CGM::min(a.y, b.y) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec2<T> abs( const Vec2<T>& vec )
{
   return Vec2<T>( CGM::abs(vec.x), CGM::abs(vec.y) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampMax( Vec2<T>& vec, const T& max )
{
   CGM::clampMax( vec.x, max );
   CGM::clampMax( vec.y, max );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampMin( Vec2<T>& vec, const T& min )
{
   CGM::clampMin( vec.x, min );
   CGM::clampMin( vec.y, min );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec2<T> clamp( const Vec2<T>& vec, const T& min, const T& max )
{
   return Vec2<T>(
      CGM::clamp( vec.x, min, max ),
      CGM::clamp( vec.y, min, max )
   );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampEq( Vec2<T>& vec, const T& min, const T& max )
{
   CGM::clampEq( vec.x, min, max );
   CGM::clampEq( vec.y, min, max );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampMax( Vec2<T>& vec, const Vec2<T>& max )
{
   CGM::clampMax( vec.x, max.x );
   CGM::clampMax( vec.y, max.y );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampMin( Vec2<T>& vec, const Vec2<T>& min )
{
   CGM::clampMin( vec.x, min.x );
   CGM::clampMin( vec.y, min.y );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec2<T> clamp( const Vec2<T>& vec, const Vec2<T>& min, const Vec2<T>& max )
{
   return Vec2<T>(
      CGM::clamp( vec.x, min.x, max.x ),
      CGM::clamp( vec.y, min.y, max.y )
   );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void clampEq( Vec2<T>& vec, const Vec2<T>& min, const Vec2<T>& max )
{
   CGM::clampEq( vec.x, min.x, max.x );
   CGM::clampEq( vec.y, min.y, max.y );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec2<T> floor( const Vec2<T>& vec )
{
   return Vec2<T>( CGM::floor(vec.x), CGM::floor(vec.y) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec2<T> ceil( const Vec2<T>& vec )
{
   return Vec2<T>( CGM::ceil(vec.x), CGM::ceil(vec.y) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec2<T> round( const Vec2<T>& vec )
{
   return Vec2<T>( CGM::round(vec.x), CGM::round(vec.y) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec2<T> fract( const Vec2<T>& vec )
{
   return Vec2<T>( CGM::fract(vec.x), CGM::fract(vec.y) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec2<T> fixedPrec( const Vec2<T>& vec, uint nFracBits = 24 )
{
   return Vec2<T>( CGM::fixedPrec(vec.x, nFracBits),
                   CGM::fixedPrec(vec.y, nFracBits) );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline Vec2<T> nextPow2( const Vec2<T>& vec )
{
   return Vec2<T>( CGM::nextPow2(vec.x),
                   CGM::nextPow2(vec.y) );
}

} // namespace CGM

using CGM::dot;
using CGM::pseudoCross;
using CGM::normalize;
using CGM::length;
using CGM::sqrLength;
using CGM::equal;

//=============================================================================
// METHODS
//=============================================================================

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::zero()
{
   return Vec2( 0, 0 );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::as( T* ptr )
{
   return reinterpret_cast<Vec2<T>&>( *ptr );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec2<T>&
Vec2<T>::as( const T* ptr )
{
   return reinterpret_cast<const Vec2<T>&>( *ptr );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Vec2<T>::Vec2( const Vec2<T>& vec )
{
   _e[0] = vec._e[0];
   _e[1] = vec._e[1];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Vec2<T>::Vec2( const T& e0, const T& e1 )
{
   _e[0] = e0;
   _e[1] = e1;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Vec2<T>::Vec2( const T& e )
{
   _e[0] = e;
   _e[1] = e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Vec2<T>::Vec2( const T* ptr )
{
   _e[0] = ptr[0];
   _e[1] = ptr[1];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T*
Vec2<T>::ptr()
{
   return _e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T*
Vec2<T>::ptr() const
{
   return _e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec2<T>::length() const
{
   return CGM::length( *this );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec2<T>::sqrLength() const
{
   return CGM::dot( *this, *this );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec2<T>::dot( const Vec2<T>& vec ) const
{
   return CGM::dot( *this, vec );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec2<T>::pseudoCross( const Vec2<T>& vec ) const
{
   return CGM::pseudoCross( *this, vec );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::max( const Vec2<T>& vec ) const
{
   return CGM::max( *this, vec );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::min( const Vec2<T>& vec ) const
{
   return CGM::min( *this, vec );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec2<T>::max() const
{
   return CGM::max( x, y );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Vec2<T>::min() const
{
   return CGM::min( x, y );
}

//------------------------------------------------------------------------------
//! 
template< typename T > inline uint
Vec2<T>::maxComponent() const
{
   return CGM::abs(x) > CGM::abs(y) ? 0 : 1;
}

//------------------------------------------------------------------------------
//! 
template< typename T > inline uint
Vec2<T>::minComponent() const
{
   return CGM::abs(x) < CGM::abs(y) ? 0 : 1;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::clampMax( const T& v )
{
   CGM::clampMax( *this, v );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::clampMin( const T& v )
{
   CGM::clampMin( *this, v );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::clamp( const T& min, const T& max )
{
   _e[0] = CGM::clamp( _e[0], min, max );
   _e[1] = CGM::clamp( _e[1], min, max );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::maxLength( const T& maxLen )
{
   const T maxLenSqr = maxLen * maxLen;
   T lenSqr = sqrLength();
   if( lenSqr > maxLenSqr ) (*this) *= maxLen / CGM::sqrt( lenSqr );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::minLength( const T& minLen )
{
   const T minLenSqr = minLen * minLen;
   T lenSqr = sqrLength();
   if( lenSqr < minLenSqr ) (*this) *= minLen / CGM::sqrt( lenSqr );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::projectOnto( const Vec2<T>& vec ) const
{
   return vec * (vec.dot(*this) / vec.sqrLength());
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Vec2<T>::decompose( const Vec2<T>& vec, Vec2<T>& parallel, Vec2<T>& perpendicular ) const
{
   parallel = projectOnto(vec);
   perpendicular = (*this) - parallel;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::inverse()
{
   _e[0] = T(1) / _e[0];
   _e[1] = T(1) / _e[1];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::negate()
{
   _e[0] = -_e[0];
   _e[1] = -_e[1];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::normalize()
{
   T tmp = (T)1 / length();
   _e[0] *= tmp;
   _e[1] *= tmp;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::rescale( const T len )
{
   T tmp = len / length();
   _e[0] *= tmp;
   _e[1] *= tmp;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::getInversed() const
{
   return Vec2<T>( T(1) / _e[0], T(1) / _e[1] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::getNormalized() const
{
   return CGM::normalize( *this );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::getRescaled( const T len ) const
{
   T tmp = len / length();
   return (*this) * tmp;
}

//------------------------------------------------------------------------------
//! Returns one of the 4 axis-aligned vectors, depending on which gives a larger
//! projection.
template< typename T > inline Vec2<T>
Vec2<T>::getAxisAlignedDirection() const
{
   uint m = maxComponent();
   Vec2<T> tmp = Vec2<T>( (T)0 );
   tmp(m) = CGM::copySign( (T)1, _e[m] );
   return tmp;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::operator+( const Vec2<T>& rhs ) const
{
   return Vec2<T>(
      _e[0] + rhs._e[0],
      _e[1] + rhs._e[1]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::operator-( const Vec2<T>& rhs ) const
{
   return Vec2<T>(
      _e[0] - rhs._e[0],
      _e[1] - rhs._e[1]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::operator-() const
{
   return Vec2<T>( -_e[0], -_e[1] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::operator*( const T& rhs ) const
{
   return Vec2<T>( _e[0] * rhs, _e[1] * rhs );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::operator*( const Vec2<T>& rhs ) const
{
   return Vec2<T>( _e[0] * rhs._e[0], _e[1] * rhs._e[1] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::operator/( const T& rhs ) const
{
   return Vec2<T>( _e[0] / rhs, _e[1] / rhs );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::operator/( const Vec2<T>& rhs ) const
{
   return Vec2<T>( _e[0] / rhs._e[0], _e[1] / rhs._e[1] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::operator+=( const Vec2<T>& rhs )
{
   _e[0] += rhs._e[0];
   _e[1] += rhs._e[1];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::operator-=( const Vec2<T>& rhs )
{
   _e[0] -= rhs._e[0];
   _e[1] -= rhs._e[1];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::operator*=( const T& rhs )
{
   _e[0] *= rhs;
   _e[1] *= rhs;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::operator*=( const Vec2<T>& rhs )
{
   _e[0] *= rhs._e[0];
   _e[1] *= rhs._e[1];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::operator/=( const T& rhs )
{
   _e[0] /= rhs;
   _e[1] /= rhs;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::operator/=( const Vec2<T>& rhs )
{
   _e[0] /= rhs._e[0];
   _e[1] /= rhs._e[1];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
Vec2<T>::operator=( const Vec2<T>& rhs )
{
   _e[0] = rhs._e[0];
   _e[1] = rhs._e[1];
   return *this;
}


//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Vec2<T>::operator==( const Vec2<T>& rhs ) const
{
   return _e[0] == rhs._e[0] && _e[1] == rhs._e[1];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Vec2<T>::operator!=( const Vec2<T>& rhs ) const
{
   return _e[0] != rhs._e[0] || _e[1] != rhs._e[1];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Vec2<T>::equal( const Vec2<T>& vec, const T threshold ) const
{
   return CGM::equal( *this, vec, threshold );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Vec2<T>::operator()( int idx )
{
  return _e[idx];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T&
Vec2<T>::operator()( int idx ) const
{
  return _e[idx];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Vec2<T>::operator()( int xAxis, int yAxis ) const
{
  return Vec2<T>( _e[xAxis], _e[yAxis] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
operator*( const T& val, const Vec2<T>& vec )
{
   return Vec2<T>( vec(0) * val, vec(1) * val );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<( TextStream& stream, const Vec2<T>& vec )
{
   return stream << "(" << vec(0) << "," << vec(1) << ")";
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator>>( TextStream& stream, Vec2<T>& vec )
{
   char c;
   return stream >> c >> vec(0) >> c >> vec(1) >> c;
}

/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef Vec2< int >    Vec2i;
typedef Vec2< float >  Vec2f;
typedef Vec2< double > Vec2d;

NAMESPACE_END

#endif
