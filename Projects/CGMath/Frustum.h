/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_FRUSTUM_H
#define CGMATH_FRUSTUM_H

#include <CGMath/StdDefs.h>

#include <CGMath/AABBox.h>
#include <CGMath/CGMath.h>
#include <CGMath/Plane.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Frustum
==============================================================================*/

template < class T >
class Frustum
{

public:

   /*----- types and enumerations ----*/

   enum {
      LEFT,
      RIGHT,
      BOTTOM,
      TOP,
      FRONT,
      BACK
   };

   enum {
      OUTSIDE,
      INSIDE,
      INTERSECT
   };

   /*----- methods -----*/

   // Constructors/destructor.
   Frustum() {}
   Frustum( const Plane<T> planes[6] );
   Frustum( const Ref<T>&, T front, T back, T fovx, T fovy );
   Frustum( const Ref<T>&, T front, T back, T fovx, T fovy, T shearX, T shearY );

   ~Frustum() {}

   // Accessors.
   const Plane<T>& plane( uint i ) const;

   // Return one of the eight corner forming the frustum.
   Vec3<T> corner( int ) const;

   // Intersection tests.
   // These tests could return INTERSECT even if the true result is OUTSIDE.
   int intersect( const AABBox<T>& ) const;
   int intersect( const AABBox<T>&, const Ref<T>& )  const;

   // Return the distance of a bounding box from a plane of the frustum.
   T distance( const AABBox<T>&, int plane )  const;
   T distance( const AABBox<T>&, const Ref<T>&, int plane )  const;

private:

   /*----- data members -----*/

   Plane<T> _planes[6];  //!< The 6 planes determining the frustum's volume (facing out).
};

//------------------------------------------------------------------------------
//!
template< typename T > inline
Frustum<T>::Frustum( const Plane<T> planes[6] )
{
      _planes[0] = planes[0];
      _planes[1] = planes[1];
      _planes[2] = planes[2];
      _planes[3] = planes[3];
      _planes[4] = planes[4];
      _planes[5] = planes[5];
}

