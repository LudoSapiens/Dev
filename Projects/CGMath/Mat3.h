/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_MAT3_H
#define CGMATH_MAT3_H

#include <CGMath/Vec3.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Mat3
==============================================================================*/

//! 3D Matrix class.
//!
//! The matrix is define in column major but the constructor
//! and the accessor use a row major definition.
//!
//! internal: ( 0 3 6 )  external: ( 0 1 2 )
//!           ( 1 4 7 )            ( 3 4 5 )
//!           ( 2 5 8 )            ( 6 7 8 )

template< typename T >
class Mat3
{

public:

   /*----- static methods -----*/

   static Mat3 identity();
   static Mat3 zero();
   static Mat3 rotationX( const T& angle ); // pitch
   static Mat3 rotationY( const T& angle ); // heading
   static Mat3 rotationZ( const T& angle ); // roll
   static Mat3 rotationX( const T& cos, const T& sin );
   static Mat3 rotationY( const T& cos, const T& sin );
   static Mat3 rotationZ( const T& cos, const T& sin );
   static Mat3 scaling( const T& sx, const T& sy, const T& sz );
   static Mat3 scaling( const Vec3<T>& s );

   /*----- methods -----*/

   template< typename S > Mat3( const Mat3<S>& m )
   {
      _e[0] = (T)m(0);
      _e[1] = (T)m(1);
      _e[2] = (T)m(2);
      _e[3] = (T)m(3);
      _e[4] = (T)m(4);
      _e[5] = (T)m(5);
      _e[6] = (T)m(6);
      _e[7] = (T)m(7);
      _e[8] = (T)m(8);
   }

   Mat3(){}
   Mat3( const Mat3<T>& m );
   Mat3( const T& e00, const T& e01, const T& e02,
         const T& e10, const T& e11, const T& e12,
         const T& e20, const T& e21, const T& e22
   );
   Mat3( const Vec3<T>& l0, const Vec3<T>& l1, const Vec3<T>& l2 );

   ~Mat3(){}

   T* ptr();
   const T* ptr() const;

   T determinant() const;

   Mat3& inverse();
   Mat3& inverse( const T& det );
   bool  checkInverse();
   Mat3& transpose();

   Mat3  getInversed() const;
   Mat3  getInversed( const T& det ) const;
   Mat3  getTransposed() const;

   Mat3 operator+( const Mat3<T>& m ) const;
   Mat3 operator-( const Mat3<T>& m ) const;
   Mat3 operator-() const;
   Mat3 operator*( const T& val ) const;
   Mat3 operator*( const Mat3<T>& m ) const;
   Mat3 operator/( const T& val ) const;
   Mat3 operator/( const Mat3<T>& m ) const;

   Vec2<T> operator*( const Vec2<T>& vec ) const;
   Vec2<T> operator^( const Vec2<T>& vec ) const;
   Vec3<T> operator*( const Vec3<T>& vec ) const;

   Mat3& operator+=( const Mat3<T>& m );
   Mat3& operator-=( const Mat3<T>& m );
   Mat3& operator*=( const T& val );
   Mat3& operator*=( const Mat3<T>& m );
   Mat3& operator/=( const T& val );
   Mat3& operator/=( const Mat3<T>& m );
   Mat3& operator=( const Mat3<T>& m );

   bool operator==( const Mat3<T>& m ) const;
   bool operator!=( const Mat3<T>& m ) const;

   T& operator()( int pos );
   const T& operator()( int pos ) const;
   T& operator()( int line, int col );
   const T& operator()( int line, int col ) const;

   const Vec3<T>& col( int idx ) const;
         Vec3<T>  row( int idx ) const;

   void col( int idx, const Vec3<T>& v );
   void row( int idx, const Vec3<T>& v );

private:

   /*----- data members -----*/

   union {
      struct {
         T _00;
         T _10;
         T _20;
         T _01;
         T _11;
         T _21;
         T _02;
         T _12;
         T _22;
      };
      T _e[9];
   };
};


