/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_PLANE_H
#define CGMATH_PLANE_H

#include <CGMath/StdDefs.h>

#include <CGMath/CGMath.h>
#include <CGMath/Vec3.h>

#include <Base/Dbg/Defs.h>
#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Plane
==============================================================================*/

template < class T >
class Plane
{

public:

   /*----- methods -----*/

   // Constructors/destructor.
   template< typename S > Plane( const Plane<S>& p )
   {
      _dir      = p.direction();
      _distance = p.d();
   }

   Plane() {}
   Plane( const Vec3<T>& dir );
   Plane( const Vec3<T>& dir, T d );
   Plane( const Vec3<T>& dir, const Vec3<T>& pos );
   Plane( const Vec3<T>& pA, const Vec3<T>& pB, const Vec3<T>& pC );
   Plane( T a, T b, T c, T d );
   Plane( const Plane<T>& plane );

   ~Plane() {}

   // Accessors.
   const Vec3<T>& direction() const;
   const T a() const;
   const T b() const;
   const T c() const;
   const T d() const;

   void direction( const Vec3<T>& newDir );
   void a( const T newA );
   void b( const T newB );
   void c( const T newC );
   void d( const T newD );

   const T evaluate( const Vec3<T>& pt ) const;
   bool inFront( const Vec3<T>& pt ) const;
   bool contains( const Vec3<T>& pt, const T& epsilon = T(0) ) const;
   void project( Vec3<T>& pt, int axis ) const;

   Plane<T>& inverse();
   Plane<T> getInversed() const;
   Plane<T>& normalize();
   Plane<T> getNormalized() const;

   // The routines below require the plane to be normalized prior to being called
   Vec3<T> closest( const Vec3<T>& pt ) const;
   T distance( const Vec3<T>& pt ) const;
   void closestAndDistance( const Vec3<T>& pt, Vec3<T>& closest, T& distance ) const;
   bool equal( const Plane<T>& rhs, const T threshold = CGConst<T>::epsilon() ) const;

   Plane<T> operator+( const Plane<T>& rhs ) const;
   Plane<T> operator-( const Plane<T>& rhs ) const;

private:

   /*----- data members -----*/

   Vec3<T> _dir;
   T       _distance;
};

