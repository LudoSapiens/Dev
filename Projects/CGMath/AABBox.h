/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_AABBOX_H
#define CGMATH_AABBOX_H

#include <CGMath/StdDefs.h>

#include <CGMath/Mat3.h>
#include <CGMath/Ref.h>
#include <CGMath/Vec2.h>
#include <CGMath/Vec3.h>

#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS AABBox
==============================================================================*/

//! Axis Aligned BoundingBox

template< typename T >
class AABBox
{

public:

   /*----- static methods -----*/

   inline static AABBox<T> empty();

   /*----- methods -----*/

   inline AABBox();
   inline AABBox( T r );
   inline AABBox( const Vec3<T>& );
   inline AABBox( const Vec3<T>&, const Vec3<T>& );
   inline AABBox( const Vec2<T>&, const Vec2<T>&, const Vec2<T>& );
   inline AABBox( T x0, T x1, T y0, T y1, T z0, T z1 );
   inline AABBox( const Mat3<T>& m, const Vec3<T>& t, const AABBox<T>& aabb );
   inline AABBox( const Mat3<T>& m, const Vec3<T>& t, const Vec3<T>& size );

   inline ~AABBox();

   // Public methods
   inline void set( T r );
   inline void set( const Vec3<T>& );
   inline void set( const Vec3<T>&, const Vec3<T>& );
   inline void set( const Vec2<T>&, const Vec2<T>&, const Vec2<T>& );
   inline void set( T x0, T x1, T y0, T y1, T z0, T z1 );
   inline void set( const Mat3<T>& m, const Vec3<T>& t, const AABBox<T>& aabb );
   inline void set( const Mat3<T>& m, const Vec3<T>& t, const Vec3<T>& size );

   inline void operator|=( const Vec3<T>& );
   inline void operator|=( const AABBox& );
   inline AABBox<T> operator&( const AABBox& );
   inline void grow( T v );
   inline void grow( const Vec3<T>& v );
   inline void shrink( T v );
   inline void shrink( const Vec3<T>& v );

   inline void translate( const Vec3<T>& v );

   inline const Vec2<T>& slabX() const;
   inline const Vec2<T>& slabY() const;
   inline const Vec2<T>& slabZ() const;
   inline const Vec2<T>& slab( uint axis ) const;

   inline Vec2<T>& slabX();
   inline Vec2<T>& slabY();
   inline Vec2<T>& slabZ();
   inline Vec2<T>& slab( uint axis );

   inline T min( uint axis ) const;
   inline T max( uint axis ) const;

   inline T& min( uint axis );
   inline T& max( uint axis );

   inline T maxAbs( uint axis ) const;
   inline T maxAbs() const;

   inline Vec3<T> size() const;
   inline T size( uint axis ) const;
   inline T maxSize() const;

   inline T surface() const;
   inline T volume() const;

   inline Vec3<T> corner( int x, int y, int z ) const;
   inline Vec3<T> corner( int corner ) const;

   inline Vec3<T> center() const;
   inline T center( uint axis ) const;

   inline bool isEmpty() const;

   inline bool isOverlapping( const AABBox<T>& ) const;
   inline bool isOverlapping( const AABBox<T>&, const Ref<T>& ) const;
   inline bool isInside( const Vec3<T>& ) const;
   inline bool isInsideCO( const Vec3<T>& ) const;
   inline bool isInsideOO( const Vec3<T>& ) const;
   inline bool isInside( const AABBox<T>& ) const;

   inline uint longestSide() const;

private:

   /*----- data members -----*/

   Vec2<T> _s[3];
};