//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::identity()
{
   return Mat3<T>(
      T(1), T(0), T(0),
      T(0), T(1), T(0),
      T(0), T(0), T(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::zero()
{
   return Mat3<T>(
      T(0), T(0), T(0),
      T(0), T(0), T(0),
      T(0), T(0), T(0)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::rotationX( const T& angle )
{
   return rotationX( CGM::cos( angle ), CGM::sin( angle ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::rotationY( const T& angle )
{
   return rotationY( CGM::cos( angle ), CGM::sin( angle ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::rotationZ( const T& angle )
{
   return rotationZ( CGM::cos( angle ), CGM::sin( angle ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::rotationX( const T& cos, const T& sin )
{
   return Mat3<T>(
      T(1), T(0), T(0),
      T(0),  cos, -sin,
      T(0),  sin,  cos
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::rotationY( const T& cos, const T& sin )
{
   return Mat3<T>(
       cos, T(0),  sin,
      T(0), T(1), T(0),
      -sin, T(0),  cos
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::rotationZ( const T& cos, const T& sin )
{
   return Mat3<T>(
      cos,  -sin, T(0),
      sin,   cos, T(0),
      T(0), T(0), T(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::scaling( const T& sx, const T& sy, const T& sz )
{
   return Mat3<T>(
      sx,   T(0), T(0),
      T(0),   sy, T(0),
      T(0), T(0),   sz
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::scaling( const Vec3<T>& s )
{
   return Mat3<T>(
      s.x,  T(0), T(0),
      T(0),  s.y, T(0),
      T(0), T(0),  s.z
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Mat3<T>::Mat3( const Mat3<T>& m )
{
   *this = m;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Mat3<T>::Mat3(
   const T& e00, const T& e01, const T& e02,
   const T& e10, const T& e11, const T& e12,
   const T& e20, const T& e21, const T& e22
)
{
   _e[0] = e00;
   _e[1] = e10;
   _e[2] = e20;
   _e[3] = e01;
   _e[4] = e11;
   _e[5] = e21;
   _e[6] = e02;
   _e[7] = e12;
   _e[8] = e22;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Mat3<T>::Mat3( const Vec3<T>& l0, const Vec3<T>& l1, const Vec3<T>& l2 )
{
   _e[0] = l0.x;
   _e[1] = l1.x;
   _e[2] = l2.x;
   _e[3] = l0.y;
   _e[4] = l1.y;
   _e[5] = l2.y;
   _e[6] = l0.z;
   _e[7] = l1.z;
   _e[8] = l2.z;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T*
Mat3<T>::ptr()
{
   return _e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T*
Mat3<T>::ptr() const
{
   return _e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Mat3<T>::determinant() const
{
   return  _e[0] * ( _e[4] * _e[8] - _e[7] * _e[5] ) +
           _e[3] * ( _e[7] * _e[2] - _e[1] * _e[8] ) +
           _e[6] * ( _e[1] * _e[5] - _e[4] * _e[2] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>&
Mat3<T>::inverse()
{
   return inverse( determinant() );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>&
Mat3<T>::inverse( const T& det )
{
   if( CGM::abs( det ) < 1e-12 ) return *this;

   T idet = (T)(1.0/det);

   T e0 = _e[0];
   T e1 = _e[1];
   T e2 = _e[2];
   T e3 = _e[3];
   T e4 = _e[4];
   T e5 = _e[5];
   T e6 = _e[6];

   _e[0] = idet*(  e4 * _e[8] - _e[7] * e5 );
   _e[1] = -idet*( e1 * _e[8] - _e[7] * e2 );
   _e[2] = idet*(  e1 * e5    - e4    * e2 );
   _e[3] = -idet*( e3 * _e[8] - e6    * e5 );
   _e[4] = idet*(  e0 * _e[8] - e6    * e2 );
   _e[5] = -idet*( e0 * e5    - e3    * e2 );
   _e[6] = idet*(  e3 * _e[7] - e6    * e4 );
   _e[7] = -idet*( e0 * _e[7] - e6    * e1 );
   _e[8] = idet*(  e0 * e4    - e3    * e1 );

   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Mat3<T>::checkInverse()
{
   T det = determinant();
   if( CGM::abs( det ) < 1e-12 ) return false;

   T idet = (T)(1.0/det);

   T e0 = _e[0];
   T e1 = _e[1];
   T e2 = _e[2];
   T e3 = _e[3];
   T e4 = _e[4];
   T e5 = _e[5];
   T e6 = _e[6];

   _e[0] = idet*(  e4 * _e[8] - _e[7] * e5 );
   _e[1] = -idet*( e1 * _e[8] - _e[7] * e2 );
   _e[2] = idet*(  e1 * e5    - e4    * e2 );
   _e[3] = -idet*( e3 * _e[8] - e6    * e5 );
   _e[4] = idet*(  e0 * _e[8] - e6    * e2 );
   _e[5] = -idet*( e0 * e5    - e3    * e2 );
   _e[6] = idet*(  e3 * _e[7] - e6    * e4 );
   _e[7] = -idet*( e0 * _e[7] - e6    * e1 );
   _e[8] = idet*(  e0 * e4    - e3    * e1 );

   return true;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>&
Mat3<T>::transpose()
{
   CGM::swap( _e[1], _e[3] );
   CGM::swap( _e[2], _e[6] );
   CGM::swap( _e[5], _e[7] );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::getInversed() const
{
   return getInversed( determinant() );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::getInversed( const T& det ) const
{
   if( CGM::abs( det ) < 1e-12 ) return Mat3<T>();

   T idet = (T)(1.0/det);

   return Mat3<T>(
      idet*(  _e[4] * _e[8] - _e[7] * _e[5] ),
      -idet*( _e[3] * _e[8] - _e[6] * _e[5] ),
      idet*(  _e[3] * _e[7] - _e[6] * _e[4] ),

      -idet*( _e[1] * _e[8] - _e[7] * _e[2] ),
      idet*(  _e[0] * _e[8] - _e[6] * _e[2] ),
      -idet*( _e[0] * _e[7] - _e[6] * _e[1] ),

      idet*(  _e[1] * _e[5] - _e[4] * _e[2] ),
      -idet*( _e[0] * _e[5] - _e[3] * _e[2] ),
      idet*(  _e[0] * _e[4] - _e[3] * _e[1] )
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::getTransposed() const
{
   return Mat3<T>(
      _e[0], _e[1], _e[2],
      _e[3], _e[4], _e[5],
      _e[6], _e[7], _e[8]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::operator+( const Mat3<T>& m ) const
{
   return Mat3<T>(
      _e[0] + m._e[0],
      _e[3] + m._e[3],
      _e[6] + m._e[6],

      _e[1] + m._e[1],
      _e[4] + m._e[4],
      _e[7] + m._e[7],

      _e[2] + m._e[2],
      _e[5] + m._e[5],
      _e[8] + m._e[8]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::operator-( const Mat3<T>& m ) const
{
   return Mat3<T>(
      _e[0] - m._e[0],
      _e[3] - m._e[3],
      _e[6] - m._e[6],

      _e[1] - m._e[1],
      _e[4] - m._e[4],
      _e[7] - m._e[7],

      _e[2] - m._e[2],
      _e[5] - m._e[5],
      _e[8] - m._e[8]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::operator-() const
{
   return Mat3<T>(
      -_e[0], -_e[3], -_e[6],
      -_e[1], -_e[4], -_e[7],
      -_e[2], -_e[5], -_e[8]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::operator*( const T& val ) const
{
   return Mat3<T>(
      _e[0] * val,
      _e[3] * val,
      _e[6] * val,

      _e[1] * val,
      _e[4] * val,
      _e[7] * val,

      _e[2] * val,
      _e[5] * val,
      _e[8] * val
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::operator*( const Mat3<T>& m ) const
{
   return Mat3<T>(
      _e[0]*m._e[0] + _e[3]*m._e[1] + _e[6]*m._e[2],
      _e[0]*m._e[3] + _e[3]*m._e[4] + _e[6]*m._e[5],
      _e[0]*m._e[6] + _e[3]*m._e[7] + _e[6]*m._e[8],

      _e[1]*m._e[0] + _e[4]*m._e[1] + _e[7]*m._e[2],
      _e[1]*m._e[3] + _e[4]*m._e[4] + _e[7]*m._e[5],
      _e[1]*m._e[6] + _e[4]*m._e[7] + _e[7]*m._e[8],

      _e[2]*m._e[0] + _e[5]*m._e[1] + _e[8]*m._e[2],
      _e[2]*m._e[3] + _e[5]*m._e[4] + _e[8]*m._e[5],
      _e[2]*m._e[6] + _e[5]*m._e[7] + _e[8]*m._e[8]
  );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::operator/( const T& val ) const
{
   T ival = (T)1 / val;
   return Mat3<T>(
      _e[0] * ival,
      _e[3] * ival,
      _e[6] * ival,

      _e[1] * ival,
      _e[4] * ival,
      _e[7] * ival,

      _e[2] * ival,
      _e[5] * ival,
      _e[8] * ival
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
Mat3<T>::operator/( const Mat3<T>& m ) const
{
   return Mat3<T>(
      _e[0] / m._e[0],
      _e[3] / m._e[3],
      _e[6] / m._e[6],

      _e[1] / m._e[1],
      _e[4] / m._e[4],
      _e[7] / m._e[7],

      _e[2] / m._e[2],
      _e[5] / m._e[5],
      _e[8] / m._e[8]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Mat3<T>::operator*( const Vec2<T>& vec ) const
{
   return Vec2<T>(
      _e[0]*vec(0) + _e[3]*vec(1) + _e[6],
      _e[1]*vec(0) + _e[4]*vec(1) + _e[7]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Mat3<T>::operator^( const Vec2<T>& vec ) const
{
   return Vec2<T>(
      _e[0]*vec(0) + _e[3]*vec(1),
      _e[1]*vec(0) + _e[4]*vec(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Mat3<T>::operator*( const Vec3<T>& vec ) const
{
   return Vec3<T>(
      _e[0]*vec(0) + _e[3]*vec(1) + _e[6]*vec(2),
      _e[1]*vec(0) + _e[4]*vec(1) + _e[7]*vec(2),
      _e[2]*vec(0) + _e[5]*vec(1) + _e[8]*vec(2)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>&
Mat3<T>::operator+=( const Mat3<T>& m )
{
   _e[0] += m._e[0];
   _e[1] += m._e[1];
   _e[2] += m._e[2];
   _e[3] += m._e[3];
   _e[4] += m._e[4];
   _e[5] += m._e[5];
   _e[6] += m._e[6];
   _e[7] += m._e[7];
   _e[8] += m._e[8];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>&
Mat3<T>::operator-=( const Mat3<T>& m )
{
  _e[0] -= m._e[0];
  _e[1] -= m._e[1];
  _e[2] -= m._e[2];
  _e[3] -= m._e[3];
  _e[4] -= m._e[4];
  _e[5] -= m._e[5];
  _e[6] -= m._e[6];
  _e[7] -= m._e[7];
  _e[8] -= m._e[8];
  return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>&
Mat3<T>::operator*=( const T& val )
{
   _e[0] *= val;
   _e[1] *= val;
   _e[2] *= val;
   _e[3] *= val;
   _e[4] *= val;
   _e[5] *= val;
   _e[6] *= val;
   _e[7] *= val;
   _e[8] *= val;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>&
Mat3<T>::operator*=( const Mat3<T>& m )
{
   T e0 = _e[0];
   T e1 = _e[1];
   T e2 = _e[2];
   T e3 = _e[3];
   T e4 = _e[4];
   T e5 = _e[5];

   _e[0] = e0*m._e[0] + e3*m._e[1] + _e[6]*m._e[2];
   _e[1] = e1*m._e[0] + e4*m._e[1] + _e[7]*m._e[2];
   _e[2] = e2*m._e[0] + e5*m._e[1] + _e[8]*m._e[2];

   _e[3] = e0*m._e[3] + e3*m._e[4] + _e[6]*m._e[5];
   _e[4] = e1*m._e[3] + e4*m._e[4] + _e[7]*m._e[5];
   _e[5] = e2*m._e[3] + e5*m._e[4] + _e[8]*m._e[5];

   _e[6] = e0*m._e[6] + e3*m._e[7] + _e[6]*m._e[8];
   _e[7] = e1*m._e[6] + e4*m._e[7] + _e[7]*m._e[8];
   _e[8] = e2*m._e[6] + e5*m._e[7] + _e[8]*m._e[8];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>&
Mat3<T>::operator/=( const T& val )
{
   T ival = (T)1 / val;

   _e[0] *= ival;
   _e[1] *= ival;
   _e[2] *= ival;
   _e[3] *= ival;
   _e[4] *= ival;
   _e[5] *= ival;
   _e[6] *= ival;
   _e[7] *= ival;
   _e[8] *= ival;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>&
Mat3<T>::operator/=( const Mat3<T>& m )
{
   _e[0] /= m._e[0];
   _e[1] /= m._e[1];
   _e[2] /= m._e[2];
   _e[3] /= m._e[3];
   _e[4] /= m._e[4];
   _e[5] /= m._e[5];
   _e[6] /= m._e[6];
   _e[7] /= m._e[7];
   _e[8] /= m._e[8];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>&
Mat3<T>::operator=( const Mat3<T>& m )
{
   _e[0] = m._e[0];
   _e[1] = m._e[1];
   _e[2] = m._e[2];
   _e[3] = m._e[3];
   _e[4] = m._e[4];
   _e[5] = m._e[5];
   _e[6] = m._e[6];
   _e[7] = m._e[7];
   _e[8] = m._e[8];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Mat3<T>::operator==( const Mat3<T>& m ) const
{
   return _e[0] == m._e[0] && _e[1] == m._e[1] && _e[2] == m._e[2] &&
          _e[3] == m._e[3] && _e[4] == m._e[4] && _e[5] == m._e[5] &&
          _e[6] == m._e[6] && _e[7] == m._e[7] && _e[8] == m._e[8];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Mat3<T>::operator!=( const Mat3<T>& m ) const
{
   return !( *this == m );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Mat3<T>::operator()( int pos )
{
   return _e[pos];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T&
Mat3<T>::operator()( int pos ) const
{
   return _e[pos];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Mat3<T>::operator()( int line, int col )
{
   return _e[ line + (col*3) ];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T&
Mat3<T>::operator()( int line, int col ) const
{
   return _e[ line + (col*3) ];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec3<T>&
Mat3<T>::col( int idx ) const
{
   return *(const Vec3f*)(_e + 3*idx);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Mat3<T>::row( int idx ) const
{
   return Vec3<T>( _e[idx], _e[idx+3], _e[idx+6] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Mat3<T>::col( int idx, const Vec3<T>& v )
{
   _e[3*idx  ] = v.x;
   _e[3*idx+1] = v.y;
   _e[3*idx+2] = v.z;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Mat3<T>::row( int idx, const Vec3<T>& v )
{
   _e[idx  ] = v.x;
   _e[idx+3] = v.y;
   _e[idx+6] = v.z;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat3<T>
operator*( const T& val, const Mat3<T>& m )
{
   return Mat3<T>(
      m(0,0) * val,
      m(0,1) * val,
      m(0,2) * val,

      m(1,0) * val,
      m(1,1) * val,
      m(1,2) * val,

      m(2,0) * val,
      m(2,1) * val,
      m(2,2) * val
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<( TextStream& stream, const Mat3<T>& m )
{
   return stream << "[[" << m(0,0) << "," << m(0,1) << "," << m(0,2)
                 << "][" << m(1,0) << "," << m(1,1) << "," << m(1,2)
                 << "][" << m(2,0) << "," << m(2,1) << "," << m(2,2)
                 << "]]";
}

/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef Mat3< int >    Mat3i;
typedef Mat3< float >  Mat3f;
typedef Mat3< double > Mat3d;

NAMESPACE_END

#endif
