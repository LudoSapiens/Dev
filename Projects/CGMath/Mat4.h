/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_MAT4_H
#define CGMATH_MAT4_H

#include <CGMath/Mat3.h>
#include <CGMath/Vec4.h>
#include <CGMath/Ray.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Mat4
==============================================================================*/

//! 4D Matrix class.
//!
//!  The matrix is define in column major but the constructor
//!  and the accessor use a row major definition.
//!
//!   internal: ( 0 4  8 12 )  external: (  0  1  2  3 )
//!             ( 1 5  9 13 )            (  4  5  6  7 )
//!             ( 2 6 10 14 )            (  8  9 10 11 )
//!             ( 3 7 11 15 )            ( 12 13 14 15 )
//!

template< typename T >
class Mat4
{

public:

   /*----- static methods -----*/

   static Mat4 identity();
   static Mat4 zero();
   static Mat4 reflection( const Vec3<T>& n, const Vec3<T>& pos );
   static Mat4 rotationX( const T& angle ); // pitch
   static Mat4 rotationY( const T& angle ); // heading
   static Mat4 rotationZ( const T& angle ); // roll
   static Mat4 rotationX( const T& cos, const T& sin );
   static Mat4 rotationY( const T& cos, const T& sin );
   static Mat4 rotationZ( const T& cos, const T& sin );
   static Mat4 translation( const T& tx, const T& ty, const T& tz );
   static Mat4 translation( const Vec3<T>& t );
   static Mat4 scaling( const T& sx, const T& sy, const T& sz );
   static Mat4 scaling( const Vec3<T>& s );
   static Mat4 shearY( const T& tanyx, const T& tanyz );
   static Mat4 fitting( const Vec3<T>& min, const Vec3<T>& max );
   static Mat4 ortho(
      const T& l, const T& r,
      const T& b, const T& t,
      const T& n, const T& f
   );
   static Mat4 orthoScale(
      const T& orthoScale,
      const T& aspect,
      const T& front,
      const T& back,
      const T& shearX = (T)0,
      const T& shearY = (T)0
   );
   static Mat4 perspectiveX(
      const T& fovx,
      const T& aspect,
      const T& front,
      const T& back,
      const T& shearX = (T)0,
      const T& shearY = (T)0
   );
   static Mat4 perspectiveY(
      const T& fovy,
      const T& aspect,
      const T& front,
      const T& back,
      const T& shearX = (T)0,
      const T& shearY = (T)0
   );
   static Mat4 perspectiveSmallest(
      const T& fov,
      const T& yOverX,
      const T& aspect,
      const T& front,
      const T& back,
      const T& shearX = (T)0,
      const T& shearY = (T)0
   );
   static Mat4 perspectiveLargest(
      const T& fov,
      const T& yOverX,
      const T& aspect,
      const T& front,
      const T& back,
      const T& shearX = (T)0,
      const T& shearY = (T)0
   );
   static void perspectiveX_SxSy(
      const T& fov,
      const T& aspect,
      T& sx, T& sy
   );
   static void perspectiveY_SxSy(
      const T& fov,
      const T& aspect,
      T& sx, T& sy
   );
   static void perspectiveSmallest_SxSy(
      const T& fov,
      const T& yOverX,
      const T& aspect,
      T& sx, T& sy
   );
   static void perspectiveLargest_SxSy(
      const T& fov,
      const T& yOverX,
      const T& aspect,
      T& sx, T& sy
   );
   static Mat4 viewport( const Vec2<T>& pos, const Vec2<T>& size );

   /*----- methods -----*/

   template< typename S > Mat4( const Mat4<S>& m )
   {
      _e[0]  = (T)m(0);
      _e[1]  = (T)m(1);
      _e[2]  = (T)m(2);
      _e[3]  = (T)m(3);
      _e[4]  = (T)m(4);
      _e[5]  = (T)m(5);
      _e[6]  = (T)m(6);
      _e[7]  = (T)m(7);
      _e[8]  = (T)m(8);
      _e[9]  = (T)m(9);
      _e[10] = (T)m(10);
      _e[11] = (T)m(11);
      _e[12] = (T)m(12);
      _e[13] = (T)m(13);
      _e[14] = (T)m(14);
      _e[15] = (T)m(15);
   }

   Mat4(){}
   Mat4( const Mat4<T>& m );
   Mat4( const T& e00, const T& e01, const T& e02, const T& e03,
         const T& e10, const T& e11, const T& e12, const T& e13,
         const T& e20, const T& e21, const T& e22, const T& e23,
         const T& e30, const T& e31, const T& e32, const T& e33
   );
   Mat4( const Mat3<T>& m );
   ~Mat4(){}

   T* ptr();
   const T* ptr() const;

   T  determinant() const;

   Mat4& inverse();
   Mat4& inverse( const T& det );
   bool  checkInverse();
   Mat4& translate( const Vec3<T>& vec );
   Mat4& translateBefore( const Vec3<T>& vec );
   Mat4& transpose();
   Mat4  getInversed() const;
   Mat4  getInversed( const T& det ) const;
   Mat4  getTransposed() const;

   Mat4 operator+( const Mat4<T>& m ) const;
   Mat4 operator-( const Mat4<T>& m ) const;
   Mat4 operator-() const;
   Mat4 operator*( const T& val ) const;
   Mat4 operator*( const Mat4<T>& m ) const;
   Mat4 operator/( const T& val ) const;
   Mat4 operator/( const Mat4<T>& m ) const;

   Vec4<T> operator*( const Vec4<T>& vec ) const;
   Vec3<T> operator*( const Vec3<T>& vec ) const;
   Vec3<T> operator^( const Vec3<T>& vec ) const;
   Vec3<T> operator|( const Vec3<T>& vec ) const;
   Vec4<T> operator|( const Vec4<T>& vec ) const;