//------------------------------------------------------------------------------
//!
template< typename T > inline AABBox<T>
AABBox<T>::empty()
{
   return AABBox<T>(
      CGConst<T>::highest(), CGConst<T>::lowest(),
      CGConst<T>::highest(), CGConst<T>::lowest(),
      CGConst<T>::highest(), CGConst<T>::lowest()
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
AABBox<T>::AABBox() {}

//------------------------------------------------------------------------------
//!
template< typename T > inline
AABBox<T>::AABBox( T r )
{
   set(r);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
AABBox<T>::AABBox( const Vec3<T>& pt )
{
   set(pt);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
AABBox<T>::AABBox( const Vec3<T>& c0, const Vec3<T>& c7 )
{
   set(c0, c7);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
AABBox<T>::AABBox( const Vec2<T>& x, const Vec2<T>& y, const Vec2<T>& z )
{
   set(x, y, z);
}

//------------------------------------------------------------------------------
//!
template< typename T >  inline
AABBox<T>::AABBox( T x0, T x1, T y0, T y1, T z0, T z1 )
{
   set(x0, x1, y0, y1, z0, z1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
AABBox<T>::AABBox( const Mat3<T>& m, const Vec3<T>& t, const AABBox<T>& aabb )
{
   set(m, t, aabb);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
AABBox<T>::AABBox( const Mat3<T>& m, const Vec3<T>& t, const Vec3<T>& size )
{
   set(m, t, size);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
AABBox<T>::~AABBox() {}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AABBox<T>::set( T r )
{
   _s[0](0) = -r;
   _s[0](1) = r;
   _s[1](0) = -r;
   _s[1](1) = r;
   _s[2](0) = -r;
   _s[2](1) = r;
}

//------------------------------------------------------------------------------
//! Set the bounding box to contain a point.
template< typename T > inline void
AABBox<T>::set( const Vec3<T>& pt )
{
   _s[0](0) = pt.x;
   _s[0](1) = pt.x;
   _s[1](0) = pt.y;
   _s[1](1) = pt.y;
   _s[2](0) = pt.z;
   _s[2](1) = pt.z;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AABBox<T>::set( const Vec3<T>& c0, const Vec3<T>& c7 )
{
   _s[0](0) = c0.x;
   _s[0](1) = c7.x;
   _s[1](0) = c0.y;
   _s[1](1) = c7.y;
   _s[2](0) = c0.z;
   _s[2](1) = c7.z;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AABBox<T>::set( const Vec2<T>& x, const Vec2<T>& y, const Vec2<T>& z )
{
   _s[0] = x;
   _s[1] = y;
   _s[2] = z;
}

//------------------------------------------------------------------------------
//!
template< typename T >  inline void
AABBox<T>::set( T x0, T x1, T y0, T y1, T z0, T z1 )
{
   _s[0](0) = x0;
   _s[0](1) = x1;
   _s[1](0) = y0;
   _s[1](1) = y1;
   _s[2](0) = z0;
   _s[2](1) = z1;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AABBox<T>::set( const Mat3<T>& m, const Vec3<T>& t, const AABBox<T>& aabb )
{
   for( uint i = 0; i < 3; ++i )
   {
      _s[i](0) = t(i);
      _s[i](1) = t(i);
      for( uint j = 0; j < 3; ++j )
      {
         T s0 = m(i,j) * aabb._s[j](0);
         T s1 = m(i,j) * aabb._s[j](1);
         if( s0 < s1 )
         {
            _s[i](0) += s0;
            _s[i](1) += s1;
         }
         else
         {
            _s[i](0) += s1;
            _s[i](1) += s0;
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AABBox<T>::set( const Mat3<T>& m, const Vec3<T>& t, const Vec3<T>& size )
{
   for( uint i = 0; i < 3; ++i )
   {
      _s[i](0) = t(i);
      _s[i](1) = t(i);
      for( uint j = 0; j < 3; ++j )
      {
         T s1 = m(i,j) * size(j);
         if( T(0) < s1 )
         {
            _s[i](1) += s1;
         }
         else
         {
            _s[i](0) += s1;
         }
      }
   }
}

//------------------------------------------------------------------------------
//! Add a point to the bounding box.
//! Before using it make sure the bounding box is
//! initiliazed.
template< typename T > inline void
AABBox<T>::operator|=( const Vec3<T>& point )
{
   if( point(0) < _s[0](0) )
   {
      _s[0](0) = point(0);
   }
   if( point(0) > _s[0](1) )
   {
      _s[0](1) = point(0);
   }

   if( point(1) < _s[1](0) )
   {
      _s[1](0) = point(1);
   }
   if( point(1) > _s[1](1) )
   {
      _s[1](1) = point(1);
   }

   if( point(2) < _s[2](0) )
   {
      _s[2](0) = point(2);
   }
   if( point(2) > _s[2](1) )
   {
      _s[2](1) = point(2);
   }
}

//------------------------------------------------------------------------------
//! Add a bb to the bounding box.
//! Before using it make sure the bounding box is
//! initiliazed.
template< typename T > inline void
AABBox<T>::operator|=( const AABBox<T>& bb )
{
   if( bb._s[0](0) < _s[0](0) )
   {
      _s[0](0) = bb._s[0](0);
   }
   if( bb._s[0](1) > _s[0](1) )
   {
      _s[0](1) = bb._s[0](1);
   }

   if( bb._s[1](0) < _s[1](0) )
   {
      _s[1](0) = bb._s[1](0);
   }
   if( bb._s[1](1) > _s[1](1) )
   {
      _s[1](1) = bb._s[1](1);
   }

   if( bb._s[2](0) < _s[2](0) )
   {
      _s[2](0) = bb._s[2](0);
   }
   if( bb._s[2](1) > _s[2](1) )
   {
      _s[2](1) = bb._s[2](1);
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline AABBox<T>
AABBox<T>::operator&( const AABBox& bb )
{
   return AABBox(
      CGM::max( corner(0), bb.corner(0) ),
      CGM::min( corner(7), bb.corner(7) )
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AABBox<T>::grow( T v )
{
   _s[0](0) -= v;
   _s[1](0) -= v;
   _s[2](0) -= v;
   _s[0](1) += v;
   _s[1](1) += v;
   _s[2](1) += v;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AABBox<T>::grow( const Vec3<T>& v )
{
   _s[0](0) -= v.x;
   _s[1](0) -= v.y;
   _s[2](0) -= v.z;
   _s[0](1) += v.x;
   _s[1](1) += v.y;
   _s[2](1) += v.z;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AABBox<T>::shrink( T v )
{
   _s[0](0) += v;
   _s[1](0) += v;
   _s[2](0) += v;
   _s[0](1) -= v;
   _s[1](1) -= v;
   _s[2](1) -= v;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AABBox<T>::shrink( const Vec3<T>& v )
{
   _s[0](0) += v.x;
   _s[1](0) += v.y;
   _s[2](0) += v.z;
   _s[0](1) -= v.x;
   _s[1](1) -= v.y;
   _s[2](1) -= v.z;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline void
AABBox<T>::translate( const Vec3<T>& v )
{
   _s[0](0) += v.x;
   _s[1](0) += v.y;
   _s[2](0) += v.z;
   _s[0](1) += v.x;
   _s[1](1) += v.y;
   _s[2](1) += v.z;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec2<T>&
AABBox<T>::slabX() const
{
   return _s[0];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec2<T>&
AABBox<T>::slabY() const
{
   return _s[1];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec2<T>&
AABBox<T>::slabZ() const
{
   return _s[2];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Vec2<T>&
AABBox<T>::slab( uint axis ) const
{
   return _s[axis];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
AABBox<T>::slabX()
{
   return _s[0];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
AABBox<T>::slabY()
{
   return _s[1];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
AABBox<T>::slabZ()
{
   return _s[2];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec2<T>&
AABBox<T>::slab( uint axis )
{
   return _s[axis];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AABBox<T>::min( uint axis ) const
{
   return _s[axis](0);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AABBox<T>::max( uint axis ) const
{
   return _s[axis](1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
AABBox<T>::min( uint axis )
{
   return _s[axis](0);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T&
AABBox<T>::max( uint axis )
{
   return _s[axis](1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AABBox<T>::maxAbs( uint axis ) const
{
   return CGM::max( CGM::abs(_s[axis](0)), CGM::abs(_s[axis](1)) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AABBox<T>::maxAbs() const
{
   return CGM::max( maxAbs(0), CGM::max( maxAbs(1), maxAbs(2) ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
AABBox<T>::size() const
{
   return Vec3<T>( _s[0](1)-_s[0](0), _s[1](1)-_s[1](0), _s[2](1)-_s[2](0) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AABBox<T>::size( uint axis ) const
{
   return _s[axis](1) - _s[axis](0);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AABBox<T>::maxSize() const
{
   T xs = _s[0](1) - _s[0](0);
   T ys = _s[1](1) - _s[1](0);
   T zs = _s[2](1) - _s[2](0);
   return CGM::max( CGM::max( xs, ys ), zs );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AABBox<T>::surface() const
{
   T xs = _s[0](1) - _s[0](0);
   T ys = _s[1](1) - _s[1](0);
   T zs = _s[2](1) - _s[2](0);

   return (xs*ys + xs*zs + ys*zs)*T(2);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AABBox<T>::volume() const
{
   T xs = _s[0](1) - _s[0](0);
   T ys = _s[1](1) - _s[1](0);
   T zs = _s[2](1) - _s[2](0);

   return xs*ys*zs;
}

//------------------------------------------------------------------------------
//! Return the asked corner
template< typename T > inline Vec3<T>
AABBox<T>::corner( int x, int y, int z ) const
{
   return Vec3<T>( _s[0](x), _s[1](y), _s[2](z) );
}

//------------------------------------------------------------------------------
//! Return the asked corner
template< typename T > inline Vec3<T>
AABBox<T>::corner( int corner ) const
{
   return Vec3<T>(
      _s[0]( corner & 1 ),
      _s[1]( (corner>>1) & 1),
      _s[2]( (corner>>2) & 1 )
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
AABBox<T>::center() const
{
   return Vec3<T>(
      ( _s[0](0) + _s[0](1) ) * (T)0.5,
      ( _s[1](0) + _s[1](1) ) * (T)0.5,
      ( _s[2](0) + _s[2](1) ) * (T)0.5
   );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
AABBox<T>::center( uint axis ) const
{
   return ( _s[axis](0) + _s[axis](1) ) * (T)0.5;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
AABBox<T>::isEmpty() const
{
   return (_s[0](1) < _s[0](0)) || (_s[1](1) < _s[1](0)) || (_s[2](1) < _s[2](0));
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
AABBox<T>::isOverlapping( const AABBox<T>& box ) const
{
   if( _s[0](0) > box._s[0](1) || _s[0](1) < box._s[0](0) )
   {
      return false;
   }
   if( _s[1](0) > box._s[1](1) || _s[1](1) < box._s[1](0) )
   {
      return false;
   }
   if( _s[2](0) > box._s[2](1) || _s[2](1) < box._s[2](0) )
   {
      return false;
   }
   return true;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline bool
AABBox<T>::isOverlapping( const AABBox<T>& box, const Ref<T>& ref ) const
{
   // Computation of center and axis.
   Vec3<T> extentsA = size() * (T)0.5;
   Vec3<T> posA     = extentsA + corner(0,0,0);


   Vec3<T> extentsB = box.size() * (T)0.5;
   Vec3<T> axesB[3];
   ref.orientation().getAxes( axesB[0], axesB[1], axesB[2] );

   Vec3<T> posB = ref.position() +
      axesB[0]*(extentsB(0) + box.min(0)) +
      axesB[1]*(extentsB(1) + box.min(1)) +
      axesB[2]*(extentsB(2) + box.min(2));

   Vec3<T> t = posB-posA;

   Vec3<T> absB[3];
   absB[0] = Vec3<T>( CGM::abs( axesB[0].x ), CGM::abs( axesB[0].y ), CGM::abs( axesB[0].z ) );
   absB[1] = Vec3<T>( CGM::abs( axesB[1].x ), CGM::abs( axesB[1].y ), CGM::abs( axesB[1].z ) );
   absB[2] = Vec3<T>( CGM::abs( axesB[2].x ), CGM::abs( axesB[2].y ), CGM::abs( axesB[2].z ) );

   T ra, rb;

   // This box basis vectors.
   for( uint i = 0; i < 3; ++i )
   {
      ra = extentsA(i);
      rb = absB[0](i)*extentsB(0) + absB[1](i)*extentsB(1) + absB[2](i)*extentsB(2);

      if( CGM::abs(t(i)) > ra+rb )
      {
         return false;
      }
   }

   // Other box basis vectors.
   for( uint i = 0; i < 3; ++i )
   {
      ra = absB[i](0)*extentsA(0) + absB[i](1)*extentsA(1) + absB[i](2)*extentsA(2);
      rb = extentsB(i);

      if( CGM::abs(t.dot(axesB[i])) > ra+rb )
      {
         return false;
      }
   }

   // Cross product basis vectors.
   // Ax X Bx
   ra = absB[0](2)*extentsA(1) + absB[0](1)*extentsA(2);
   rb = absB[2](0)*extentsB(1) + absB[1](0)*extentsB(2);

   if( CGM::abs(t(2)*axesB[0](1) - t(1)*axesB[0](2)) > ra+rb )
   {
      return false;
   }

   // Ax X By
   ra = absB[1](2)*extentsA(1) + absB[1](1)*extentsA(2);
   rb = absB[2](0)*extentsB(0) + absB[0](0)*extentsB(2);

   if( CGM::abs(t(2)*axesB[1](1) - t(1)*axesB[1](2)) > ra+rb )
   {
      return false;
   }

   // Ax X Bz
   ra = absB[2](2)*extentsA(1) + absB[2](1)*extentsA(2);
   rb = absB[1](0)*extentsB(0) + absB[0](0)*extentsB(1);

   if( CGM::abs(t(2)*axesB[2](1) - t(1)*axesB[2](2)) > ra+rb )
   {
      return false;
   }

   // Ay X Bx
   ra = absB[0](2)*extentsA(0) + absB[0](0)*extentsA(2);
   rb = absB[2](1)*extentsB(1) + absB[1](1)*extentsB(2);

   if( CGM::abs(t(0)*axesB[0](2) - t(2)*axesB[0](0)) > ra+rb )
   {
      return false;
   }

   // Ay X By
   ra = absB[1](2)*extentsA(0) + absB[1](0)*extentsA(2);
   rb = absB[2](1)*extentsB(0) + absB[0](1)*extentsB(2);

   if( CGM::abs(t(0)*axesB[1](2) - t(2)*axesB[1](0)) > ra+rb )
   {
      return false;
   }

   // Ay X Bz
   ra = absB[2](2)*extentsA(0) + absB[2](0)*extentsA(2);
   rb = absB[1](1)*extentsB(0) + absB[0](1)*extentsB(1);

   if( CGM::abs(t(0)*axesB[2](2) - t(2)*axesB[2](0)) > ra+rb )
   {
      return false;
   }

   // Az X Bx
   ra = absB[0](1)*extentsA(0) + absB[0](0)*extentsA(1);
   rb = absB[2](2)*extentsB(1) + absB[1](2)*extentsB(2);

   if( CGM::abs(t(1)*axesB[0](0) - t(0)*axesB[0](1)) > ra+rb )
   {
      return false;
   }

   // Az X By
   ra = absB[1](1)*extentsA(0) + absB[1](0)*extentsA(1);
   rb = absB[2](2)*extentsB(0) + absB[0](2)*extentsB(2);

   if( CGM::abs(t(1)*axesB[1](0) - t(0)*axesB[1](1)) > ra+rb )
   {
      return false;
   }

   // Az X Bz
   ra = absB[2](1)*extentsA(0) + absB[2](0)*extentsA(1);
   rb = absB[1](2)*extentsB(0) + absB[0](2)*extentsB(1);

   if( CGM::abs(t(1)*axesB[2](0) - t(0)*axesB[2](1)) > ra+rb )
   {
      return false;
   }

   return true;
}

//------------------------------------------------------------------------------
//! Verifies if the specified point is inside the bounding box.
//! It uses a closed interval in every dimension (i.e. min <= v < max).
template< typename T > inline bool
AABBox<T>::isInside( const Vec3<T>& v ) const
{
   return _s[0](0) <= v(0) && v(0) <= _s[0](1) &&
          _s[1](0) <= v(1) && v(1) <= _s[1](1) &&
          _s[2](0) <= v(2) && v(2) <= _s[2](1);
}

//------------------------------------------------------------------------------
//! Verifies if the specified point is inside the bounding box.
//! It uses a half-open interval in every dimension (i.e. min <= v < max).
template< typename T > inline bool
AABBox<T>::isInsideCO( const Vec3<T>& v ) const
{
   return _s[0](0) <= v(0) && v(0) < _s[0](1) &&
          _s[1](0) <= v(1) && v(1) < _s[1](1) &&
          _s[2](0) <= v(2) && v(2) < _s[2](1);
}

//------------------------------------------------------------------------------
//! Verifies if the specified point is inside the bounding box.
//! It uses an open interval in every dimension (i.e. min < v < max).
template< typename T > inline bool
AABBox<T>::isInsideOO( const Vec3<T>& v ) const
{
   return _s[0](0) < v(0) && v(0) < _s[0](1) &&
          _s[1](0) < v(1) && v(1) < _s[1](1) &&
          _s[2](0) < v(2) && v(2) < _s[2](1);
}

//------------------------------------------------------------------------------
//! Verifies if the specified AABB is inside the bounding box.
//! It uses a closed interval in every dimension (i.e. min <= v <= max).
template< typename T > inline bool
AABBox<T>::isInside( const AABBox<T>& bb ) const
{
   return _s[0](0) <= bb.min(0) && bb.max(0) <= _s[0](1) &&
          _s[1](0) <= bb.min(1) && bb.max(1) <= _s[1](1) &&
          _s[2](0) <= bb.min(2) && bb.max(2) <= _s[2](1);
}

//------------------------------------------------------------------------------
//!
template< typename T > inline uint
AABBox<T>::longestSide() const
{
   T x = _s[0](1) - _s[0](0);
   T y = _s[1](1) - _s[1](0);
   T z = _s[2](1) - _s[2](0);

   if( x > y )
   {
      if( x > z )
      {
         return 0;
      }
      else
      {
         return 2;
      }
   }
   else
   {
      if( y > z )
      {
         return 1;
      }
      else
      {
         return 2;
      }
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > inline TextStream&
operator<<
( TextStream& stream, const AABBox<T>& box )
{
   return stream << box.corner(0) << " - " << box.corner(7);
}




/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef AABBox< int >    AABBoxi;
typedef AABBox< float >  AABBoxf;
typedef AABBox< double > AABBoxd;

NAMESPACE_END

#endif

