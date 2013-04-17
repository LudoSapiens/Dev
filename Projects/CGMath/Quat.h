/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_QUAT_H
#define CGMATH_QUAT_H

#include <CGMath/Mat4.h>
#include <CGMath/Vec3.h>
#include <CGMath/CGMath.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Quat
==============================================================================*/

//! Quaternion class.

template< typename T >
class Quat
{

public:

   /*----- static methods -----*/

   inline static Quat identity();
   inline static Vec3<T> rotateX( const Vec3<T>& v, const T& angle );
   inline static Vec3<T> rotateY( const Vec3<T>& v, const T& angle );
   inline static Vec3<T> rotateZ( const Vec3<T>& v, const T& angle );
   inline static Quat axisAngle( const Vec3<T>& axis, const T& angle );
   inline static Quat axisCir( const Vec3<T>& axis, const T& cir );
   inline static Quat axisCos( const Vec3<T>& axis, const T& cosAngle );
   inline static Quat axes( const Vec3<T>& x, const Vec3<T>& y, const Vec3<T>& z );
   inline static Quat axes( const Vec3<T>& x, const Vec3<T>& y );
   inline static Quat eulerX( const T& angle );
   inline static Quat eulerY( const T& angle );
   inline static Quat eulerZ( const T& angle );
   inline static Quat eulerXCos( const T& cosAngle );
   inline static Quat eulerYCos( const T& cosAngle );
   inline static Quat eulerZCos( const T& cosAngle );
   inline static Quat eulerXYZ( const Vec3<T>& angles );
   inline static Quat eulerXZY( const Vec3<T>& angles );
   inline static Quat eulerYXZ( const Vec3<T>& angles );
   inline static Quat eulerYZX( const Vec3<T>& angles );
   inline static Quat eulerZXY( const Vec3<T>& angles );
   inline static Quat eulerZYX( const Vec3<T>& angles );
   inline static Quat twoVecs( const Vec3<T>& vFrom, const Vec3<T>& vTo );
   inline static Quat twoVecsNorm( const Vec3<T>& vFrom, const Vec3<T>& vTo );
   inline static Quat twoQuats( const Quat<T>& qFrom, const Quat<T>& qTo );

   inline static       Quat&  as(       T* ptr );
   inline static const Quat&  as( const T* ptr );

   /*----- methods -----*/

   Quat() {}
   Quat( const T& x, const T& y, const T& z, const T& w );
   Quat( const Vec3<T>& vec, const T& scalar );
   Quat( const Quat<T>& quat );
   template< typename S > Quat( const Quat<S>& quat );
   ~Quat(){}

   T* ptr();
   const T* ptr() const;

   Vec3<T>& vec();
   T& x();
   T& y();
   T& z();
   T& w();
   const Vec3<T>& vec() const;
   const T& x() const;
   const T& y() const;
   const T& z() const;
   const T& w() const;

   Vec3<T> toEulerXYZ() const;
   Vec3<T> toEulerXZY() const;
   Vec3<T> toEulerYXZ() const;
   Vec3<T> toEulerYZX() const;
   Vec3<T> toEulerZXY() const;
   Vec3<T> toEulerZYX() const;

   T norm() const;
   T dot( const Quat<T>& quat ) const;

   Quat& conjugate();
   Quat& inverse();
   Quat& negate();
   Quat& normalize();

   Quat  getConjugate() const;
   Quat  getInversed() const;
   Quat  getNormalized() const;

   template< typename S > void toAxisAngle( Vec3<S>& axis, S& angle ) const;
   template< typename S > void toAxisCir( Vec3<S>& axis, S& cir ) const;
   template< typename S > void toAxisCos( Vec3<S>& axis, S& cosAngle ) const;
   Mat4<T> toMatrix() const;
   Mat3<T> toMatrix3() const;
   void    getAxes( Vec3<T>& x, Vec3<T>& y, Vec3<T>& z ) const;
   void    getAxesXY( Vec3<T>& x, Vec3<T>& y ) const;
   void    getAxesXZ( Vec3<T>& x, Vec3<T>& z ) const;
   void    getAxesYZ( Vec3<T>& y, Vec3<T>& z ) const;
   Vec3<T> getAxisX() const;
   Vec3<T> getAxisY() const;
   Vec3<T> getAxisZ() const;

   Quat  getAxisAligned() const;
   Quat  getAxisAlignedFullSearch() const;

   Quat slerp( const Quat<T>& quat, T t ) const;
   bool clampedSlerp( const Quat<T>& quat, T t, T maxAngle, Quat<T>& dst ) const;
   Quat nlerp( const Quat<T>& quat, T t ) const;

   Quat  operator+( const Quat<T>& quat ) const;
   Quat  operator-( const Quat<T>& quat ) const;
   Quat  operator-() const;
   Quat  operator*( const Quat<T>& quat ) const;
   Quat  operator*( const T& val  ) const;
   Quat  operator/( const T& val ) const;

   Vec3<T> operator*( const Vec3<T>& vec ) const;

   Quat& operator+=( const Quat<T>& quat );
   Quat& operator-=( const Quat<T>& quat );
   Quat& operator*=( const Quat<T>& quat );
   Quat& operator*=( const T& val );
   Quat& operator/=( const T& val );
   Quat& operator=( const Quat& quat );

   bool operator==( const Quat<T>& rhs ) const;
   bool operator!=( const Quat<T>& rhs ) const;

   T& operator()( int idx );
   const T& operator()( int idx ) const;

private:

   /*----- methods -----*/
   static const Quat&  aa24( uint idx );

   /*----- data members -----*/

   Vec3<T> _vec;
   T       _scalar;
};