   Ray<T> operator*( const Ray<T>& ray ) const;

   Mat4& operator+=( const Mat4<T>& m );
   Mat4& operator-=( const Mat4<T>& m );
   Mat4& operator*=( const T& val );
   Mat4& operator*=( const Mat4<T>& m );
   Mat4& operator/=( const T& val );
   Mat4& operator/=( const Mat4<T>& m );
   Mat4& operator=( const Mat4<T>& m );

   bool operator==( const Mat4<T>& m ) const;
   bool operator!=( const Mat4<T>& m ) const;

   T& operator()( int pos );
   const T& operator()( int pos ) const;
   T& operator()( int line, int col );
   const T& operator()( int line, int col ) const;

   const Vec4<T>& col( int idx ) const;
         Vec4<T>  row( int idx ) const;

   const Vec3<T>& col3( int idx ) const;
         Vec3<T>  row3( int idx ) const;


   void col( int idx, const Vec4<T>& v );
   void row( int idx, const Vec4<T>& v );

private:

   /*----- data members -----*/

union {
      struct {
         T _00;
         T _10;
         T _20;
         T _30;
         T _01;
         T _11;
         T _21;
         T _31;
         T _02;
         T _12;
         T _22;
         T _32;
         T _03;
         T _13;
         T _23;
         T _33;
      };
      T _e[16];
   };
};

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::identity()
{
   return Mat4<T>(
      T(1), T(0), T(0), T(0),
      T(0), T(1), T(0), T(0),
      T(0), T(0), T(1), T(0),
      T(0), T(0), T(0), T(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::zero()
{
   return Mat4<T>(
      T(0), T(0), T(0), T(0),
      T(0), T(0), T(0), T(0),
      T(0), T(0), T(0), T(0),
      T(0), T(0), T(0), T(0)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::reflection( const Vec3<T>& n, const Vec3<T>& pos )
{
   T d = ((T)2)*dot(n,pos);
   return Mat4<T>(
      (T)1-((T)2)*n.x*n.x, -((T)2)*n.x*n.y, -((T)2)*n.x*n.z, d*n.x,
      -((T)2)*n.y*n.x, (T)1-((T)2)*n.y*n.y, -((T)2)*n.y*n.z, d*n.y,
      -((T)2)*n.z*n.x, -((T)2)*n.z*n.y, (T)1-((T)2)*n.z*n.z, d*n.z,
      (T)0, (T)0, (T)0, (T)1
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::rotationX( const T& angle )
{
   return rotationX( CGM::cos( angle ), CGM::sin( angle ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::rotationY( const T& angle )
{
   return rotationY( CGM::cos( angle ), CGM::sin( angle ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::rotationZ( const T& angle )
{
   return rotationZ( CGM::cos( angle ), CGM::sin( angle ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::rotationX( const T& cos, const T& sin )
{
   return Mat4<T>(
      T(1), T(0), T(0), T(0),
      T(0),  cos, -sin, T(0),
      T(0),  sin,  cos, T(0),
      T(0), T(0), T(0), T(1)
   );
}


//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::rotationY( const T& cos, const T& sin )
{
   return Mat4<T>(
       cos, T(0),  sin, T(0),
      T(0), T(1), T(0), T(0),
      -sin, T(0),  cos, T(0),
      T(0), T(0), T(0), T(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::rotationZ( const T& cos, const T& sin )
{
   return Mat4<T>(
       cos, -sin, T(0), T(0),
       sin,  cos, T(0), T(0),
      T(0), T(0), T(1), T(0),
      T(0), T(0), T(0), T(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::translation( const T& tx, const T& ty, const T& tz )
{
   return Mat4<T>(
      T(1), T(0), T(0), tx,
      T(0), T(1), T(0), ty,
      T(0), T(0), T(1), tz,
      T(0), T(0), T(0), T(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::translation( const Vec3<T>& t )
{
   return Mat4<T>(
      T(1), T(0), T(0), t.x,
      T(0), T(1), T(0), t.y,
      T(0), T(0), T(1), t.z,
      T(0), T(0), T(0), T(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::scaling( const T& sx, const T& sy, const T& sz )
{
   return Mat4<T>(
      sx,   T(0), T(0), T(0),
      T(0),   sy, T(0), T(0),
      T(0), T(0),   sz, T(0),
      T(0), T(0), T(0), T(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::scaling( const Vec3<T>& s )
{
   return Mat4<T>(
      s.x,  T(0), T(0), T(0),
      T(0),  s.y, T(0), T(0),
      T(0), T(0),  s.z, T(0),
      T(0), T(0), T(0), T(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::shearY( const T& tanyx, const T& tanyz )
{
   return Mat4<T>(
       T(1), T(0),  T(0), T(0),
      tanyx, T(1), tanyz, T(0),
       T(0), T(0),  T(1), T(0),
       T(0), T(0),  T(0), T(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::fitting( const Vec3<T>& min, const Vec3<T>& max )
{
   Vec3<T> d = max-min;
   Vec3<T> s = Vec3<T>(2) / d;
   Vec3<T> t = -(max+min) / d;
   return Mat4<T>(
      s.x,  T(0), T(0), t.x,
      T(0),  s.y, T(0), t.y,
      T(0), T(0),  s.z, t.z,
      T(0), T(0), T(0), T(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::ortho(
   const T& l, const T& r,
   const T& b, const T& t,
   const T& n, const T& f
)
{
   T rl = r-l;
   T tb = t-b;
   T fn = f-n;
   return Mat4<T>(
      (T)2.0/rl, (T)0,      (T)0,       -((r+l)/rl),
      (T)0,      (T)2.0/tb, (T)0,       -((t+b)/tb),
      (T)0,      (T)0,      (T)-2.0/fn, -((f+n)/fn),
      (T)0,      (T)0,      (T)0,       (T)1
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::orthoScale(
   const T& orthoScale,
   const T& aspect,
   const T& front,
   const T& back,
   const T& shearX,
   const T& shearY
)
{
   T bf  = back - front;
   T sy  = (T)1 / orthoScale;
   T sx  = sy * aspect;
   T sz  = (T)-2 / bf;
   T tz  = -( back + front ) / bf;
   T shx = shearX * aspect;
   T shy = shearY;

   return Mat4<T>(
      sx,   (T)0,  shx, (T)0,
      (T)0,   sy,  shy, (T)0,
      (T)0, (T)0,   sz,   tz,
      (T)0, (T)0, (T)0, (T)1
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::perspectiveX(
   const T& fovx,
   const T& aspect,
   const T& front,
   const T& back,
   const T& shearX,
   const T& shearY
)
{
   T bf = back - front;
   T sx  = (T)1 / CGM::tan( CGM::degToRad( fovx * (T)0.5 ) );
   T sy  = sx / aspect;
   T sz  = -( back + front ) / bf;
   T tz  = (T)-2 * back * front / bf;
   T shx = shearX * aspect;
   T shy = shearY;

   return Mat4<T>(
        sx,  (T)0,   shx,  (T)0,
      (T)0,    sy,   shy,  (T)0,
      (T)0,  (T)0,    sz,    tz,
      (T)0,  (T)0, (T)-1,  (T)0
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::perspectiveY(
   const T& fovy,
   const T& aspect,
   const T& front,
   const T& back,
   const T& shearX,
   const T& shearY
)
{
   T bf = back - front;
   T sy  = (T)1 / CGM::tan( CGM::degToRad( fovy * (T)0.5 ) );
   T sx  = sy * aspect;
   T sz  = -( back + front ) / bf;
   T tz  = (T)-2 * back * front / bf;
   T shx = shearX * aspect;
   T shy = shearY;

   return Mat4<T>(
        sx,  (T)0,   shx,  (T)0,
      (T)0,    sy,   shy,  (T)0,
      (T)0,  (T)0,    sz,    tz,
      (T)0,  (T)0, (T)-1,  (T)0
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::perspectiveSmallest(
   const T& fov,
   const T& yOverX,
   const T& aspect,
   const T& front,
   const T& back,
   const T& shearX,
   const T& shearY
)
{
   if( yOverX <= T(1) )  return perspectiveY( fov, aspect*yOverX, front, back, shearX, shearY );
   else                  return perspectiveX( fov, aspect*yOverX, front, back, shearX, shearY );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::perspectiveLargest(
   const T& fov,
   const T& yOverX,
   const T& aspect,
   const T& front,
   const T& back,
   const T& shearX,
   const T& shearY
)
{
   if( yOverX >= T(1) )  return perspectiveY( fov, aspect*yOverX, front, back, shearX, shearY );
   else                  return perspectiveX( fov, aspect*yOverX, front, back, shearX, shearY );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Mat4<T>::perspectiveX_SxSy(
   const T& fovx,
   const T& aspect,
   T& sx, T& sy
)
{
   sx = (T)1 / CGM::tan( CGM::degToRad( fovx * (T)0.5 ) );
   sy = sx / aspect;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Mat4<T>::perspectiveY_SxSy(
   const T& fovy,
   const T& aspect,
   T& sx, T& sy
)
{
   sy = (T)1 / CGM::tan( CGM::degToRad( fovy * (T)0.5 ) );
   sx = sy * aspect;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Mat4<T>::perspectiveSmallest_SxSy(
   const T& fov,
   const T& yOverX,
   const T& aspect,
   T& sx, T& sy
)
{
   if( yOverX <= T(1) )  perspectiveY_SxSy( fov, yOverX*aspect, sx, sy );
   else                  perspectiveX_SxSy( fov, yOverX*aspect, sx, sy );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Mat4<T>::perspectiveLargest_SxSy(
   const T& fov,
   const T& yOverX,
   const T& aspect,
   T& sx, T& sy
)
{
   if( yOverX >= T(1) )  perspectiveY_SxSy( fov, yOverX*aspect, sx, sy );
   else                  perspectiveX_SxSy( fov, yOverX*aspect, sx, sy );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::viewport( const Vec2<T>& pos, const Vec2<T>& size )
{
   Vec2<T> vs   = size / (T)2;
   Vec2<T> vpos = vs + pos;

   return Mat4<T>(
       vs(0), (T)0.0, (T)0.0, vpos(0),
      (T)0.0,  vs(1), (T)0.0, vpos(1),
      (T)0.0, (T)0.0, (T)0.5,  (T)0.5,
      (T)0.0, (T)0.0, (T)0.0,  (T)1.0
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Mat4<T>::Mat4( const Mat4<T>& m )
{
   *this = m;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Mat4<T>::Mat4(
   const T& e00, const T& e01, const T& e02, const T& e03,
   const T& e10, const T& e11, const T& e12, const T& e13,
   const T& e20, const T& e21, const T& e22, const T& e23,
   const T& e30, const T& e31, const T& e32, const T& e33
)
{
   _e[0] = e00;
   _e[1] = e10;
   _e[2] = e20;
   _e[3] = e30;

   _e[4] = e01;
   _e[5] = e11;
   _e[6] = e21;
   _e[7] = e31;

   _e[8] = e02;
   _e[9] = e12;
   _e[10]= e22;
   _e[11]= e32;

   _e[12]= e03;
   _e[13]= e13;
   _e[14]= e23;
   _e[15]= e33;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Mat4<T>::Mat4( const Mat3<T>& m )
{
   _e[0]  = m(0,0);
   _e[1]  = m(1,0);
   _e[2]  = m(2,0);
   _e[3]  = 0;

   _e[4]  = m(0,1);
   _e[5]  = m(1,1);
   _e[6]  = m(2,1);
   _e[7]  = 0;

   _e[8]  = m(0,2);
   _e[9]  = m(1,2);
   _e[10] = m(2,2);
   _e[11] = 0;

   _e[12] = 0;
   _e[13] = 0;
   _e[14] = 0;
   _e[15] = 1;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T*
Mat4<T>::ptr()
{
   return _e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T*
Mat4<T>::ptr() const
{
   return _e;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Mat4<T>::determinant() const
{
   // Cramer's rule.
   T t0  = _e[10]*_e[15] - _e[11]*_e[14];
   T t1  = _e[9]*_e[15]  - _e[11]*_e[13];
   T t2  = _e[9]*_e[14]  - _e[10]*_e[13];
   T t3  = _e[8]*_e[15]  - _e[11]*_e[12];
   T t4  = _e[8]*_e[14]  - _e[10]*_e[12];
   T t5  = _e[8]*_e[13]  - _e[9]*_e[12];
   T t6  = _e[2]*_e[7]   - _e[3]*_e[6];
   T t7  = _e[1]*_e[7]   - _e[3]*_e[5];
   T t8  = _e[1]*_e[6]   - _e[2]*_e[5];
   T t9  = _e[0]*_e[7]   - _e[3]*_e[4];
   T t10 = _e[0]*_e[6]   - _e[2]*_e[4];
   T t11 = _e[0]*_e[5]   - _e[1]*_e[4];

   return t0*t11 - t1*t10 + t2*t9 + t3*t8 - t4*t7 + t5*t6;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>&
Mat4<T>::inverse()
{
   // Cramer's rule.
   T t0  = _e[10]*_e[15] - _e[11]*_e[14];
   T t1  = _e[9]*_e[15]  - _e[11]*_e[13];
   T t2  = _e[9]*_e[14]  - _e[10]*_e[13];
   T t3  = _e[8]*_e[15]  - _e[11]*_e[12];
   T t4  = _e[8]*_e[14]  - _e[10]*_e[12];
   T t5  = _e[8]*_e[13]  - _e[9]*_e[12];
   T t6  = _e[2]*_e[7]   - _e[3]*_e[6];
   T t7  = _e[1]*_e[7]   - _e[3]*_e[5];
   T t8  = _e[1]*_e[6]   - _e[2]*_e[5];
   T t9  = _e[0]*_e[7]   - _e[3]*_e[4];
   T t10 = _e[0]*_e[6]   - _e[2]*_e[4];
   T t11 = _e[0]*_e[5]   - _e[1]*_e[4];

   T det = t0*t11 - t1*t10 + t2*t9 + t3*t8 - t4*t7 + t5*t6;

   if( CGM::abs( det ) < 1e-12 ) return *this;

   T idet = 1.0/det;

   T e0 = _e[0];
   T e8 = _e[8];
   T e9 = _e[9];
   T e10 = _e[10];
   T e12 = _e[12];
   T e13 = _e[13];

   _e[0]  =  _e[5]*t0  - _e[6]*t1   + _e[7]*t2;
   _e[8]  =  _e[4]*t1  - _e[5]*t3   + _e[7]*t5;
   _e[12] = -_e[4]*t2  + _e[5]*t4   - _e[6]*t5;
   _e[4]  = -_e[4]*t0  + _e[6]*t3   - _e[7]*t4;

   _e[5]  =  e0*t0     - _e[2]*t3   + _e[3]*t4;
   _e[9]  = -e0*t1     + _e[1]*t3   - _e[3]*t5;
   _e[13] =  e0*t2     - _e[1]*t4   + _e[2]*t5;
   _e[1]  = -_e[1]*t0  + _e[2]*t1   - _e[3]*t2;

   _e[2]  =  e13*t6    - _e[14]*t7  + _e[15]*t8;
   _e[6]  = -e12*t6    + _e[14]*t9  - _e[15]*t10;
   _e[10] =  e12*t7    - e13*t9     + _e[15]*t11;
   _e[14] = -e12*t8    + e13*t10    - _e[14]*t11;

   _e[3]  = -e9*t6     + e10*t7     - _e[11]*t8;
   _e[7]  =  e8*t6     - e10*t9     + _e[11]*t10;
   _e[11] = -e8*t7     + e9*t9      - _e[11]*t11;
   _e[15] =  e8*t8     - e9*t10     + e10*t11;

   for( int i = 0; i < 16; ++i )
   {
      _e[i] *= idet;
   }

   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Mat4<T>::checkInverse()
{
   // Cramer's rule.
   T t0  = _e[10]*_e[15] - _e[11]*_e[14];
   T t1  = _e[9]*_e[15]  - _e[11]*_e[13];
   T t2  = _e[9]*_e[14]  - _e[10]*_e[13];
   T t3  = _e[8]*_e[15]  - _e[11]*_e[12];
   T t4  = _e[8]*_e[14]  - _e[10]*_e[12];
   T t5  = _e[8]*_e[13]  - _e[9]*_e[12];
   T t6  = _e[2]*_e[7]   - _e[3]*_e[6];
   T t7  = _e[1]*_e[7]   - _e[3]*_e[5];
   T t8  = _e[1]*_e[6]   - _e[2]*_e[5];
   T t9  = _e[0]*_e[7]   - _e[3]*_e[4];
   T t10 = _e[0]*_e[6]   - _e[2]*_e[4];
   T t11 = _e[0]*_e[5]   - _e[1]*_e[4];

   T det = t0*t11 - t1*t10 + t2*t9 + t3*t8 - t4*t7 + t5*t6;

   if( CGM::abs( det ) < 1e-12 )
   {
      return false;
   }

   T idet = 1.0/det;

   T e0 = _e[0];
   T e8 = _e[8];
   T e9 = _e[9];
   T e10 = _e[10];
   T e12 = _e[12];
   T e13 = _e[13];

   _e[0]  =  _e[5]*t0  - _e[6]*t1   + _e[7]*t2;
   _e[8]  =  _e[4]*t1  - _e[5]*t3   + _e[7]*t5;
   _e[12] = -_e[4]*t2  + _e[5]*t4   - _e[6]*t5;
   _e[4]  = -_e[4]*t0  + _e[6]*t3   - _e[7]*t4;

   _e[5]  =  e0*t0     - _e[2]*t3   + _e[3]*t4;
   _e[9]  = -e0*t1     + _e[1]*t3   - _e[3]*t5;
   _e[13] =  e0*t2     - _e[1]*t4   + _e[2]*t5;
   _e[1]  = -_e[1]*t0  + _e[2]*t1   - _e[3]*t2;

   _e[2]  =  e13*t6    - _e[14]*t7  + _e[15]*t8;
   _e[6]  = -e12*t6    + _e[14]*t9  - _e[15]*t10;
   _e[10] =  e12*t7    - e13*t9     + _e[15]*t11;
   _e[14] = -e12*t8    + e13*t10    - _e[14]*t11;

   _e[3]  = -e9*t6     + e10*t7     - _e[11]*t8;
   _e[7]  =  e8*t6     - e10*t9     + _e[11]*t10;
   _e[11] = -e8*t7     + e9*t9      - _e[11]*t11;
   _e[15] =  e8*t8     - e9*t10     + e10*t11;

   for( int i = 0; i < 16; ++i )
   {
      _e[i] *= idet;
   }

   return true;
}

//------------------------------------------------------------------------------
//! Apply  (T) * (this)
//! This operation suppose that the last row of the matrix
//! is ( 0 0 0 1 )
template< typename T > inline Mat4<T>&
Mat4<T>::translate( const Vec3<T>& vec )
{
   _e[12] += vec(0);
   _e[13] += vec(1);
   _e[14] += vec(2);
   return *this;
}

//------------------------------------------------------------------------------
//! Apply (this) * T
//! This operation suppose that the last row of the matrix
//! is ( 0 0 0 1 )
template< typename T > inline Mat4<T>&
Mat4<T>::translateBefore( const Vec3<T>& vec )
{
   _e[12] += _e[0] * vec(0) + _e[4] * vec(1) + _e[8]  * vec(2);
   _e[13] += _e[1] * vec(0) + _e[5] * vec(1) + _e[9]  * vec(2);
   _e[14] += _e[2] * vec(0) + _e[6] * vec(1) + _e[10] * vec(2);
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>&
Mat4<T>::transpose()
{
   CGM::swap( _e[ 1], _e[ 4] );
   CGM::swap( _e[ 2], _e[ 8] );
   CGM::swap( _e[ 3], _e[12] );
   CGM::swap( _e[ 6], _e[ 9] );
   CGM::swap( _e[ 7], _e[13] );
   CGM::swap( _e[11], _e[14] );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::getInversed() const
{
   // Cramer's rule.
   Mat4<T> m;

   T t0  = _e[10]*_e[15] - _e[11]*_e[14];
   T t1  = _e[9]*_e[15]  - _e[11]*_e[13];
   T t2  = _e[9]*_e[14]  - _e[10]*_e[13];
   T t3  = _e[8]*_e[15]  - _e[11]*_e[12];
   T t4  = _e[8]*_e[14]  - _e[10]*_e[12];
   T t5  = _e[8]*_e[13]  - _e[9]*_e[12];
   T t6  = _e[2]*_e[7]   - _e[3]*_e[6];
   T t7  = _e[1]*_e[7]   - _e[3]*_e[5];
   T t8  = _e[1]*_e[6]   - _e[2]*_e[5];
   T t9  = _e[0]*_e[7]   - _e[3]*_e[4];
   T t10 = _e[0]*_e[6]   - _e[2]*_e[4];
   T t11 = _e[0]*_e[5]   - _e[1]*_e[4];

   T det = t0*t11 - t1*t10 + t2*t9 + t3*t8 - t4*t7 + t5*t6;

   if( CGM::abs( det ) < 1e-12 ) return Mat4<T>();

   T idet = (T)1.0/det;

   m._e[0]  =  _e[5]*t0  - _e[6]*t1   + _e[7]*t2;
   m._e[4]  = -_e[4]*t0  + _e[6]*t3   - _e[7]*t4;
   m._e[8]  =  _e[4]*t1  - _e[5]*t3   + _e[7]*t5;
   m._e[12] = -_e[4]*t2  + _e[5]*t4   - _e[6]*t5;

   m._e[1]  = -_e[1]*t0  + _e[2]*t1   - _e[3]*t2;
   m._e[5]  =  _e[0]*t0  - _e[2]*t3   + _e[3]*t4;
   m._e[9]  = -_e[0]*t1  + _e[1]*t3   - _e[3]*t5;
   m._e[13] =  _e[0]*t2  - _e[1]*t4   + _e[2]*t5;

   m._e[2]  =  _e[13]*t6 - _e[14]*t7  + _e[15]*t8;
   m._e[6]  = -_e[12]*t6 + _e[14]*t9  - _e[15]*t10;
   m._e[10] =  _e[12]*t7 - _e[13]*t9  + _e[15]*t11;
   m._e[14] = -_e[12]*t8 + _e[13]*t10 - _e[14]*t11;

   m._e[3]  = -_e[9]*t6  + _e[10]*t7  - _e[11]*t8;
   m._e[7]  =  _e[8]*t6  - _e[10]*t9  + _e[11]*t10;
   m._e[11] = -_e[8]*t7  + _e[9]*t9   - _e[11]*t11;
   m._e[15] =  _e[8]*t8  - _e[9]*t10  + _e[10]*t11;

   for( int i = 0; i < 16; ++i )
   {
      m._e[i] *= idet;
   }

   return m;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::getTransposed() const
{
   return Mat4<T>(
      _e[0],  _e[1],  _e[2],  _e[3],
      _e[4],  _e[5],  _e[6],  _e[7],
      _e[8],  _e[9],  _e[10], _e[11],
      _e[12], _e[13], _e[14], _e[15]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::operator+( const Mat4<T>& m ) const
{
   return Mat4<T>(
      _e[0]  + m._e[0],
      _e[4]  + m._e[4],
      _e[8]  + m._e[8],
      _e[12] + m._e[12],

      _e[1]  + m._e[1],
      _e[5]  + m._e[5],
      _e[9]  + m._e[9],
      _e[13] + m._e[13],

      _e[2]  + m._e[2],
      _e[6]  + m._e[6],
      _e[10] + m._e[10],
      _e[14] + m._e[14],

      _e[3]  + m._e[3],
      _e[7]  + m._e[7],
      _e[11] + m._e[11],
      _e[15] + m._e[15]
  );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::operator-( const Mat4<T>& m ) const
{
   return Mat4<T>(
      _e[0]  - m._e[0],
      _e[4]  - m._e[4],
      _e[8]  - m._e[8],
      _e[12] - m._e[12],

      _e[1]  - m._e[1],
      _e[5]  - m._e[5],
      _e[9]  - m._e[9],
      _e[13] - m._e[13],

      _e[2]  - m._e[2],
      _e[6]  - m._e[6],
      _e[10] - m._e[10],
      _e[14] - m._e[14],

      _e[3]  - m._e[3],
      _e[7]  - m._e[7],
      _e[11] - m._e[11],
      _e[15] - m._e[15]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::operator-() const
{
   return Mat4<T>(
      -_e[0], -_e[4], -_e[8], -_e[12],
      -_e[1], -_e[5], -_e[9], -_e[13],
      -_e[2], -_e[6], -_e[10], -_e[14],
      -_e[3], -_e[7], -_e[11], -_e[15]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::operator*( const T& val ) const
{
   return Mat4<T>(
    _e[0]  * val,
    _e[4]  * val,
    _e[8]  * val,
    _e[12] * val,

    _e[1]  * val,
    _e[5]  * val,
    _e[9]  * val,
    _e[13] * val,

    _e[2]  * val,
    _e[6]  * val,
    _e[10] * val,
    _e[14] * val,

    _e[3]  * val,
    _e[7]  * val,
    _e[11] * val,
    _e[15] * val
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::operator*( const Mat4<T>& m ) const
{
   return Mat4<T>(
      _e[0]*m._e[0] + _e[4]*m._e[1] + _e[8]*m._e[2]  + _e[12]*m._e[3],
      _e[0]*m._e[4] + _e[4]*m._e[5] + _e[8]*m._e[6]  + _e[12]*m._e[7],
      _e[0]*m._e[8] + _e[4]*m._e[9] + _e[8]*m._e[10] + _e[12]*m._e[11],
      _e[0]*m._e[12]+ _e[4]*m._e[13]+ _e[8]*m._e[14] + _e[12]*m._e[15],

      _e[1]*m._e[0] + _e[5]*m._e[1] + _e[9]*m._e[2]  + _e[13]*m._e[3],
      _e[1]*m._e[4] + _e[5]*m._e[5] + _e[9]*m._e[6]  + _e[13]*m._e[7],
      _e[1]*m._e[8] + _e[5]*m._e[9] + _e[9]*m._e[10] + _e[13]*m._e[11],
      _e[1]*m._e[12]+ _e[5]*m._e[13]+ _e[9]*m._e[14] + _e[13]*m._e[15],

      _e[2]*m._e[0] + _e[6]*m._e[1] + _e[10]*m._e[2] + _e[14]*m._e[3],
      _e[2]*m._e[4] + _e[6]*m._e[5] + _e[10]*m._e[6] + _e[14]*m._e[7],
      _e[2]*m._e[8] + _e[6]*m._e[9] + _e[10]*m._e[10]+ _e[14]*m._e[11],
      _e[2]*m._e[12]+ _e[6]*m._e[13]+ _e[10]*m._e[14]+ _e[14]*m._e[15],

      _e[3]*m._e[0] + _e[7]*m._e[1] + _e[11]*m._e[2] + _e[15]*m._e[3],
      _e[3]*m._e[4] + _e[7]*m._e[5] + _e[11]*m._e[6] + _e[15]*m._e[7],
      _e[3]*m._e[8] + _e[7]*m._e[9] + _e[11]*m._e[10]+ _e[15]*m._e[11],
      _e[3]*m._e[12]+ _e[7]*m._e[13]+ _e[11]*m._e[14]+ _e[15]*m._e[15]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::operator/( const T& val ) const
{
   T ival = (T)1 / val;
   return Mat4<T>(
      _e[0]  * ival,
      _e[4]  * ival,
      _e[8]  * ival,
      _e[12] * ival,

      _e[1]  * ival,
      _e[5]  * ival,
      _e[9]  * ival,
      _e[13] * ival,

      _e[2]  * ival,
      _e[6]  * ival,
      _e[10] * ival,
      _e[14] * ival,

      _e[3]  * ival,
      _e[7]  * ival,
      _e[11] * ival,
      _e[15] * ival
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
Mat4<T>::operator/( const Mat4<T>& m ) const
{
   return Mat4<T>(
      _e[0]  / m._e[0],
      _e[4]  / m._e[4],
      _e[8]  / m._e[8],
      _e[12] / m._e[12],

      _e[1]  / m._e[1],
      _e[5]  / m._e[5],
      _e[9]  / m._e[9],
      _e[13] / m._e[13],

      _e[2]  / m._e[2],
      _e[6]  / m._e[6],
      _e[10] / m._e[10],
      _e[14] / m._e[14],

      _e[3]  / m._e[3],
      _e[7]  / m._e[7],
      _e[11] / m._e[11],
      _e[15] / m._e[15]
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Mat4<T>::operator*( const Vec4<T>& vec ) const
{
   return Vec4<T>(
      _e[0]*vec(0) + _e[4]*vec(1) + _e[8]*vec(2) + _e[12]*vec(3),
      _e[1]*vec(0) + _e[5]*vec(1) + _e[9]*vec(2) + _e[13]*vec(3),
      _e[2]*vec(0) + _e[6]*vec(1) + _e[10]*vec(2)+ _e[14]*vec(3),
      _e[3]*vec(0) + _e[7]*vec(1) + _e[11]*vec(2)+ _e[15]*vec(3)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Mat4<T>::operator*( const Vec3<T>& vec ) const
{
   return Vec3<T>(
      _e[0]*vec(0) + _e[4]*vec(1) + _e[8]*vec(2) + _e[12],
      _e[1]*vec(0) + _e[5]*vec(1) + _e[9]*vec(2) + _e[13],
      _e[2]*vec(0) + _e[6]*vec(1) + _e[10]*vec(2)+ _e[14]
   );
}


//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Mat4<T>::operator^( const Vec3<T>& vec ) const
{
   return Vec3<T>(
      _e[0]*vec(0) + _e[4]*vec(1) + _e[8]*vec(2),
      _e[1]*vec(0) + _e[5]*vec(1) + _e[9]*vec(2),
      _e[2]*vec(0) + _e[6]*vec(1) + _e[10]*vec(2)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Mat4<T>::operator|( const Vec3<T>& vec ) const
{
   T div = (T)(1) / ( _e[3]*vec(0) + _e[7]*vec(1) + _e[11]*vec(2)+ _e[15] );

   return Vec3<T>(
      div * ( _e[0]*vec(0) + _e[4]*vec(1) + _e[8]*vec(2) + _e[12] ),
      div * ( _e[1]*vec(0) + _e[5]*vec(1) + _e[9]*vec(2) + _e[13] ),
      div * ( _e[2]*vec(0) + _e[6]*vec(1) + _e[10]*vec(2)+ _e[14] )
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Mat4<T>::operator|( const Vec4<T>& vec ) const
{
   T div = (T)(1) / ( _e[3]*vec(0) + _e[7]*vec(1) + _e[11]*vec(2)+ _e[15]*vec(3) );

   return Vec4<T>(
      div * ( _e[0]*vec(0) + _e[4]*vec(1) + _e[ 8]*vec(2) + _e[12]*vec(3) ),
      div * ( _e[1]*vec(0) + _e[5]*vec(1) + _e[ 9]*vec(2) + _e[13]*vec(3) ),
      div * ( _e[2]*vec(0) + _e[6]*vec(1) + _e[10]*vec(2) + _e[14]*vec(3) ),
      T(1)
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Ray<T>
Mat4<T>::operator*( const Ray<T>& ray ) const
{
   return Ray<T>(
      *this * ray.origin(),
      *this ^ ray.direction()
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>&
Mat4<T>::operator+=( const Mat4<T>& m )
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
   _e[9] += m._e[9];
   _e[10]+= m._e[10];
   _e[11]+= m._e[11];

   _e[12]+= m._e[12];
   _e[13]+= m._e[13];
   _e[14]+= m._e[14];
   _e[15]+= m._e[15];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>&
Mat4<T>::operator-=( const Mat4<T>& m )
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
   _e[9] -= m._e[9];
   _e[10]-= m._e[10];
   _e[11]-= m._e[11];

   _e[12]-= m._e[12];
   _e[13]-= m._e[13];
   _e[14]-= m._e[14];
   _e[15]-= m._e[15];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>&
Mat4<T>::operator*=( const T& val )
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
   _e[9] *= val;
   _e[10]*= val;
   _e[11]*= val;

   _e[12]*= val;
   _e[13]*= val;
   _e[14]*= val;
   _e[15]*= val;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>&
Mat4<T>::operator*=( const Mat4<T>& m )
{
   T e0  = _e[0];
   T e1  = _e[1];
   T e2  = _e[2];
   T e3  = _e[3];
   T e4  = _e[4];
   T e5  = _e[5];
   T e6  = _e[6];
   T e7  = _e[7];
   T e8  = _e[8];
   T e9  = _e[9];
   T e10 = _e[10];
   T e11 = _e[11];

   _e[0] = e0*m._e[0] + e4*m._e[1] + e8*m._e[2]  + _e[12]*m._e[3];
   _e[1] = e1*m._e[0] + e5*m._e[1] + e9*m._e[2]  + _e[13]*m._e[3];
   _e[2] = e2*m._e[0] + e6*m._e[1] + e10*m._e[2] + _e[14]*m._e[3];
   _e[3] = e3*m._e[0] + e7*m._e[1] + e11*m._e[2] + _e[15]*m._e[3];

   _e[4] = e0*m._e[4] + e4*m._e[5] + e8*m._e[6]  + _e[12]*m._e[7];
   _e[5] = e1*m._e[4] + e5*m._e[5] + e9*m._e[6]  + _e[13]*m._e[7];
   _e[6] = e2*m._e[4] + e6*m._e[5] + e10*m._e[6] + _e[14]*m._e[7];
   _e[7] = e3*m._e[4] + e7*m._e[5] + e11*m._e[6] + _e[15]*m._e[7];

   _e[8] = e0*m._e[8] + e4*m._e[9] + e8*m._e[10] + _e[12]*m._e[11];
   _e[9] = e1*m._e[8] + e5*m._e[9] + e9*m._e[10] + _e[13]*m._e[11];
   _e[10]= e2*m._e[8] + e6*m._e[9] + e10*m._e[10]+ _e[14]*m._e[11];
   _e[11]= e3*m._e[8] + e7*m._e[9] + e11*m._e[10]+ _e[15]*m._e[11];

   _e[12]= e0*m._e[12]+ e4*m._e[13]+ e8*m._e[14] + _e[12]*m._e[15];
   _e[13]= e1*m._e[12]+ e5*m._e[13]+ e9*m._e[14] + _e[13]*m._e[15];
   _e[14]= e2*m._e[12]+ e6*m._e[13]+ e10*m._e[14]+ _e[14]*m._e[15];
   _e[15]= e3*m._e[12]+ e7*m._e[13]+ e11*m._e[14]+ _e[15]*m._e[15];

   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>&
Mat4<T>::operator/=( const T& val )
{
   T ival = (T)1 / val;
   _e[0]  *= ival;
   _e[1]  *= ival;
   _e[2]  *= ival;
   _e[3]  *= ival;

   _e[4]  *= ival;
   _e[5]  *= ival;
   _e[6]  *= ival;
   _e[7]  *= ival;

   _e[8]  *= ival;
   _e[9]  *= ival;
   _e[10] *= ival;
   _e[11] *= ival;

   _e[12] *= ival;
   _e[13] *= ival;
   _e[14] *= ival;
   _e[15] *= ival;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>&
Mat4<T>::operator/=( const Mat4<T>& m )
{
   _e[0]  /= m._e[0];
   _e[1]  /= m._e[1];
   _e[2]  /= m._e[2];
   _e[3]  /= m._e[3];

   _e[4]  /= m._e[4];
   _e[5]  /= m._e[5];
   _e[6]  /= m._e[6];
   _e[7]  /= m._e[7];

   _e[8]  /= m._e[8];
   _e[9]  /= m._e[9];
   _e[10] /= m._e[10];
   _e[11] /= m._e[11];

   _e[12] /= m._e[12];
   _e[13] /= m._e[13];
   _e[14] /= m._e[14];
   _e[15] /= m._e[15];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>&
Mat4<T>::operator=( const Mat4<T>& m )
{
   _e[0]  = m._e[0];
   _e[1]  = m._e[1];
   _e[2]  = m._e[2];
   _e[3]  = m._e[3];

   _e[4]  = m._e[4];
   _e[5]  = m._e[5];
   _e[6]  = m._e[6];
   _e[7]  = m._e[7];

   _e[8]  = m._e[8];
   _e[9]  = m._e[9];
   _e[10] = m._e[10];
   _e[11] = m._e[11];

   _e[12] = m._e[12];
   _e[13] = m._e[13];
   _e[14] = m._e[14];
   _e[15] = m._e[15];
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Mat4<T>::operator==( const Mat4<T>& m ) const
{
   return _e[0] == m._e[0] && _e[1] == m._e[1] && _e[2] == m._e[2] && _e[3] == m._e[3] &&
          _e[4] == m._e[4] && _e[5] == m._e[5] && _e[6] == m._e[6] && _e[7] == m._e[7] &&
          _e[8] == m._e[8] && _e[9] == m._e[9] && _e[10] == m._e[10] && _e[11] == m._e[11] &&
          _e[12] == m._e[12] && _e[13] == m._e[13] && _e[14] == m._e[14] && _e[15] == m._e[15];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Mat4<T>::operator!=( const Mat4<T>& m ) const
{
   return !( *this == m );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Mat4<T>::operator()( int pos )
{
   return _e[pos];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T&
Mat4<T>::operator()( int pos ) const
{
   return _e[pos];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
Mat4<T>::operator()( int line, int col )
{
   return _e[ line + (col<<2) ];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T&
Mat4<T>::operator()( int line, int col ) const
{
   return _e[ line + (col<<2) ];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec4<T>&
Mat4<T>::col( int idx ) const
{
   return *(const Vec4f*)(_e + 4*idx);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec4<T>
Mat4<T>::row( int idx ) const
{
   return Vec4<T>( _e[idx], _e[idx+4], _e[idx+8], _e[idx+12] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec3<T>&
Mat4<T>::col3( int idx ) const
{
   return *(const Vec3f*)(_e + 4*idx);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Mat4<T>::row3( int idx ) const
{
   return Vec3<T>( _e[idx], _e[idx+4], _e[idx+8] );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Mat4<T>::col( int idx, const Vec4<T>& v )
{
   _e[4*idx  ] = v.x;
   _e[4*idx+1] = v.y;
   _e[4*idx+2] = v.z;
   _e[4*idx+3] = v.w;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Mat4<T>::row( int idx, const Vec4<T>& v )
{
   _e[idx   ] = v.x;
   _e[idx+ 4] = v.y;
   _e[idx+ 8] = v.z;
   _e[idx+12] = v.w;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Mat4<T>
operator*( const T& val, const Mat4<T>& m )
{
   return Mat4<T>(
      m(0,0) * val,
      m(0,1) * val,
      m(0,2) * val,
      m(0,3) * val,

      m(1,0) * val,
      m(1,1) * val,
      m(1,2) * val,
      m(1,3) * val,

      m(2,0) * val,
      m(2,1) * val,
      m(2,2) * val,
      m(2,3) * val,

      m(3,0) * val,
      m(3,1) * val,
      m(3,2) * val,
      m(3,3) * val
  );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<( TextStream& stream, const Mat4<T>& m )
{
   return stream
      << "[[" << m(0,0) << "," << m(0,1) << "," << m(0,2) << "," << m(0,3)
      << "][" << m(1,0) << "," << m(1,1) << "," << m(1,2) << "," << m(1,3)
      << "][" << m(2,0) << "," << m(2,1) << "," << m(2,2) << "," << m(2,3)
      << "][" << m(3,0) << "," << m(3,1) << "," << m(3,2) << "," << m(3,3)
      << "]]";
}

/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef Mat4< int >    Mat4i;
typedef Mat4< float >  Mat4f;
typedef Mat4< double > Mat4d;

NAMESPACE_END

#endif
