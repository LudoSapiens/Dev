/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_POLYGON_H
#define CGMATH_POLYGON_H

#include <CGMath/StdDefs.h>

#include <CGMath/Plane.h>
#include <CGMath/Vec3.h>

#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Polygon
==============================================================================*/

template< typename T >
class Polygon:
   public RCObject
{

public:

   /*----- types and enumerations ----*/

   typedef Vector< Vec3<T> > Container;

   /*----- methods -----*/

   Polygon();

   // Building.
   inline void reserveVertices( size_t qty );
   inline void addVertex( const Vec3<T>& );
   inline void insertVertex( size_t pos, const Vec3<T>& );
   inline void removeVertex( size_t i );
   inline void removeAllVertices();
   inline void computeDerivedData();
   inline void plane( const Plane<T>& plane );

   // Attributes.
   inline size_t numVertices() const;
   inline const Plane<T>& plane() const;
   inline const Vec3<T>& normal() const;
   inline T area() const;

   inline Vec3<T>& vertex( size_t i );
   inline const Vec3<T>& vertex( size_t i ) const;

   inline typename Container::Iterator begin()            { return _vertices.begin(); }
   inline typename Container::Iterator end()              { return _vertices.end(); }
   inline typename Container::ConstIterator begin() const { return _vertices.begin(); }
   inline typename Container::ConstIterator end() const   { return _vertices.end(); }

   inline Vec3<T> computeCentroid() const;

   // Queries.
   inline bool inFront( const Vec3<T>& ) const;
   inline bool inBack( const Vec3<T>& ) const;
   inline bool isInside( const Vec3<T>& ) const;
   inline bool isConvex() const;

   // Operations.
   inline void inverse();

protected:

   virtual ~Polygon();

   /*----- data members -----*/

   Container _vertices;
   Plane<T>  _plane;
   mutable T _area;
};

