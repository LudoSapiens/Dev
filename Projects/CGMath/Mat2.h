/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_MAT2_H
#define CGMATH_MAT2_H

#include <CGMath/Vec2.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Mat2
==============================================================================*/

//! 2D Matrix class.
//!
//! The matrix is define in column major but the constructor
//! and the accessor use a row major definition.
//!
//! internal: ( 0 2 )  external: ( 0 1 )
//!           ( 1 3 )            ( 2 3 )

template< typename T >
class Mat2
{

public:

   /*----- static methods -----*/

   static Mat2 identity();
   static Mat2 zero();
   static Mat2 rotation( const T& angle );
   static Mat2 rotation( const T& cos, const T& sin );
   static Mat2 scaling( const T& sx, const T& sy );
   static Mat2 scaling( const Vec2<T>& s );

   /*----- methods -----*/

   template< typename S > Mat2( const Mat2<S>& m )
   {
      _e[0] = (T)m(0);
      _e[1] = (T)m(1);
      _e[2] = (T)m(2);
      _e[3] = (T)m(3);
   }

   Mat2(){}
   Mat2( const Mat2<T>& m );
   Mat2( const T& e00, const T& e01,
         const T& e10, const T& e11 );
   ~Mat2(){}

   T* ptr();
   const T* ptr() const;

   Mat2& inverse();
   Mat2& transpose();

   Mat2  getInversed() const;
   Mat2  getTransposed() const;

   Mat2 operator+( const Mat2<T>& m ) const;
   Mat2 operator-( const Mat2<T>& m ) const;
   Mat2 operator-() const;
   Mat2 operator*( const T& val ) const;
   Mat2 operator*( const Mat2<T>& m ) const;
   Mat2 operator/( const T& val ) const;
   Mat2 operator/( const Mat2<T>& m ) const;

   Vec2<T> operator*( const Vec2<T>& vec ) const;

   Mat2& operator+=( const Mat2<T>& m );
   Mat2& operator-=( const Mat2<T>& m );
   Mat2& operator*=( const T& val );
   Mat2& operator*=( const Mat2<T>& m );
   Mat2& operator/=( const T& val );
   Mat2& operator/=( const Mat2<T>& m );
   Mat2& operator=( const Mat2<T>& m );
   
   bool operator==( const Mat2<T>& m ) const;
   bool operator!=( const Mat2<T>& m ) const;

   T& operator()( int pos );
   const T& operator()( int pos ) const;
   T& operator()( int line, int col );
   const T& operator()( int line, int col ) const;

   const Vec2<T>& col( int idx ) const;
         Vec2<T>  row( int idx ) const;

   void col( int idx, const Vec2<T>& v );
   void row( int idx, const Vec2<T>& v );

private:

   /*----- data members -----*/

