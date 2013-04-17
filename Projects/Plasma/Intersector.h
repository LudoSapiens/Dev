/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_INTERSECTOR_H
#define PLASMA_INTERSECTOR_H

#include <Plasma/StdDefs.h>
#include <Plasma/Procedural/Boundary.h>

#include <CGMath/AABBox.h>
#include <CGMath/Ray.h>
#include <CGMath/Plane.h>

NAMESPACE_BEGIN

class Entity;
class World;
class Geometry;

/*==============================================================================
  CLASS Intersector
==============================================================================*/

//!

class PLASMA_DLL_API Intersector
{

public:

   /*----- classes -----*/

   struct Hit
   {
      /*----- methods -----*/
      Hit():
         _pos(0.0f), _normal(1.0f,0.0f,0.0f), _barypos(0.0f), 
         _t( CGConstf::infinity() ), _tmin(0.0f),
         _entity(0), _geom(0), _fid(0)
      {}

      /*----- data members -----*/

      Vec3f     _pos;
      Vec3f     _normal;
      Vec2f     _barypos;
      float     _t;
      float     _tmin;
      Entity*   _entity;
      Geometry* _geom;
      union {
         uint        _fid;
         const void* _face;
      };
   };
 
   /*----- static methods -----*/

   // Ray-World intersection.
   static bool trace(
      World*       world,
      const Rayf&  ray,
      Hit&         hit
   );
   // Test for intersection between ]0,1] of the ray.
   static bool trace(
      World*       world,
      const Rayf&  ray
   );
   
   // Ray-Entity intersection.
   static bool trace(
      Entity*      entity,
      const Rayf&  ray,
      Hit&         hit
   );
   static bool trace(
      Entity*      entity,
      const Rayf&  ray
   );
      
   // Ray-geometry intersection.
   static bool trace(
      Geometry*   geom,
      const Rayf& ray,
      Hit&        hit
   );
   static bool trace(
      Geometry*   geom,
      const Rayf& ray
   );
   static bool trace(
      Geometry*              geom,
      const Vector< Mat4f >& matrices,
      const Rayf&            ray,
      Hit&                   hit
   );
   static bool trace(
      Geometry*              geom,
      const Vector< Mat4f >& matrices,
      const Rayf&            ray
   );

   // Ray-sphere.
   static bool trace(
      const Vec3f& center,
      const float  radius,
      const Rayf&  ray,
      float&       distance
   );
   // Ray-sphere intersection returning 0, 1, or 2 intersections.
   static uint trace(
      const Vec3f& center,
      const float  radius,
      const Rayf&  ray,
      float&       d1,
      float&       d2
   );

   // Ray-triangle intersection.
   static bool trace(
      const Vec3f& p0,
      const Vec3f& p1,
      const Vec3f& p2,
      const Rayf&  ray,
      Hit&         hit
   );
   static bool getBarycentric(
      const Vec3f& p0,
      const Vec3f& p1,
      const Vec3f& p2,
      const Rayf&  ray,
      Hit&         hit
   );

   // Ray-plane.
   static bool trace(
      const Planef& plane,
      const Rayf&   ray,
      float&        distance
   );

   // Ray-bounding box intersection.
   static bool trace(
      const AABBoxf& bb,
      const Rayf&    ray,
      float&         minDistance,
      float&         maxDistance
   );

   // Ray-Boundary intersection.
   static bool trace(
      const Boundary& boundary,
      const Rayf&     ray,
      float&          distance
   );
   
   // Triangle - Triangle intersection.
   static bool triTriIntersect(
      const Vec3f& v0,
      const Vec3f& v1,
      const Vec3f& v2,
      const Vec3f& u0,
      const Vec3f& u1,
      const Vec3f& u2
   );
};

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const Intersector::Hit& hit )
{
   return os << "["
             << "t=" << hit._t << " tmin=" << hit._tmin << " pos=" << hit._pos
             << " normal=" << hit._normal << " bary=" << hit._barypos
             << "]";
}

NAMESPACE_END

#endif