//------------------------------------------------------------------------------
//! Defines a frustum along the referential's +Z axis.
template< typename T > inline
Frustum<T>::Frustum( const Ref<T>& ref, T front, T back, T fovx_2, T fovy_2 )
{
   Vec3<T> ax, ay, az;
   ref.orientation().getAxes( ax, ay, az );
   ax = -ax;
   az = -az;
   _planes[FRONT] = Plane<T>( -az, ref.position() + az*front );
   _planes[BACK]  = Plane<T>(  az, ref.position() + az*back );

   T siny = CGM::sin(fovy_2);
   T cosy = CGM::cos(fovy_2);

   _planes[TOP]    = Plane<T>(  cosy*ay-siny*az, ref.position() );
   _planes[BOTTOM] = Plane<T>( -cosy*ay-siny*az, ref.position() );

   T sinx = CGM::sin(fovx_2);
   T cosx = CGM::cos(fovx_2);

   _planes[LEFT]   = Plane<T>(  cosx*ax-sinx*az, ref.position() );
   _planes[RIGHT]  = Plane<T>( -cosx*ax-sinx*az, ref.position() );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline
Frustum<T>::Frustum( const Ref<T>& ref, T front, T back, T fovx_2, T fovy_2, T shearX, T shearY )
{
   CHECK( shearX == 0 && shearY == 0 ); // Need to determine an efficient way to support those.

   Vec3<T> ax, ay, az;
   ref.orientation().getAxes( ax, ay, az );
   ax = -ax;
   az = -az;
   _planes[FRONT] = Plane<T>( -az, ref.position() + az*front );
   _planes[BACK]  = Plane<T>(  az, ref.position() + az*back );

   T siny = CGM::sin(fovy_2);
   T cosy = CGM::cos(fovy_2);

   _planes[TOP]    = Plane<T>(  cosy*ay-siny*az, ref.position() );
   _planes[BOTTOM] = Plane<T>( -cosy*ay-siny*az, ref.position() );

   T sinx = CGM::sin(fovx_2);
   T cosx = CGM::cos(fovx_2);

   _planes[LEFT]   = Plane<T>(  cosx*ax-sinx*az, ref.position() );
   _planes[RIGHT]  = Plane<T>( -cosx*ax-sinx*az, ref.position() );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline const Plane<T>&
Frustum<T>::plane( uint i ) const
{
   return _planes[i];
}

//------------------------------------------------------------------------------
//!
template< typename T > inline Vec3<T>
Frustum<T>::corner( int c ) const
{
   const Plane<T>& p0 = _planes[c&1];
   const Plane<T>& p1 = _planes[((c&2)>>1)+2];
   const Plane<T>& p2 = _planes[((c&4)>>2)+4];
   return intersectPlanes( p0, p1, p2 );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline int
Frustum<T>::intersect( const AABBox<T>& box ) const
{
   int result = INSIDE;
   for( uint i = 0; i < 6; ++i )
   {
      // Find nearest point.
      const Vec3<T>& dir = _planes[i].direction();
      int x = dir(0) >= 0.0f ? 0 : 1;
      int y = dir(1) >= 0.0f ? 0 : 1;
      int z = dir(2) >= 0.0f ? 0 : 1;

      if( _planes[i].inFront( box.corner( x, y, z ) ) )
      {
         return OUTSIDE;
      }

      // Find farthest point.
      if( _planes[i].inFront( box.corner( 1-x, 1-y, 1-z ) ) )
      {
         result = INTERSECT;
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline int
Frustum<T>::intersect( const AABBox<T>& box, const Ref<T>& ref ) const
{
   // Get axes.
   Vec3<T> ax, ay, az;
   ref.orientation().getAxes( ax, ay, az );

   int result = INSIDE;
   for( uint i = 0; i < 6; ++i )
   {
      // Find nearest point.
      const Vec3<T>& dir = _planes[i].direction();
      int x = dir.dot( ax ) >= 0.0f ? 0 : 1;
      int y = dir.dot( ay ) >= 0.0f ? 0 : 1;
      int z = dir.dot( az ) >= 0.0f ? 0 : 1;

      Vec3<T> pt;
      pt = box.corner( x, y, z );
      pt = pt(0)*ax + pt(1)*ay + pt(2)*az + ref.position();

      if( _planes[i].inFront( pt ) )
      {
         return OUTSIDE;
      }

      // Find farthest point.
      pt = box.corner( 1-x, 1-y, 1-z );
      pt = pt(0)*ax + pt(1)*ay + pt(2)*az + ref.position();
      if( _planes[i].inFront( pt ) )
      {
         result = INTERSECT;
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Frustum<T>::distance( const AABBox<T>& box, int plane ) const
{
   // Find nearest point.
   const Vec3<T>& dir = _planes[plane].direction();
   int x = dir(0) >= 0.0f ? 0 : 1;
   int y = dir(1) >= 0.0f ? 0 : 1;
   int z = dir(2) >= 0.0f ? 0 : 1;

   return _planes[plane].distance( box.corner( x, y, z ) );
}

//------------------------------------------------------------------------------
//!
template< typename T > inline T
Frustum<T>::distance( const AABBox<T>& box, const Ref<T>& ref, int plane ) const
{
   Vec3<T> ax, ay, az;
   ref.orientation().getAxes( ax, ay, az );

   // Find nearest point.
   const Vec3<T>& dir = _planes[plane].direction();
   int x = dir.dot( ax ) >= 0.0f ? 0 : 1;
   int y = dir.dot( ay ) >= 0.0f ? 0 : 1;
   int z = dir.dot( az ) >= 0.0f ? 0 : 1;

   Vec3<T> pt;
   pt = box.corner( x, y, z );
   pt = pt(0)*ax + pt(1)*ay + pt(2)*az + ref.position();

   return _planes[plane].distance( pt );
}

/*==============================================================================
  TYPEDEF
==============================================================================*/

typedef Frustum< float >  Frustumf;
typedef Frustum< double > Frustumd;

NAMESPACE_END

#endif