   union {
      struct {
         T _00;
         T _10;
         T _01;
         T _11;
      };
      T _e[4];
   };
};

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::identity()
{
   return Mat2<T>(
      T(1), T(0),
      T(0), T(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::zero()
{
   return Mat2<T>(
      T(0), T(0),
      T(0), T(0)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::rotation( const T& rad )
{
   return rotation( CGM::cos( rad ), CGM::sin( rad ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::rotation( const T& cos, const T& sin )
{
   return Mat2<T>(
      cos, -sin,
      sin,  cos
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::scaling( const T& sx, const T& sy )
{
   return Mat2<T>(
      sx,   T(0),
      T(0),   sy
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::scaling( const Vec2<T>& s )
{
   return Mat2<T>(
      s.x,  T(0),
      T(0),  s.y
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Mat2<T>::Mat2( const Mat2<T>& m )
{
   *this = m;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Mat2<T>::Mat2( const T& e00, const T& e01, const T& e10, const T& e11 )
{
   _e[0] = e00;
   _e[1] = e10;
   _e[2] = e01;
   _e[3] = e11;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T*
Mat2<T>::ptr()
{
   return _e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T*
Mat2<T>::ptr() const
{
   return _e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>&
Mat2<T>::inverse()
{
   double det = _e[0] * _e[3] - _e[2] * _e[1];

   if( det < 1e-12 ) return Mat2<T>();

   double idet = 1.0/det;

   T e0 = _e[0];

   _e[0] =  idet * _e[3];
   _e[1] = -idet * _e[1];
   _e[2] = -idet * _e[2];
   _e[3] =  idet * e0;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>&
Mat2<T>::transpose()
{
   CGM::swap( _e[1], _e[2] );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::getInversed() const
{
   double det = _e[0] * _e[3] - _e[2] * _e[1];

   if( det < 1e-12 ) return Mat2<T>();

   double idet = 1.0/det;

   return Mat2<T>(
      T( idet * _e[3]), T(-idet * _e[2]),
      T(-idet * _e[1]), T( idet * _e[0])
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::getTransposed() const
{
   return Mat2<T>(
      _e[0], _e[1],
      _e[2], _e[3]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::operator+( const Mat2<T>& m ) const
{
   return Mat2<T>(
      _e[0] + m._e[0], _e[2] + m._e[2],
      _e[1] + m._e[1], _e[3] + m._e[3]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::operator-( const Mat2<T>& m ) const
{
   return Mat2<T>(
      _e[0] - m._e[0], _e[2] - m._e[2],
      _e[1] - m._e[1], _e[3] - m._e[3]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::operator-() const
{
   return Mat2<T>(
      -_e[0], -_e[2],
      -_e[1], -_e[3]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::operator*( const T& val ) const
{
   return Mat2<T>(
      _e[0] * val, _e[2] * val,
      _e[1] * val, _e[3] * val
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::operator*( const Mat2<T>& m ) const
{
   return Mat2<T>(
      _e[0]*m._e[0] + _e[2]*m._e[1],
      _e[0]*m._e[2] + _e[2]*m._e[3],
      _e[1]*m._e[0] + _e[3]*m._e[1],
      _e[1]*m._e[2] + _e[3]*m._e[3]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::operator/( const T& val ) const
{
   T ival = (T)1 / val;
   return Mat2<T>(
      _e[0] * ival, _e[2] * ival,
      _e[1] * ival, _e[3] * ival
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
Mat2<T>::operator/( const Mat2<T>& m ) const
{
   return Mat2<T>(
      _e[0] / m._e[0], _e[2] / m._e[2],
      _e[1] / m._e[1], _e[3] / m._e[3]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Mat2<T>::operator*( const Vec2<T>& vec ) const
{
   return Vec2<T>(
      _e[0] * vec(0) + _e[2] * vec(1),
      _e[1] * vec(0) + _e[3] * vec(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>&
Mat2<T>::operator+=( const Mat2<T>& m )
{
   _e[0] += m._e[0];
   _e[1] += m._e[1];
   _e[2] += m._e[2];
   _e[3] += m._e[3];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>&
Mat2<T>::operator-=( const Mat2<T>& m )
{
   _e[0] -= m._e[0];
   _e[1] -= m._e[1];
   _e[2] -= m._e[2];
   _e[3] -= m._e[3];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>&
Mat2<T>::operator*=( const T& val )
{
   _e[0] *= val;
   _e[1] *= val;
   _e[2] *= val;
   _e[3] *= val;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>&
Mat2<T>::operator*=( const Mat2<T>& m )
{
   T e0 = _e[0];
   T e1 = _e[1];

   _e[0] = e0*m._e[0] + _e[2]*m._e[1];
   _e[1] = e1*m._e[0] + _e[3]*m._e[1];
   _e[2] = e0*m._e[2] + _e[2]*m._e[3];
   _e[3] = e1*m._e[2] + _e[3]*m._e[3];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>&
Mat2<T>::operator/=( const T& val )
{
   T ival = (T)1 / val;
   _e[0] *= ival;
   _e[1] *= ival;
   _e[2] *= ival;
   _e[3] *= ival;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>&
Mat2<T>::operator/=( const Mat2<T>& m )
{
   _e[0] /= m._e[0];
   _e[1] /= m._e[1];
   _e[2] /= m._e[2];
   _e[3] /= m._e[3];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>&
Mat2<T>::operator=( const Mat2<T>& m )
{
   _e[0] = m._e[0];
   _e[1] = m._e[1];
   _e[2] = m._e[2];
   _e[3] = m._e[3];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Mat2<T>::operator==( const Mat2<T>& m ) const
{
   return _e[0] == m._e[0] && _e[1] == m._e[1] &&
          _e[2] == m._e[2] && _e[3] == m._e[3];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Mat2<T>::operator!=( const Mat2<T>& m ) const
{
   return !( *this == m );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Mat2<T>::operator()( int pos )
{
   return _e[pos];
}

//------------------------------------------------------------------------------
//! 
template< typename T > const T&
Mat2<T>::operator()( int pos ) const
{
   return _e[pos];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Mat2<T>::operator()( int line, int col )
{
   return _e[ line + (col<<1) ];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T&
Mat2<T>::operator()( int line, int col ) const
{
   return _e[ line + (col<<1) ];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec2<T>&
Mat2<T>::col( int idx ) const
{
   return *(const Vec2f*)(_e + 2*idx);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>
Mat2<T>::row( int idx ) const
{
   return Vec2<T>( _e[idx], _e[idx+2] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Mat2<T>::col( int idx, const Vec2<T>& v )
{
   _e[2*idx  ] = v.x;
   _e[2*idx+1] = v.y;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Mat2<T>::row( int idx, const Vec2<T>& v )
{
   _e[idx  ] = v.x;
   _e[idx+2] = v.y;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat2<T>
operator*( const T& val, const Mat2<T>& m )
{
   return Mat2<T>(
      m(0,0) * val, m(0,1) * val,
      m(1,0) * val, m(1,1) * val
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<( TextStream& stream, const Mat2<T>& m )
{
   return stream << "[[" << m(0,0) << "," << m(0,1)
                 << "][" << m(1,0) << "," << m(1,1) << "]]";
}

/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef Mat2< int >    Mat2i;
typedef Mat2< float >  Mat2f;
typedef Mat2< double > Mat2d;

NAMESPACE_END

#endif
