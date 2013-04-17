/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_QUADRIC_H
#define CGMATH_QUADRIC_H

#include <CGMath/StdDefs.h>

#include <CGMath/Vec3.h>
#include <CGMath/Mat3.h>
#include <CGMath/Mat4.h>
#include <CGMath/Plane.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Quadric
==============================================================================*/
template< typename T >
class Quadric
{
public:

   /*----- static methods -----*/

   static Quadric<T> zero();

   /*----- methods -----*/

   Quadric() {}
   Quadric( const Plane<T>& plane, const T& w );
   Quadric( const Vec3<T>& c0, const Vec3<T>& c1, const T& w );

   Quadric<T>&  set( const Plane<T>& plane, const T& w );
   Quadric<T>&  set( const T& a, const T& b, const T& c, const T& d, const T& w );
   Quadric<T>&  set( const Vec3<T>& c0, const Vec3<T>& c1, const T& w );

   Mat3<T>  A() const;
   Vec3<T>  b() const;
   T        c() const;

   Mat4<T>  homogeneous() const;

   T  evaluate( const T& x, const T& y, const T& z ) const;
   T  evaluate( const Vec3<T>& v ) const;

   bool  optimize( Vec3<T>& v ) const;

   Quadric&  operator+=( const Quadric<T>& q );
   Quadric&  operator-=( const Quadric<T>& q );
   Quadric&  operator*=( T sf );

protected:

   /*----- data members -----*/

   // The 10 coefficients.
   T  _a2, _ab, _ac, _ad,
           _b2, _bc, _bd,
                _c2, _cd,
                     _d2;

}; //class Quadric

//------------------------------------------------------------------------------
//!
template< typename T > Quadric<T>
Quadric<T>::zero()
{
   Quadric<T> q;
   q._a2 = (T)0;
   q._ab = (T)0;
   q._ac = (T)0;
   q._ad = (T)0;
   q._b2 = (T)0;
   q._bc = (T)0;
   q._bd = (T)0;
   q._c2 = (T)0;
   q._cd = (T)0;
   q._d2 = (T)0;
   return q;
}

//------------------------------------------------------------------------------
//!
template< typename T >
Quadric<T>::Quadric( const Plane<T>& plane, const T& w )
{
   set( plane.a(), plane.b(), plane.c(), plane.d(), w );
}

//------------------------------------------------------------------------------
//!
template< typename T >
Quadric<T>::Quadric( const Vec3<T>& c0, const Vec3<T>& c1, const T& w )
{
   set( c0, c1, w );
}

//------------------------------------------------------------------------------
//!
template< typename T > Quadric<T>&
Quadric<T>::set( const Plane<T>& plane, const T& w )
{
   set( plane.a(), plane.b(), plane.c(), plane.d(), w );
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > Quadric<T>&
Quadric<T>::set( const T& a, const T& b, const T& c, const T& d, const T& w )
{
   _a2 = a*a*w; _ab = a*b*w; _ac = a*c*w; _ad = a*d*w;
                _b2 = b*b*w; _bc = b*c*w; _bd = b*d*w;
                             _c2 = c*c*w; _cd = c*d*w;
                                          _d2 = d*d*w;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > Quadric<T>&
Quadric<T>::set( const Vec3<T>& c0, const Vec3<T>& c1, const T& w )
{
   _a2 = (T)2 * w;
   _ab = (T)0;
   _ac = (T)0;
   _ad = (-c0.x-c1.x)*w;
   _b2 = (T)2 * w;
   _bc = (T)0;
   _bd = (-c0.y-c1.y)*w;
   _c2 = (T)2 * w;
   _cd = (-c0.z-c1.z)*w;
   _d2 = (c0.x*c0.x + c1.x*c1.x + c0.y*c0.y + c1.y*c1.y + c0.z*c0.z + c1.z*c1.z)*w;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > Mat3<T>
Quadric<T>::A() const
{
   return Mat3<T>( _a2, _ab, _ac,
                   _ab, _b2, _bc,
                   _ac, _bc, _c2 );
}

//------------------------------------------------------------------------------
//!
template< typename T > Vec3<T>
Quadric<T>::b() const
{
   return Vec3<T>( _ad, _bd, _cd );
}

//------------------------------------------------------------------------------
//!
template< typename T > T
Quadric<T>::c() const
{
   return _d2;
}

//------------------------------------------------------------------------------
//!
template< typename T > Mat4<T>
Quadric<T>::homogeneous() const
{
   return Mat4<T>( _a2, _ab, _ac, _ad,
                   _ab, _b2, _bc, _bd,
                   _ac, _bc, _c2, _cd,
                   _ad, _bd, _cd, _d2 );
}

//------------------------------------------------------------------------------
//!
template< typename T > T
Quadric<T>::evaluate( const T& x, const T& y, const T& z ) const
{
   // (vT * A * v) + (2 * bT * v) + c
   return x*x*_a2 + 2*x*y*_ab + 2*x*z*_ac + 2*x*_ad
                  +   y*y*_b2 + 2*y*z*_bc + 2*y*_bd
                              +   z*z*_c2 + 2*z*_cd
                                          +     _d2;
}

//------------------------------------------------------------------------------
//!
template< typename T > T
Quadric<T>::evaluate( const Vec3<T>& v ) const
{
   return evaluate( v.x, v.y, v.z );
}

//------------------------------------------------------------------------------
//!
template< typename T > bool
Quadric<T>::optimize( Vec3<T>& v ) const
{
   // Get determinant of A.
   Mat3<T> mA = A();
   T det = mA.determinant();
   if( CGM::equal(det, (T)0.0) )
   {
      return false;
   }
   else
   {
      // v = -A'b
      v = -( mA.getInversed(det) * b() );
      return true;
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > Quadric<T>&
Quadric<T>::operator+=( const Quadric<T>& q )
{
   _a2 += q._a2; _ab += q._ab; _ac += q._ac; _ad += q._ad;
                 _b2 += q._b2; _bc += q._bc; _bd += q._bd;
                               _c2 += q._c2; _cd += q._cd;
                                             _d2 += q._d2;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > Quadric<T>&
Quadric<T>::operator-=( const Quadric<T>& q )
{
   _a2 -= q._a2; _ab -= q._ab; _ac -= q._ac; _ad -= q._ad;
                 _b2 -= q._b2; _bc -= q._bc; _bd -= q._bd;
                               _c2 -= q._c2; _cd -= q._cd;
                                             _d2 -= q._d2;
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename T > Quadric<T>&
Quadric<T>::operator*=( T sf )
{
   _a2 *= sf; _ab *= sf; _ac *= sf; _ad *= sf;
              _b2 *= sf; _bc *= sf; _bd *= sf;
                         _c2 *= sf; _cd *= sf;
                                    _d2 *= sf;
   return *this;
}

/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef Quadric< float >  Quadricf; // Can cause precision issues.
typedef Quadric< double > Quadricd;

NAMESPACE_END

#endif //CGMATH_QUADRIC_H
