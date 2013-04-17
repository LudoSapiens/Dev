/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_SPHERE_H
#define CGMATH_SPHERE_H

#include <CGMath/StdDefs.h>

#include <CGMath/Vec3.h>

#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Sphere
==============================================================================*/

template< typename T >
class Sphere
{

public: 
   
   /*----- methods -----*/

   Sphere();
   Sphere( T radius );
   Sphere( const Vec3<T>& center, T radius );

  ~Sphere();

   // Attributes.
   inline void radius( T );
   inline T radius() const;

   inline void center( const Vec3<T>& );
   inline const Vec3<T>& center() const;

   // Queries.
   inline bool isInside( const Vec3<T>& pt ) const;
   inline bool isOverlapping( const Sphere<T>& ) const;

   // Operations.
   inline Vec3<T> project( const Vec3<T>& pt ) const;

private:
   
   /*----- data members -----*/
   
   Vec3<T> _center;
   T       _radius;
};

//------------------------------------------------------------------------------
//!
template< typename T > inline
Sphere<T>::Sphere() {}
   
//------------------------------------------------------------------------------
//!
template< typename T > inline
Sphere<T>::Sphere( T radius ) :
   _center( Vec3<T>::zero() ),
   _radius( radius )
{
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Sphere<T>::Sphere( const Vec3<T>& center, T radius ) :
   _center( center ),
   _radius( radius )
{
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Sphere<T>::~Sphere() {}

//------------------------------------------------------------------------------
//!
template< typename T > inline void 
Sphere<T>::radius( T radius )
{
   _radius = radius;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Sphere<T>::radius() const
{
   return _radius;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void 
Sphere<T>::center( const Vec3<T>& center )
{
   _center = center;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec3<T>&
Sphere<T>::center() const
{
   return _center;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Sphere<T>::isInside( const Vec3<T>& pt ) const
{
   return (pt-_center).sqrLength() <= _radius*_radius;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Sphere<T>::isOverlapping( const Sphere<T>& sph ) const
{
   T dist = _radius+sph.radius();
   return (_center-sph.center()).sqrLength() <= dist*dist;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Sphere<T>::project( const Vec3<T>& pt ) const
{
   return _center + (pt-_center).rescale(_radius);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<
( TextStream& stream, const Sphere<T>& sphere )
{
   return stream << sphere.center() << " - " << sphere.radius();
}

/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef Sphere< float >  Spheref;
typedef Sphere< double > Sphered;

NAMESPACE_END

#endif