//------------------------------------------------------------------------------
//!
template< typename T > inline
Plane<T>::Plane( const Vec3<T>& dir ) :
   _dir( dir ),
   _distance( T(0) )
{}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Plane<T>::Plane( const Vec3<T>& dir, T d ) :
   _dir( dir ),
   _distance( d )
{}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Plane<T>::Plane( const Vec3<T>& dir, const Vec3<T>& pos ) :
   _dir( dir ),
   _distance( -pos.dot( dir ) )
{
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Plane<T>::Plane( const Vec3<T>& pA, const Vec3<T>& pB, const Vec3<T>& pC )
{
   const Vec3f ab = pB - pA;
   const Vec3f ac = pC - pA;
   _dir = ab.cross(ac);
   _distance = -pA.dot(_dir);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Plane<T>::Plane( T a, T b, T c, T d ) :
   _dir( a, b, c ),
   _distance( d )
{}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Plane<T>::Plane( const Plane<T>& plane ) :
   _dir( plane._dir ),
   _distance( plane._distance )
{}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec3<T>&
Plane<T>::direction() const
{
   return _dir;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T
Plane<T>::a() const
{
   return _dir(0);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T
Plane<T>::b() const
{
   return _dir(1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T
Plane<T>::c() const
{
   return _dir(2);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const T
Plane<T>::d() const
{
   return _distance;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Plane<T>::direction( const Vec3<T>& newDir )
{
   _dir = newDir;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Plane<T>::a( const T newA )
{
   _dir(0) = newA;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Plane<T>::b( const T newB )
{
   _dir(1) = newB;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Plane<T>::c( const T newC )
{
   _dir(2) = newC;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Plane<T>::d( const T newD )
{
   _distance = newD;
}

//------------------------------------------------------------------------------
//! Evaluates A*x + B*y + C*z + D
template< typename T > inline const T
Plane<T>::evaluate( const Vec3<T>& pt ) const
{
   return _dir.dot(pt) + _distance;
}

//------------------------------------------------------------------------------
//! Returns true if the specified point is on the same side of the place as the direction vector
template< typename T > inline bool
Plane<T>::inFront( const Vec3<T>& pt ) const
{
   return evaluate(pt) > T(0);
}

//------------------------------------------------------------------------------
//! Returns true if the specified point is on the plane (given the specified epsilon)
template< typename T > inline bool
Plane<T>::contains( const Vec3<T>& pt, const T& epsilon ) const
{
   return CGM::abs(evaluate(pt)) <= epsilon;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Plane<T>::project( Vec3<T>& pt, int axis ) const
{
   switch(axis)
   {
      case 0: pt.x = (-_distance -b()*pt.y -c()*pt.z)/a(); break;
      case 1: pt.y = (-_distance -a()*pt.x -c()*pt.z)/b(); break;
      case 2: pt.z = (-_distance -a()*pt.x -b()*pt.y)/c(); break;
      default:;
   }
}

//------------------------------------------------------------------------------
//! Inverse the plane direction.
template< typename T > inline Plane<T>&
Plane<T>::inverse()
{
   _dir      = -_dir;
   _distance = -_distance;
   return *this;
}

//------------------------------------------------------------------------------
//! Inverse the plane direction.
template< typename T > inline Plane<T>
Plane<T>::getInversed() const
{
   return Plane<T>( -_dir, -_distance );
}

//------------------------------------------------------------------------------
//! Makes the direction vector unit lenght, and adjusts 'd' accordingly
template< typename T > inline Plane<T>&
Plane<T>::normalize()
{
   T sf = T(1)/_dir.length();
   _dir *= sf;
   _distance *= sf;
   return *this;
}

//------------------------------------------------------------------------------
//! Makes the direction vector unit lenght, and adjusts 'd' accordingly
template< typename T > inline Plane<T>
Plane<T>::getNormalized() const
{
   T sf = T(1)/_dir.length();
   return Plane<T>(_dir*sf, _distance*sf);
}

//------------------------------------------------------------------------------
//! Finds the closest point to 'pt' which lies on the plane.
//! IMPORTANT: The plane must be normalized prior to calling this function.
template< typename T > inline Vec3<T>
Plane<T>::closest( const Vec3<T>& pt ) const
{
   return pt - _dir*evaluate(pt);
}

//------------------------------------------------------------------------------
//! Evaluates A*x + B*y + C*z + D, which actually corresponds to the distance
//! IMPORTANT: The plane must be normalized prior to calling this function.
template< typename T > inline T
Plane<T>::distance( const Vec3<T>& pt ) const
{
   //Verify that we are normalized
   CHECK( CGM::equal(_dir.length(), (T)1, (T)CGM::EqualityThreshold) );
   return evaluate(pt);
}

//------------------------------------------------------------------------------
//! Returns both the closest point to the plane, along with its distance
//! IMPORTANT: The plane must be normalized prior to calling this function.
template< typename T > inline void
Plane<T>::closestAndDistance( const Vec3<T>& pt, Vec3<T>& closest, T& distance ) const
{
   distance = evaluate(pt);
   closest = pt - _dir*distance;
}

//------------------------------------------------------------------------------
//! Returns true if the two planes are "equal".
template< typename T > inline bool
Plane<T>::equal( const Plane<T>& rhs, const T threshold ) const
{
   return _dir.equal( rhs._dir, threshold ) &&
      CGM::equal( _distance, rhs._distance, threshold );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Plane<T>
Plane<T>::operator+( const Plane<T>& rhs ) const
{
   return Plane<T>( _dir + rhs._dir, _distance + rhs._distance );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Plane<T>
Plane<T>::operator-( const Plane<T>& rhs ) const
{
   return Plane<T>( _dir - rhs._dir, _distance - rhs._distance );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<
( TextStream& stream, const Plane<T>& pl )
{
   return stream << "(" << pl.a() << "," << pl.b() << "," << pl.c() << "," << pl.d() << ")";
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator>>
( TextStream& stream, Plane<T>& pl )
{
   char ch;
   T a, b, c, d;
   //(a, b, c, d)
   stream >> ch >> a >> ch >> b >> ch >> c >> ch >> d >> ch;
   pl.a(a);
   pl.b(b);
   pl.c(c);
   pl.d(d);
   return stream;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
intersectPlanes( const Plane<T>& p0, const Plane<T>& p1, const Plane<T>& p2 )
{
   Vec3<T> u = cross( p1.direction(), p2.direction() );
   T d       = dot( p0.direction(), u );
   return ( -u*p0.d() + cross( p0.direction(), p2.direction()*p1.d()-p1.direction()*p2.d() ) ) / d;
}

/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef Plane< float >  Planef;
typedef Plane< double > Planed;

NAMESPACE_END

#endif
