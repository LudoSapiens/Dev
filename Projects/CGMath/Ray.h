/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_RAY_H
#define CGMATH_RAY_H

#include <CGMath/StdDefs.h>

#include <CGMath/Vec3.h>

#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Ray
==============================================================================*/

//! Container class for ray.

template< typename T >
class  Ray
{

public: 

   /*----- methods -----*/

   // Constructors/destructor.
   Ray() {}
   Ray( const Vec3<T>& dir );
   Ray( const Vec3<T>& origin, const Vec3<T>& dir );
   Ray( const Ray<T>& ray );

   ~Ray() {}

   // Accessors.
   void origin( const Vec3<T>& origin );
   void direction( const Vec3<T>& dir );
   Vec3<T>& origin();
   Vec3<T>& direction();
   const Vec3<T>& origin() const;
   const Vec3<T>& direction() const;

   Vec3<T> point() const;
   Vec3<T> point( T t ) const;

   T  computeT( const Vec3<T>& point ) const;

   TextStream&  operator<<( const TextStream& os ) const;

private: 

   /*----- data members -----*/

   Vec3<T> _origin;
   Vec3<T> _dir;
};

//------------------------------------------------------------------------------
//!
template< typename T > inline
Ray<T>::Ray( const Vec3<T>& dir ) :
   _origin( Vec3<T>::zero() ),
   _dir( dir )
{}
   
//------------------------------------------------------------------------------
//!
template< typename T > inline
Ray<T>::Ray( const Vec3<T>& origin, const Vec3<T>& dir ) :
   _origin( origin ),
   _dir( dir )
{}
   
//------------------------------------------------------------------------------
//!
template< typename T > inline
Ray<T>::Ray( const Ray<T>& ray ) :
   _origin( ray._origin ),
   _dir( ray._dir )
{}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Ray<T>::origin( const Vec3<T>& origin )
{
   _origin = origin;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Ray<T>::direction( const Vec3<T>& dir )
{
   _dir = dir;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>& 
Ray<T>::origin()
{
   return _origin;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>& 
Ray<T>::direction()
{
   return _dir;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec3<T>& 
Ray<T>::origin() const
{
   return _origin;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec3<T>& 
Ray<T>::direction() const
{
   return _dir;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Ray<T>::point() const
{
   return _origin + _dir;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Ray<T>::point( T t ) const
{
   return _origin + t*_dir;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Ray<T>::computeT( const Vec3<T>& point ) const
{
   return _dir.dot( point - _origin ) / _dir.dot( _dir );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<( TextStream& os, const Ray<T>& ray )
{
   return os << "ray["
             << "ori=" << ray.origin()
             << " "
             << "dir=" << ray.direction()
             << "]";
}

/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef Ray< float >  Rayf;
typedef Ray< double > Rayd;

NAMESPACE_END

#endif