//------------------------------------------------------------------------------
//!
template< typename T > inline T*
Quat<T>::ptr()
{
   return _vec.ptr();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T*
Quat<T>::ptr() const
{
   return _vec.ptr();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::identity()
{
   return Quat( 0, 0, 0, 1 );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Quat<T>::rotateX( const Vec3<T>& v, const T& angle )
{
   T halfAngle = (T)(0.5) * angle;
   T sina      = CGM::sin( halfAngle );
   T cosa      = CGM::cos( halfAngle );

   // Compute coefficients.
   T x2 = sina + sina;
   T xx = sina * x2;
   T wx = cosa * x2;

   return Vec3<T>( v(0), v(1)*(1-xx) - v(2)*wx, v(1)*wx + v(2)*(1-xx) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Quat<T>::rotateY( const Vec3<T>& v, const T& angle )
{
   T halfAngle = (T)(0.5) * angle;
   T sina      = CGM::sin( halfAngle );
   T cosa      = CGM::cos( halfAngle );

   // Compute coefficients.
   T y2 = sina + sina;
   T yy = sina * y2;
   T wy = cosa * y2;

   return Vec3<T>( v(0)*(1-yy) + v(2)*wy, v(1), v(2)*(1-yy)-v(0)*wy );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Quat<T>::rotateZ( const Vec3<T>& v, const T& angle )
{
   T halfAngle = (T)(0.5) * angle;
   T sina      = CGM::sin( halfAngle );
   T cosa      = CGM::cos( halfAngle );

   // Compute coefficients.
   T z2 = sina + sina;
   T zz = sina * z2;
   T wz = cosa * z2;

   return Vec3<T>( v(0)*(1-zz) - v(1)*wz, v(0)*wz + v(1)*(1-zz), v(2) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::axisAngle( const Vec3<T>& axis, const T& angle )
{
   T halfAngle = (T)(0.5) * angle;
   T sina      = CGM::sin( halfAngle );

   return Quat<T>( axis * sina, CGM::cos( halfAngle ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::axisCir( const Vec3<T>& axis, const T& cir )
{
   return axisAngle( axis, CGM::cirToRad( cir ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::axisCos( const Vec3<T>& axis, const T& cosAngle )
{
   if( cosAngle <= (T)-1 )
   {
      return Quat<T>( axis, (T)0 );
   }
   else if( cosAngle >= (T)1 )
   {
      return Quat<T>( Vec3<T>::zero(), (T)1 );
   }
   else
   {
      // The following sina code uses a 0 instead of a 1
      // in Visual Studio 2011 Preview 11.0.40825.2 PREREL,
      // so we rewrote it (and shuddered).
      //T sina      = CGM::sqrt( (T)(0.5) * ((T)1 - cosAngle) );
      //T scalar    = CGM::sqrt( (T)(0.5) * ((T)1 + cosAngle) );
      T sina      = CGM::sqrt( T(0.5) - (cosAngle*T(0.5)) );
      T scalar    = CGM::sqrt( T(0.5) + (cosAngle*T(0.5)) );
      return Quat<T>( axis*sina, scalar );
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::axes( const Vec3<T>& x, const Vec3<T>& y, const Vec3<T>& z )
{
   Quat<T> tmp;

   T   tr, s;
   int i, j, k;

   int nxt[3] = {1, 2, 0};

   tr = x(0) + y(1) + z(2);

   // check the diagonal
   if( tr > (T)0 )
   {
      s = CGM::sqrt( tr + 1 );
      tmp._scalar = s * (T)0.5;
      s = (T)0.5 / s;
      tmp._vec(0) = ( y(2) - z(1) ) * s;
      tmp._vec(1) = ( z(0) - x(2) ) * s;
      tmp._vec(2) = ( x(1) - y(0) ) * s;
   }
   else
   {
      // diagonal is negative
      i = 0;
      if( y(1) > x(0) )
      {
         i = 1;
      }

      T m[3][3];
      m[0][0] = x(0);
      m[0][1] = y(0);
      m[0][2] = z(0);
      m[1][0] = x(1);
      m[1][1] = y(1);
      m[1][2] = z(1);
      m[2][0] = x(2);
      m[2][1] = y(2);
      m[2][2] = z(2);

      if( z(2) > m[i][i] )
      {
         i = 2;
      }

      j = nxt[i];
      k = nxt[j];

      s = CGM::sqrt( m[i][i] - m[j][j] - m[k][k] + 1 );

      tmp._vec(i) = s * (T)0.5;

      if( s != (T)0 )
      {
         s = (T)0.5 / s;
      }

      tmp._scalar = ( m[k][j] - m[j][k]) * s;
      tmp._vec(j) = ( m[i][j] + m[j][i]) * s;
      tmp._vec(k) = ( m[i][k] + m[k][i]) * s;
   }
   return tmp;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::axes( const Vec3<T>& x, const Vec3<T>& y )
{
   return axes( x, y, cross(x, y) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::eulerX( const T& angle )
{
   T halfAngle = (T)(0.5) * angle;
   T sina      = CGM::sin( halfAngle );
   T cosa      = CGM::cos( halfAngle );

   return Quat<T>( sina, (T)0, (T)0, cosa );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::eulerY( const T& angle )
{
   T halfAngle = (T)(0.5) * angle;
   T sina      = CGM::sin( halfAngle );
   T cosa      = CGM::cos( halfAngle );

   return Quat<T>( (T)0, sina, (T)0, cosa );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::eulerZ( const T& angle )
{
   T halfAngle = (T)(0.5) * angle;
   T sina      = CGM::sin( halfAngle );
   T cosa      = CGM::cos( halfAngle );

   return Quat<T>( (T)0, (T)0, sina, cosa );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::eulerXCos( const T& cosAngle )
{
   if( cosAngle <= (T)-1 )
   {
      return Quat<T>( (T)1, (T)0, (T)0, (T)0 );
   }
   else if( cosAngle >= (T)1 )
   {
      return Quat<T>( Vec3<T>::zero(), (T)1 );
   }
   else
   {
      T sina      = CGM::sqrt( (T)(0.5) * ((T)1 - cosAngle) );
      T scalar    = CGM::sqrt( (T)(0.5) * ((T)1 + cosAngle) );
      return Quat<T>( sina, (T)0, (T)0, scalar );
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::eulerYCos( const T& cosAngle )
{
   if( cosAngle <= (T)-1 )
   {
      return Quat<T>( (T)0, (T)1, (T)0, (T)0 );
   }
   else if( cosAngle >= (T)1 )
   {
      return Quat<T>( Vec3<T>::zero(), (T)1 );
   }
   else
   {
      T sina      = CGM::sqrt( (T)(0.5) * ((T)1 - cosAngle) );
      T scalar    = CGM::sqrt( (T)(0.5) * ((T)1 + cosAngle) );
      return Quat<T>( (T)0, sina, (T)0, scalar );
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::eulerZCos( const T& cosAngle )
{
   if( cosAngle <= (T)-1 )
   {
      return Quat<T>( (T)0, (T)0, (T)1, (T)0 );
   }
   else if( cosAngle >= (T)1 )
   {
      return Quat<T>( Vec3<T>::zero(), (T)1 );
   }
   else
   {
      T sina      = CGM::sqrt( (T)(0.5) * ((T)1 - cosAngle) );
      T scalar    = CGM::sqrt( (T)(0.5) * ((T)1 + cosAngle) );
      return Quat<T>( (T)0, (T)0, sina, scalar );
   }
}

//------------------------------------------------------------------------------
//! Returns Rx(a.x) * Ry(a.y) * Rz(a.z).
template< typename T > inline Quat<T>
Quat<T>::eulerXYZ( const Vec3<T>& angles )
{
   Vec3<T> h( angles * (T)0.5 );
   Vec3<T> c( CGM::cos(h.x), CGM::cos(h.y), CGM::cos(h.z) );
   Vec3<T> s( CGM::sin(h.x), CGM::sin(h.y), CGM::sin(h.z) );

   T c2c1 = c(1)*c(2);
   T s2c1 = s(1)*c(2);
   T c2s1 = c(1)*s(2);
   T s2s1 = s(1)*s(2);
   T p0   = c(0)*c2c1 - s(0)*s2s1;
   T p1   = c(0)*c2s1 + s(0)*s2c1;
   T p2   = c(0)*s2c1 - s(0)*c2s1;
   T p3   = s(0)*c2c1 + c(0)*s2s1;

   return Quat<T>( p3, p2, p1, p0 );
}

//------------------------------------------------------------------------------
//! Returns Rx(a.x) * Rz(a.z) * Ry(a.y).
template< typename T > inline Quat<T>
Quat<T>::eulerXZY( const Vec3<T>& angles )
{
   Vec3<T> h( angles * (T)0.5 );
   Vec3<T> c( CGM::cos(h.x), CGM::cos(h.y), CGM::cos(h.z) );
   Vec3<T> s( CGM::sin(h.x), CGM::sin(h.y), CGM::sin(h.z) );

   T c2c1 = c(2)*c(1);
   T s2c1 = s(2)*c(1);
   T c2s1 = c(2)*s(1);
   T s2s1 = s(2)*s(1);
   T p0   = c(0)*c2c1 + s(0)*s2s1;
   T p1   = c(0)*c2s1 - s(0)*s2c1;
   T p2   = c(0)*s2c1 + s(0)*c2s1;
   T p3   = s(0)*c2c1 - c(0)*s2s1;

   return Quat<T>( p3, p1, p2, p0 ); // Why not (p2,p3,p1,p0)?
}

//------------------------------------------------------------------------------
//! Returns Ry(a.y) * Rx(a.x) * Rz(a.z).
template< typename T > inline Quat<T>
Quat<T>::eulerYXZ( const Vec3<T>& angles )
{
   Vec3<T> h( angles * (T)0.5 );
   Vec3<T> c( CGM::cos(h.x), CGM::cos(h.y), CGM::cos(h.z) );
   Vec3<T> s( CGM::sin(h.x), CGM::sin(h.y), CGM::sin(h.z) );

   T c2c1 = c(0)*c(2);
   T s2c1 = s(0)*c(2);
   T c2s1 = c(0)*s(2);
   T s2s1 = s(0)*s(2);
   T p0   = c(1)*c2c1 + s(1)*s2s1;
   T p1   = c(1)*c2s1 - s(1)*s2c1;
   T p2   = c(1)*s2c1 + s(1)*c2s1;
   T p3   = s(1)*c2c1 - c(1)*s2s1;

   return Quat<T>( p2, p3, p1, p0 ); // Why not (p3,p1,p2,p0)?
}

//------------------------------------------------------------------------------
//! Returns Ry(a.y) * Rz(a.z) * Rx(a.x).
template< typename T > inline Quat<T>
Quat<T>::eulerYZX( const Vec3<T>& angles )
{
   Vec3<T> h( angles * (T)0.5 );
   Vec3<T> c( CGM::cos(h.x), CGM::cos(h.y), CGM::cos(h.z) );
   Vec3<T> s( CGM::sin(h.x), CGM::sin(h.y), CGM::sin(h.z) );

   T c2c1 = c(2)*c(0);
   T s2c1 = s(2)*c(0);
   T c2s1 = c(2)*s(0);
   T s2s1 = s(2)*s(0);
   T p0   = c(1)*c2c1 - s(1)*s2s1;
   T p1   = c(1)*c2s1 + s(1)*s2c1;
   T p2   = c(1)*s2c1 - s(1)*c2s1;
   T p3   = s(1)*c2c1 + c(1)*s2s1;

   return Quat<T>( p1, p3, p2, p0 );
}

//------------------------------------------------------------------------------
//! Returns Rz(a.z) * Rx(a.x) * Ry(a.y).
template< typename T > inline Quat<T>
Quat<T>::eulerZXY( const Vec3<T>& angles )
{
   Vec3<T> h( angles * (T)0.5 );
   Vec3<T> c( CGM::cos(h.x), CGM::cos(h.y), CGM::cos(h.z) );
   Vec3<T> s( CGM::sin(h.x), CGM::sin(h.y), CGM::sin(h.z) );

   T c2c1 = c(0)*c(1);
   T s2c1 = s(0)*c(1);
   T c2s1 = c(0)*s(1);
   T s2s1 = s(0)*s(1);
   T p0   = c(2)*c2c1 - s(2)*s2s1;
   T p1   = c(2)*c2s1 + s(2)*s2c1;
   T p2   = c(2)*s2c1 - s(2)*c2s1;
   T p3   = s(2)*c2c1 + c(2)*s2s1;

   return Quat<T>( p2, p1, p3, p0 );
}

//------------------------------------------------------------------------------
//! Returns Rz(a.z) * Ry(a.y) * Rx(a.x).
template< typename T > inline Quat<T>
Quat<T>::eulerZYX( const Vec3<T>& angles )
{
   Vec3<T> h( angles * (T)0.5 );
   Vec3<T> c( CGM::cos(h.x), CGM::cos(h.y), CGM::cos(h.z) );
   Vec3<T> s( CGM::sin(h.x), CGM::sin(h.y), CGM::sin(h.z) );

   T c2c1 = c(1)*c(0);
   T s2c1 = s(1)*c(0);
   T c2s1 = c(1)*s(0);
   T s2s1 = s(1)*s(0);
   T p0   = c(2)*c2c1 + s(2)*s2s1;
   T p1   = c(2)*c2s1 - s(2)*s2c1;
   T p2   = c(2)*s2c1 + s(2)*c2s1;
   T p3   = s(2)*c2c1 - c(2)*s2s1;

   return Quat<T>( p1, p2, p3, p0 );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::twoVecs( const Vec3<T>& vFrom, const Vec3<T>& vTo )
{
   Vec3<T> axis = vFrom.cross( vTo );

   T len = axis.length();

   if( len < (T)0.0001 )
   {
      if( vFrom.dot( vTo ) < (T)0 )
      {
         // Opposite direction.
         axis = Vec3<T>::perpendicular( vFrom );
         len  = (T)0;
         return Quat<T>( axis.x, axis.y, axis.z, len );
      }
      else
      {
         // Same direction.
         return Quat<T>( Vec3<T>::zero(), (T)1 );
      }
   }

   axis *= (T)(1) / len;

   T cosAngle = vFrom.dot( vTo ) / ( vFrom.length() * vTo.length() );

   return axisCos( axis, cosAngle );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::twoVecsNorm( const Vec3<T>& vFrom, const Vec3<T>& vTo )
{
   Vec3<T> bisect = vFrom + vTo;
   bisect.normalizeSafe();

   T cosHalf = NAMESPACE::dot( vFrom, bisect );
   if( cosHalf != T(0) )
   {
      Vec3<T> perp = NAMESPACE::cross( vFrom, bisect );
      return Quat<T>( perp.x, perp.y, perp.z, cosHalf );
   }
   else
   {
      // Colinear case.
      T invLen;
      if( CGM::abs(vFrom.x) >= CGM::abs(vFrom.y) )
      {
         // Either X or Z is the largest component of vFrom.
         invLen = CGM::invSqrt( vFrom.x*vFrom.x + vFrom.z*vFrom.z );
         return Quat<T>( -vFrom.z*invLen, T(0), vFrom.x*invLen, cosHalf );
      }
      else
      {
         // Either Y or Z is the largest component of vFrom.
         invLen = CGM::invSqrt( vFrom.y*vFrom.y + vFrom.z*vFrom.z );
         return Quat<T>( T(0), vFrom.z*invLen, -vFrom.y*invLen, cosHalf );
      }
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::twoQuats( const Quat<T>& qFrom, const Quat<T>& qTo )
{
   return qTo * qFrom.getInversed();
}

//!
template< typename T > inline Quat<T>&
Quat<T>::as( T* ptr )
{
   return reinterpret_cast<Quat<T>&>( *ptr );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Quat<T>&
Quat<T>::as( const T* ptr )
{
   return reinterpret_cast<const Quat<T>&>( *ptr );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Quat<T>::Quat( const T& x, const T& y, const T& z, const T& w )
   : _vec( x, y, z ), _scalar( w )
{}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Quat<T>::Quat( const Vec3<T>& vec, const T& scalar )
   : _vec( vec ), _scalar( scalar )
{}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Quat<T>::Quat( const Quat<T>& quat )
   : _vec( quat._vec ), _scalar( quat._scalar )
{}

//------------------------------------------------------------------------------
//!
template< typename T > template< typename S > inline
Quat<T>::Quat( const Quat<S>& quat )
{
   _vec = quat.vec();
   _scalar = (T)quat.w();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Quat<T>::vec()
{
   return _vec;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Quat<T>::x()
{
   return _vec(0);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Quat<T>::y()
{
   return _vec(1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Quat<T>::z()
{
   return _vec(2);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Quat<T>::w()
{
   return _scalar;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec3<T>&
Quat<T>::vec() const
{
   return _vec;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T&
Quat<T>::x() const
{
   return _vec(0);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T&
Quat<T>::y() const
{
   return _vec(1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T&
Quat<T>::z() const
{
   return _vec(2);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T&
Quat<T>::w() const
{
   return _scalar;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Quat<T>::toEulerXYZ() const
{
   const T p0 = _scalar;
   const T p1 = _vec(2);
   const T p2 = _vec(1);
   const T p3 = _vec(0);

   const T p2xp2 = p2 * p2;

   const T a = (T)2 * (p0*p2 + p1*p3);

   if( CGM::abs(a) < (T)0.999 )
   {
      return Vec3<T>(
         CGM::atan2( (T)2*( p0*p3 - p1*p2 ), (T)1 - (T)2*( p2xp2 + p3*p3 ) ),
         CGM::asin ( a ),
         CGM::atan2( (T)2*( p0*p1 - p2*p3 ), (T)1 - (T)2*( p1*p1 + p2xp2 ) )
      );
   }
   else
   {
      return Vec3<T>(
         0,
         CGM::copySign( CGConst<T>::pi_2(), a ),
         (T)2 * CGM::atan2( p1, p0 )
      );
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Quat<T>::toEulerXZY() const
{
   const T p0 = _scalar;
   const T p1 = _vec(1);
   const T p2 = _vec(2);
   const T p3 = _vec(0);

   const T p2xp2 = p2 * p2;

   const T a = (T)2 * (p0*p2 - p1*p3);

   if( CGM::abs(a) < (T)0.999 )
   {
      // Why are X and Y flipped?
      return Vec3<T>(
         CGM::atan2( (T)2*( p0*p3 + p1*p2 ), (T)1 - (T)2*( p2xp2 + p3*p3 ) ),
         CGM::atan2( (T)2*( p0*p1 + p2*p3 ), (T)1 - (T)2*( p1*p1 + p2xp2 ) ),
         CGM::asin ( a )
      );
   }
   else
   {
      return Vec3<T>(
         0,
         (T)2 * CGM::atan2( p1, p0 ),
         CGM::copySign( CGConst<T>::pi_2(), a )
      );
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Quat<T>::toEulerYXZ() const
{
   const T p0 = _scalar;
   const T p1 = _vec(2);
   const T p2 = _vec(0);
   const T p3 = _vec(1);

   const T p2xp2 = p2 * p2;

   const T a = (T)2 * (p0*p2 - p1*p3);

   if( CGM::abs(a) < (T)0.999 )
   {
      return Vec3<T>(
         CGM::asin ( a ),
         CGM::atan2( (T)2*( p0*p3 + p1*p2 ), (T)1 - (T)2*( p2xp2 + p3*p3 ) ),
         CGM::atan2( (T)2*( p0*p1 + p2*p3 ), (T)1 - (T)2*( p1*p1 + p2xp2 ) )
      );
   }
   else
   {
      return Vec3<T>(
         CGM::copySign( CGConst<T>::pi_2(), a ),
         0,
         (T)2 * CGM::atan2( p1, p0 )
      );
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Quat<T>::toEulerYZX() const
{
   const T p0 = _scalar;
   const T p1 = _vec(0);
   const T p2 = _vec(2);
   const T p3 = _vec(1);

   const T p2xp2 = p2 * p2;

   const T a = (T)2 * (p0*p2 + p1*p3);

   if( CGM::abs(a) < (T)0.999 )
   {
      return Vec3<T>(
         CGM::atan2( (T)2*( p0*p1 - p2*p3 ), (T)1 - (T)2*( p1*p1 + p2xp2 ) ),
         CGM::atan2( (T)2*( p0*p3 - p1*p2 ), (T)1 - (T)2*( p2xp2 + p3*p3 ) ),
         CGM::asin ( a )
      );
   }
   else
   {
      return Vec3<T>(
         (T)2 * CGM::atan2( p1, p0 ),
         0,
         CGM::copySign( CGConst<T>::pi_2(), a )
      );
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Quat<T>::toEulerZXY() const
{
   const T p0 = _scalar;
   const T p1 = _vec(1);
   const T p2 = _vec(0);
   const T p3 = _vec(2);

   const T p2xp2 = p2 * p2;

   const T a = (T)2 * (p0*p2 + p1*p3);

   if( CGM::abs(a) < (T)0.999 )
   {
      return Vec3<T>(
         CGM::asin ( a ),
         CGM::atan2( (T)2*( p0*p1 - p2*p3 ), (T)1 - (T)2*( p1*p1 + p2xp2 ) ),
         CGM::atan2( (T)2*( p0*p3 - p1*p2 ), (T)1 - (T)2*( p2xp2 + p3*p3 ) )
      );
   }
   else
   {
      return Vec3<T>(
         CGM::copySign( CGConst<T>::pi_2(), a ),
         (T)2 * CGM::atan2( p1, p0 ),
         0
      );
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Quat<T>::toEulerZYX() const
{
   const T p0 = _scalar;
   const T p1 = _vec(0);
   const T p2 = _vec(1);
   const T p3 = _vec(2);

   const T p2xp2 = p2 * p2;

   const T a = (T)2 * (p0*p2 - p1*p3);

   if( CGM::abs(a) < (T)0.999 )
   {
      return Vec3<T>(
         CGM::atan2( (T)2*( p0*p1 + p2*p3 ), (T)1 - (T)2*( p1*p1 + p2xp2 ) ),
         CGM::asin ( a ),
         CGM::atan2( (T)2*( p0*p3 + p1*p2 ), (T)1 - (T)2*( p2xp2 + p3*p3 ) )
      );
   }
   else
   {
      return Vec3<T>(
         (T)2 * CGM::atan2( p1, p0 ),
         CGM::copySign( CGConst<T>::pi_2(), a ),
         0
      );
   }
}


//------------------------------------------------------------------------------
//!
template< typename T > inline T
Quat<T>::norm() const
{
   return CGM::sqrt( _vec.sqrLength() + _scalar*_scalar );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Quat<T>::dot
( const Quat<T>& quat ) const
{
   return _vec.dot( quat._vec ) + _scalar*quat._scalar;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>&
Quat<T>::conjugate()
{
   _vec.negate();
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>&
Quat<T>::inverse()
{
   T div = (T)1 / norm();
   _vec(0) *= -div;
   _vec(1) *= -div;
   _vec(2) *= -div;
   _scalar *=  div;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>&
Quat<T>::negate()
{
   _vec.negate();
   _scalar = -_scalar;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>&
Quat<T>::normalize()
{
   T div = (T)1 / norm();
   _vec(0) *= div;
   _vec(1) *= div;
   _vec(2) *= div;
   _scalar *= div;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::getConjugate() const
{
   return Quat<T>( -_vec(0), -_vec(1), -_vec(2), _scalar );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::getInversed() const
{
   T div = (T)1 / norm();
   return Quat<T>(
      _vec(0) * -div,
      _vec(1) * -div,
      _vec(2) * -div,
      _scalar *  div
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::getNormalized() const
{
   T div = (T)1 / norm();
   return Quat<T>(
      _vec(0) * div,
      _vec(1) * div,
      _vec(2) * div,
      _scalar * div
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > template< typename S > inline void
Quat<T>::toAxisAngle( Vec3<S>& axis, S& angle ) const
{
   S lSqr = (S)_vec.sqrLength();
   if( !CGM::equal( lSqr, (S)0, (S)CGM::EqualityThresholdSqr ) )
   {
      S l = CGM::sqrt( lSqr );
      S l_inv = (S)1 / l;
      axis = _vec;
      axis *= l_inv;
      S scalar = CGM::clamp( _scalar, (S)-1, (S)1 );
      angle = (S)2 * CGM::acos( scalar );
   }
   else
   {
      axis = Vec3<S>( (S)1, (S)0, (S)0 );
      angle = (S)0;
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > template< typename S > inline void
Quat<T>::toAxisCir( Vec3<S>& axis, S& angle ) const
{
   toAxisAngle( axis, angle );
   angle = CGM::radToCir( angle );
}

//------------------------------------------------------------------------------
//!
template< typename T > template< typename S > inline void
Quat<T>::toAxisCos( Vec3<S>& axis, S& cosAngle ) const
{
   S lSqr = (S)_vec.sqrLength();
   if( !CGM::equal( lSqr, (S)0, (S)CGM::EqualityThresholdSqr ) )
   {
      S l = CGM::sqrt( lSqr );
      S l_inv = (S)1 / l;
      axis = _vec;
      axis *= l_inv;
      cosAngle = (S)2 * _scalar;
   }
   else
   {
      axis = Vec3<S>( (S)1, (S)0, (S)0 );
      cosAngle = (S)1;
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Quat<T>::toMatrix() const
{
   // calculate coefficients
   T x2 = _vec(0) + _vec(0);
   T y2 = _vec(1) + _vec(1);
   T z2 = _vec(2) + _vec(2);
   T xx = _vec(0) * x2;
   T xy = _vec(0) * y2;
   T xz = _vec(0) * z2;
   T yy = _vec(1) * y2;
   T yz = _vec(1) * z2;
   T zz = _vec(2) * z2;
   T wx = _scalar * x2;
   T wy = _scalar * y2;
   T wz = _scalar * z2;

   return Mat4<T>(
      (T)1 - ( yy + zz ), xy - wz,            xz + wy,            (T)0,
      xy + wz,            (T)1 - ( xx + zz ), yz - wx,            (T)0,
      xz - wy,            yz + wx,            (T)1 - ( xx + yy ), (T)0,
      (T)0,               (T)0,               (T)0,               (T)1
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Quat<T>::toMatrix3() const
{
   // calculate coefficients
   T x2 = _vec(0) + _vec(0);
   T y2 = _vec(1) + _vec(1);
   T z2 = _vec(2) + _vec(2);
   T xx = _vec(0) * x2;
   T xy = _vec(0) * y2;
   T xz = _vec(0) * z2;
   T yy = _vec(1) * y2;
   T yz = _vec(1) * z2;
   T zz = _vec(2) * z2;
   T wx = _scalar * x2;
   T wy = _scalar * y2;
   T wz = _scalar * z2;

   return Mat3<T>(
      (T)1 - ( yy + zz ), xy - wz,            xz + wy,
      xy + wz,            (T)1 - ( xx + zz ), yz - wx,
      xz - wy,            yz + wx,            (T)1 - ( xx + yy )
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Quat<T>::getAxes( Vec3<T>& x, Vec3<T>& y, Vec3<T>& z ) const
{
   // calculate coefficients
   T x2 = _vec(0) + _vec(0);
   T y2 = _vec(1) + _vec(1);
   T z2 = _vec(2) + _vec(2);
   T xx = _vec(0) * x2;
   T xy = _vec(0) * y2;
   T xz = _vec(0) * z2;
   T yy = _vec(1) * y2;
   T yz = _vec(1) * z2;
   T zz = _vec(2) * z2;
   T wx = _scalar * x2;
   T wy = _scalar * y2;
   T wz = _scalar * z2;

   x(0) = (T)1 - ( yy + zz );
   x(1) = xy + wz;
   x(2) = xz - wy;

   y(0) = xy - wz;
   y(1) = (T)1 - ( xx + zz );
   y(2) = yz + wx;

   z(0) = xz + wy;
   z(1) = yz - wx;
   z(2) = (T)1 - ( xx + yy );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Quat<T>::getAxesXY( Vec3<T>& x, Vec3<T>& y ) const
{
   // calculate coefficients
   T x2 = _vec(0) + _vec(0);
   T y2 = _vec(1) + _vec(1);
   T z2 = _vec(2) + _vec(2);
   T xx = _vec(0) * x2;
   T xy = _vec(0) * y2;
   T xz = _vec(0) * z2;
   T yy = _vec(1) * y2;
   T yz = _vec(1) * z2;
   T zz = _vec(2) * z2;
   T wx = _scalar * x2;
   T wy = _scalar * y2;
   T wz = _scalar * z2;

   x(0) = (T)1 - ( yy + zz );
   x(1) = xy + wz;
   x(2) = xz - wy;

   y(0) = xy - wz;
   y(1) = (T)1 - ( xx + zz );
   y(2) = yz + wx;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Quat<T>::getAxesXZ( Vec3<T>& x, Vec3<T>& z ) const
{
   // calculate coefficients
   T x2 = _vec(0) + _vec(0);
   T y2 = _vec(1) + _vec(1);
   T z2 = _vec(2) + _vec(2);
   T xx = _vec(0) * x2;
   T xy = _vec(0) * y2;
   T xz = _vec(0) * z2;
   T yy = _vec(1) * y2;
   T yz = _vec(1) * z2;
   T zz = _vec(2) * z2;
   T wx = _scalar * x2;
   T wy = _scalar * y2;
   T wz = _scalar * z2;

   x(0) = (T)1 - ( yy + zz );
   x(1) = xy + wz;
   x(2) = xz - wy;

   z(0) = xz + wy;
   z(1) = yz - wx;
   z(2) = (T)1 - ( xx + yy );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Quat<T>::getAxesYZ( Vec3<T>& y, Vec3<T>& z ) const
{
   // calculate coefficients
   T x2 = _vec(0) + _vec(0);
   T y2 = _vec(1) + _vec(1);
   T z2 = _vec(2) + _vec(2);
   T xx = _vec(0) * x2;
   T xy = _vec(0) * y2;
   T xz = _vec(0) * z2;
   T yy = _vec(1) * y2;
   T yz = _vec(1) * z2;
   T zz = _vec(2) * z2;
   T wx = _scalar * x2;
   T wy = _scalar * y2;
   T wz = _scalar * z2;

   y(0) = xy - wz;
   y(1) = (T)1 - ( xx + zz );
   y(2) = yz + wx;

   z(0) = xz + wy;
   z(1) = yz - wx;
   z(2) = (T)1 - ( xx + yy );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Quat<T>::getAxisX() const
{
   // calculate coefficients
   T y2 = _vec(1) + _vec(1);
   T z2 = _vec(2) + _vec(2);
   T xy = _vec(0) * y2;
   T xz = _vec(0) * z2;
   T yy = _vec(1) * y2;
   T zz = _vec(2) * z2;
   T wy = _scalar * y2;
   T wz = _scalar * z2;

   return Vec3<T>( (T)1 - ( yy + zz ), xy + wz, xz - wy );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Quat<T>::getAxisY() const
{
   // calculate coefficients
   T x2 = _vec(0) + _vec(0);
   T y2 = _vec(1) + _vec(1);
   T z2 = _vec(2) + _vec(2);
   T xx = _vec(0) * x2;
   T xy = _vec(0) * y2;
   T yz = _vec(1) * z2;
   T zz = _vec(2) * z2;
   T wx = _scalar * x2;
   T wz = _scalar * z2;

   return Vec3<T>( xy - wz, 1 - ( xx + zz ), yz + wx );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Quat<T>::getAxisZ() const
{
   // calculate coefficients
   T x2 = _vec(0) + _vec(0);
   T y2 = _vec(1) + _vec(1);
   T z2 = _vec(2) + _vec(2);
   T xx = _vec(0) * x2;
   T xz = _vec(0) * z2;
   T yy = _vec(1) * y2;
   T yz = _vec(1) * z2;
   T wx = _scalar * x2;
   T wy = _scalar * y2;

   return Vec3<T>( xz + wy, yz - wx, (T)1 - ( xx + yy ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::getAxisAligned() const
{
#if 0
   Vec3<T> x, y;
   getAxesXY( x, y );
   //StdErr << " x=" << x << " y=" << y << nl;

   // Determine the X axis' major direction (one of 6 faces).
   uint maxX = x.maxComponent();
   uint posX = (x(maxX) < 0.0f) ? 0 : 1;
   uint codX = (maxX << 1) | posX; // 0:-X, 1:+X, 2:-Y, 3:+Y, 4:-Z, 5:+Z
   //StdErr << "maxX: " << maxX << " posX: " << posX << " codX: " << codX << nl;

   // Determine the Y axis' major direction perpendicular to X (one of 4 faces).
   Vec2<T> y2 = y( (maxX+1)%3, (maxX+2)%3 );
   //StdErr << "y2=" << y2 << nl;
   uint maxY = y2.maxComponent();
   uint posY = (y2(maxY) < 0.0f) ? 0 : 1;
   uint codY = (maxY << 1) | posY;
   //StdErr << "maxY: " << maxY << " posY: " << posY << " codY: " << codY << nl;

   uint code = (codX*4) + codY;
   //StdErr << "code: " << code << nl;
   return aa24(code);
#else
   return getAxisAlignedFullSearch();
#endif
}

template< typename T >  const Quat<T>&
Quat<T>::aa24( uint idx )
{
   // The 24 axis-aligned orientations.
   static const Quat<T> _aa24[] = {
      Quat<T>::axes( Vec3<T>(-1, 0, 0), Vec3<T>( 0,-1, 0) ), // -X, -Y
      Quat<T>::axes( Vec3<T>(-1, 0, 0), Vec3<T>( 0, 1, 0) ), // -X, +Y
      Quat<T>::axes( Vec3<T>(-1, 0, 0), Vec3<T>( 0, 0,-1) ), // -X, -Z
      Quat<T>::axes( Vec3<T>(-1, 0, 0), Vec3<T>( 0, 0, 1) ), // -X, +Z

      Quat<T>::axes( Vec3<T>( 1, 0, 0), Vec3<T>( 0,-1, 0) ), // +X, -Y
      Quat<T>::axes( Vec3<T>( 1, 0, 0), Vec3<T>( 0, 1, 0) ), // +X, +Y
      Quat<T>::axes( Vec3<T>( 1, 0, 0), Vec3<T>( 0, 0,-1) ), // +X, -Z
      Quat<T>::axes( Vec3<T>( 1, 0, 0), Vec3<T>( 0, 0, 1) ), // +X, +Z

      Quat<T>::axes( Vec3<T>( 0,-1, 0), Vec3<T>(-1, 0, 0) ), // -Y, -X
      Quat<T>::axes( Vec3<T>( 0,-1, 0), Vec3<T>( 1, 0, 0) ), // -Y, +X
      Quat<T>::axes( Vec3<T>( 0,-1, 0), Vec3<T>( 0, 0,-1) ), // -Y, -Z
      Quat<T>::axes( Vec3<T>( 0,-1, 0), Vec3<T>( 0, 0, 1) ), // -Y, +Z

      Quat<T>::axes( Vec3<T>( 0, 1, 0), Vec3<T>(-1, 0, 0) ), // +Y, -X
      Quat<T>::axes( Vec3<T>( 0, 1, 0), Vec3<T>( 1, 0, 0) ), // +Y, +X
      Quat<T>::axes( Vec3<T>( 0, 1, 0), Vec3<T>( 0, 0,-1) ), // +Y, -Z
      Quat<T>::axes( Vec3<T>( 0, 1, 0), Vec3<T>( 0, 0, 1) ), // +Y, +Z

      Quat<T>::axes( Vec3<T>( 0, 0,-1), Vec3<T>(-1, 0, 0) ), // -Z, -X
      Quat<T>::axes( Vec3<T>( 0, 0,-1), Vec3<T>( 1, 0, 0) ), // -Z, +X
      Quat<T>::axes( Vec3<T>( 0, 0,-1), Vec3<T>( 0,-1, 0) ), // -Z, -Y
      Quat<T>::axes( Vec3<T>( 0, 0,-1), Vec3<T>( 0, 1, 0) ), // -Z, +Y

      Quat<T>::axes( Vec3<T>( 0, 0, 1), Vec3<T>(-1, 0, 0) ), // +Z, -X
      Quat<T>::axes( Vec3<T>( 0, 0, 1), Vec3<T>( 1, 0, 0) ), // +Z, +X
      Quat<T>::axes( Vec3<T>( 0, 0, 1), Vec3<T>( 0,-1, 0) ), // +Z, -Y
      Quat<T>::axes( Vec3<T>( 0, 0, 1), Vec3<T>( 0, 1, 0) ), // +Z, +Y
   };

   return _aa24[idx];
}

template< typename T >  Quat<T>
Quat<T>::getAxisAlignedFullSearch() const
{

   // Find the axis which minimizes the rotation to get an alignment.
   Quat<T>  qInv  = getInversed();
   Quat<T>  tmpQ  = aa24(0) * qInv; // Equivalent to Quat::twoQuats( q, aa24[0] ), but with only one q1.getInversed().
   T        tmpW  = CGM::abs( tmpQ.w() );
   T        bestW = tmpW; // Range [-1, 1], larger is better, because if w is 1, it means an angle of 0.
   uint     bestQ = 0;
   for( uint i = 1; i < 24; ++i )
   {
      tmpQ = aa24(i) * qInv;
      tmpW = CGM::abs( tmpQ.w() );
      if( tmpW > bestW )
      {
         bestQ = i;
         bestW = tmpW;
      }
   }

   return aa24(bestQ);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::slerp( const Quat<T>& quat, T t ) const
{
   T cosa = dot( quat );

   T s1;
   T s2;
   T flip = (T)1;

   if( cosa < (T)0 )
   {
      cosa = -cosa;
      flip = -1;
   }

   // Linear interpolation, for near orientation.
   if( 1 - cosa < (T)0.0001 )
   {
      s1 = 1 - t;
      s2 = t * flip;
   }
   else
   {
      T angle   = CGM::acos( cosa );
      T sina    = CGM::sin( angle );
      T invsina = (T)1 / sina;
      s1 = CGM::sin( ( (T)1 - t ) * angle ) * invsina;
      s2 = CGM::sin( t * angle ) * invsina * flip;
   }

   return (*this * s1 + quat * s2).normalize();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Quat<T>::clampedSlerp( const Quat<T>& quat, T t, T maxAngle, Quat<T>& dst ) const
{
   bool finished = true;
   T cosa = dot( quat );

   T s1;
   T s2;
   T flip = (T)1;

   if( cosa < (T)0 )
   {
      cosa = -cosa;
      flip = -1;
   }

   // Linear interpolation, for near orientation.
   if( 1 - cosa < (T)0.0001 )
   {
      s1 = 1 - t;
      s2 = t * flip;
   }
   else
   {
      T angle   = CGM::acos( cosa );
      if( angle > maxAngle )
      {
         t *= maxAngle/angle;
         angle = maxAngle;
         finished = false;
      }
      T sina    = CGM::sin( angle );
      T invsina = (T)1 / sina;
      s1 = CGM::sin( ( (T)1 - t ) * angle ) * invsina;
      s2 = CGM::sin( t * angle ) * invsina * flip;
   }

   dst = (*this) * s1 + quat * s2;
   dst.normalize();

   return finished;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::nlerp( const Quat<T>& quat, T t ) const
{
   T cosa = dot( quat );
   T flip = cosa < (T)0 ? (T)-1 : (T)1;

   T s1 = 1 - t;
   T s2 = t * flip;

   return (*this * s1 + quat * s2).normalize();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::operator+( const Quat<T>& quat ) const
{
   return Quat<T>( _vec + quat._vec, _scalar + quat._scalar );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::operator-( const Quat<T>& quat ) const
{
   return Quat<T>( _vec - quat._vec, _scalar - quat._scalar );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::operator-() const
{
   return Quat<T>( -_vec, -_scalar );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::operator*( const Quat<T>& quat ) const
{
   T v0 = (_vec(2) - _vec(1)) * (quat._vec(1) - quat._vec(2));
   T v1 = (_scalar + _vec(0)) * (quat._scalar + quat._vec(0));
   T v2 = (_scalar - _vec(0)) * (quat._vec(1) + quat._vec(2));
   T v3 = (_vec(2) + _vec(1)) * (quat._scalar - quat._vec(0));
   T v4 = (_vec(2) - _vec(0)) * (quat._vec(0) - quat._vec(1));
   T v5 = (_vec(2) + _vec(0)) * (quat._vec(0) + quat._vec(1));
   T v6 = (_scalar + _vec(1)) * (quat._scalar - quat._vec(2));
   T v7 = (_scalar - _vec(1)) * (quat._scalar + quat._vec(2));
   T v8 = v5 + v6 + v7;
   T v9 = ( v4 + v8 ) / (T)2;

   return Quat<T>( v1+v9-v8, v2+v9-v7, v3+v9-v6, v0+v9-v5 );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::operator*( const T& val  ) const
{
   return Quat<T>( _vec * val, _scalar * val );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>
Quat<T>::operator/( const T& val ) const
{
   return Quat<T>( _vec / val, _scalar / val );
}


//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Quat<T>::operator*( const Vec3<T>& vec ) const
{
   // calculate coefficients
   T x2 = _vec(0) + _vec(0);
   T y2 = _vec(1) + _vec(1);
   T z2 = _vec(2) + _vec(2);
   T xx = _vec(0) * x2;
   T xy = _vec(0) * y2;
   T xz = _vec(0) * z2;
   T yy = _vec(1) * y2;
   T yz = _vec(1) * z2;
   T zz = _vec(2) * z2;
   T wx = _scalar * x2;
   T wy = _scalar * y2;
   T wz = _scalar * z2;

   return Vec3<T>(
      vec(0)*((T)1 - ( yy + zz )) + vec(1)*(xy - wz) + vec(2)*(xz + wy),
      vec(0)*(xy + wz) + vec(1)*((T)1 - ( xx + zz )) + vec(2)*(yz - wx),
      vec(0)*(xz - wy) + vec(1)*(yz + wx) + vec(2)*((T)1 - ( xx + yy ))
   );
}


//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>&
Quat<T>::operator+=( const Quat<T>& quat )
{
   _vec    += quat._vec;
   _scalar += quat._scalar;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>&
Quat<T>::operator-=( const Quat<T>& quat )
{
   _vec    -= quat._vec;
   _scalar -= quat._scalar;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>&
Quat<T>::operator*=( const Quat<T>& quat )
{
   T v0 = (_vec(2) - _vec(1)) * (quat._vec(1) - quat._vec(2));
   T v1 = (_scalar + _vec(0)) * (quat._scalar + quat._vec(0));
   T v2 = (_scalar - _vec(0)) * (quat._vec(1) + quat._vec(2));
   T v3 = (_vec(2) + _vec(1)) * (quat._scalar - quat._vec(0));
   T v4 = (_vec(2) - _vec(0)) * (quat._vec(0) - quat._vec(1));
   T v5 = (_vec(2) + _vec(0)) * (quat._vec(0) + quat._vec(1));
   T v6 = (_scalar + _vec(1)) * (quat._scalar - quat._vec(2));
   T v7 = (_scalar - _vec(1)) * (quat._scalar + quat._vec(2));
   T v8 = v5 + v6 + v7;
   T v9 = ( v4 + v8 ) / (T)2;

   _vec(0) = v1 + v9 - v8;
   _vec(1) = v2 + v9 - v7;
   _vec(2) = v3 + v9 - v6;
   _scalar = v0 + v9 - v5;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>&
Quat<T>::operator*=( const T& val )
{
   _vec    *= val;
   _scalar *= val;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>&
Quat<T>::operator/=( const T& val )
{
   _vec    /= val;
   _scalar /= val;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Quat<T>&
Quat<T>::operator=( const Quat<T>& quat )
{
   _vec    = quat._vec;
   _scalar = quat._scalar;
   return *this;
}



//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Quat<T>::operator==( const Quat<T>& rhs ) const
{
   return _vec == rhs._vec && _scalar == rhs._scalar;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Quat<T>::operator!=( const Quat<T>& rhs ) const
{
   return _vec != rhs._vec && _scalar != rhs._scalar;
}



//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Quat<T>::operator()( int idx )
{
   if( idx == 3 )
   {
      return _scalar;
   }
   return _vec(idx);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T&
Quat<T>::operator()( int idx ) const
{
   if( idx == 3 )
   {
      return _scalar;
   }
   return _vec(idx);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<( TextStream& stream, const Quat<T>& quat )
{
   return stream << "("
                 << quat(0) << ","
                 << quat(1) << ","
                 << quat(2) << ","
                 << quat(3) << ")";
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator>>( TextStream& stream, Quat<T>& quat )
{
   char c;
   return stream >> c
                 >> quat(0) >> c
                 >> quat(1) >> c
                 >> quat(2) >> c
                 >> quat(3) >> c;
}

namespace CGM {

//------------------------------------------------------------------------------
//!
template< typename T >
inline bool equal( const Quat<T>& a, const Quat<T>& b, const T threshold = (T)EqualityThreshold )
{
   return ( CGM::equal(a.vec(),  b.vec(), threshold) && CGM::equal(a.w(),  b.w(), threshold) ) ||
          ( CGM::equal(a.vec(), -b.vec(), threshold) && CGM::equal(a.w(), -b.w(), threshold) );
}

} // namespace CGM

/*==============================================================================
  TYPEDEF
  ==============================================================================*/

typedef Quat< float >  Quatf;
typedef Quat< double > Quatd;

NAMESPACE_END

#endif //CGMATH_QUAT_H