//------------------------------------------------------------------------------
//!
template< typename T > inline
Polygon<T>::Polygon() : _area( T(-1) ) {}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Polygon<T>::~Polygon() {}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Polygon<T>::reserveVertices( size_t qty )
{
   _vertices.reserve( qty );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Polygon<T>::addVertex( const Vec3<T>& pt )
{
   _vertices.pushBack( pt );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Polygon<T>::insertVertex( size_t pos, const Vec3<T>& pt )
{
   _vertices.insert( _vertices.begin()+pos, pt );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Polygon<T>::removeVertex( size_t i )
{
   _vertices.erase( _vertices.begin() + i );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Polygon<T>::removeAllVertices()
{
   _vertices.clear();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Polygon<T>::computeDerivedData()
{
   if( !_vertices.empty() )
   {
      // Computing normal with Newell's method.
      Vec3f n( 0.0f );
      for( size_t v = 0; v < _vertices.size(); ++v )
      {
         const Vec3f& v0 = _vertices[v];
         const Vec3f& v1 = _vertices[(v+1)%numVertices()];

         n.x += (v0.y-v1.y) * (v0.z+v1.z);
         n.y += (v0.z-v1.z) * (v0.x+v1.x);
         n.z += (v0.x-v1.x) * (v0.y+v1.y);
      }
      _plane = Plane<T>( n, _vertices[0] );
      _plane.normalize();
   }
   // Invalid area.
   _area = T(-1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Polygon<T>::plane( const Plane<T>& plane )
{
   _plane = plane;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline size_t
Polygon<T>::numVertices() const
{
   return _vertices.size();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Plane<T>&
Polygon<T>::plane() const
{
   return _plane;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec3<T>&
Polygon<T>::normal() const
{
   return _plane.direction();
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>&
Polygon<T>::vertex( size_t i )
{
   return _vertices[i];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec3<T>&
Polygon<T>::vertex( size_t i ) const
{
   return _vertices[i];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Polygon<T>::computeCentroid() const
{
   Vec3<T> centroid = Vec3<T>::zero();
   for( size_t i = 0; i < _vertices.size(); ++i )
   {
      centroid += _vertices[i];
   }
   centroid /= (T)_vertices.size();
   return centroid;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Polygon<T>::inFront( const Vec3<T>& pt ) const
{
   return _plane.inFront( pt );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Polygon<T>::inBack( const Vec3<T>& pt ) const
{
   return !_plane.inFront( pt );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Polygon<T>::isInside( const Vec3<T>& pt ) const
{
   // Projection.
   uint x = 0;
   uint y = 1;
   if( CGM::abs(normal().x) > CGM::abs(normal().y) )
   {
      if( CGM::abs(normal().x) > CGM::abs(normal().z) )
      {
         x = 2;
         y = 1;
      }
   }
   else
   {
      if( CGM::abs(normal().y) > CGM::abs(normal().z) )
      {
         x = 0;
         y = 2;
      }
   }

   // Test inclusion.
   bool in = false;
   size_t v0, v1;
   for( v0 = numVertices()-1, v1 = 0; v1 < numVertices(); v0 = v1++ )
   {
      if( ((_vertices[v0](y) <= pt(y)) && (_vertices[v1](y) >  pt(y))) ||
          ((_vertices[v0](y) >  pt(y)) && (_vertices[v1](y) <= pt(y)))
         )
      {
         T vt = (pt(y)-_vertices[v0](y)) / (_vertices[v1](y)-_vertices[v0](y));
         if( pt(x) < _vertices[v0](x) + vt * (_vertices[v1](x)-_vertices[v0](x)) )
         {
            in = !in;
         }
      }
   }

   return in;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
Polygon<T>::isConvex() const
{
  // Projection.
   uint x = 0;
   uint y = 1;
   uint z = 2;
   if( CGM::abs(normal().x) > CGM::abs(normal().y) )
   {
      if( CGM::abs(normal().x) > CGM::abs(normal().z) )
      {
         x = 2;
         y = 1;
         z = 0;
      }
   }
   else
   {
      if( CGM::abs(normal().y) > CGM::abs(normal().z) )
      {
         x = 0;
         y = 2;
         z = 1;
      }
   }

   uint order = 0;

   size_t v0 = numVertices()-2;
   size_t v1 = numVertices()-1;
   size_t v2 = 0;

   for( ; v2 < numVertices(); v0 = v1, v1=v2++ )
   {
      float z = (_vertices[v1](x) - _vertices[v0](x)) * (_vertices[v2](y) - _vertices[v1](y));
      z      -= (_vertices[v1](y) - _vertices[v0](y)) * (_vertices[v2](x) - _vertices[v1](x));

      if( z < 0.0f )
      {
         order |= 1;
      }
      else if( z >= 0.0f )
      {
         order |= 2;
      }
   }
   return order != 3;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
Polygon<T>::inverse()
{
   _plane.inverse();
   _vertices.reverse( _vertices.begin(), _vertices.end() );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Polygon<T>::area() const
{
   if( _area >= 0.0f )
   {
      return _area;
   }
   else
   {
      // Projection.
      uint x = 0;
      uint y = 1;
      uint z = 2;
      if( CGM::abs(normal().x) > CGM::abs(normal().y) )
      {
         if( CGM::abs(normal().x) > CGM::abs(normal().z) )
         {
            x = 2;
            y = 1;
            z = 0;
         }
      }
      else
      {
         if( CGM::abs(normal().y) > CGM::abs(normal().z) )
         {
            x = 0;
            y = 2;
            z = 1;
         }
      }

      // Projected area computation.
      _area = T(0);
      for( size_t i = numVertices()-1, j = 0; j < numVertices(); i = j++ )
      {
         _area += _vertices[i](x)*_vertices[j](y) - _vertices[j](x)*_vertices[i](y);
      }
     _area = CGM::abs(_area /(T(2)*_plane.direction()(z)));
   }
   return _area;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<
( TextStream& stream, const Polygon<T>& poly )
{
   stream << "p: " << poly.plane() << " v: ";
   for( size_t i = 0; i < poly.numVertices(); ++i )
   {
      stream << poly.vertex(i);
   }

   return stream;
}


/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef Polygon< float >  Polygonf;
typedef Polygon< double > Polygond;

NAMESPACE_END

#endif

