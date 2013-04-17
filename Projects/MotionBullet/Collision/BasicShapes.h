/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTIONBULLET_BASICSHAPES_H
#define MOTIONBULLET_BASICSHAPES_H

#include <MotionBullet/StdDefs.h>
#include <MotionBullet/Collision/CollisionShape.h>

#include <CGMath/Vec3.h>
#include <CGMath/Vec4.h>

#include <Base/ADT/Vector.h>
#include <Base/Dbg/Defs.h>

class btSphereShape;
class btBoxShape;
class btCylinderShape;
class btConeShape;
class btConvexHullShape;
class btMultiSphereShape;
class btBvhTriangleMeshShape;

NAMESPACE_BEGIN

/*==============================================================================
   CLASS SphereShape
==============================================================================*/
//!
class SphereShape
   : public CollisionShape
{

public:

   /*----- methods -----*/

   MOTION_DLL_API SphereShape( float r );

   // Attributes.
   MOTION_DLL_API float radius() const;
   MOTION_DLL_API void radius( float r );

protected:

   /*----- methods -----*/

   virtual ~SphereShape();

   virtual btCollisionShape*  bulletCollisionShape();

   /*----- data members -----*/

   btSphereShape*  _btSphereShape;
};

/*==============================================================================
   CLASS BoxShape
==============================================================================*/
//!
class BoxShape
   : public CollisionShape
{

public:

   /*----- methods -----*/

   MOTION_DLL_API BoxShape( float x, float y, float z );
   MOTION_DLL_API BoxShape( const Vec3f& s );

   // Attributes.
   inline const Vec3f& size() const;

protected:

   /*----- methods -----*/

   virtual ~BoxShape();

   virtual btCollisionShape*  bulletCollisionShape();

   /*----- data members -----*/

   btBoxShape*  _btBoxShape;
   Vec3f        _size;
};

//------------------------------------------------------------------------------
//!
inline const Vec3f&
BoxShape::size() const
{
   return _size;
}

/*==============================================================================
   CLASS CylinderShape
==============================================================================*/
//! Radius is in plane XZ.
class CylinderShape
   : public CollisionShape
{

public:

   /*----- methods -----*/

   MOTION_DLL_API CylinderShape( float r, float height );

   // Attributes.
   MOTION_DLL_API float radius() const;
   inline float height() const;

protected:

   /*----- methods -----*/

   virtual ~CylinderShape();

   virtual btCollisionShape*  bulletCollisionShape();

   /*----- data members -----*/

   btCylinderShape*  _btCylinderShape;
   float             _height;
};

//------------------------------------------------------------------------------
//!
inline float
CylinderShape::height() const
{
   return _height;
}

/*==============================================================================
   CLASS ConeShape
==============================================================================*/
//! Tip at origin, height towards +Y axis.
class ConeShape
   : public CollisionShape
{

public:

   /*----- methods -----*/

   MOTION_DLL_API ConeShape( float r, float h );

   // Attributes.
   MOTION_DLL_API float radius() const;
   MOTION_DLL_API float height() const;

protected:

   /*----- methods -----*/

   virtual ~ConeShape();

   virtual btCollisionShape*  bulletCollisionShape();

   /*----- data members -----*/

   btConeShape*  _btConeShape;
};

/*==============================================================================
   CLASS ConvexHullShape
==============================================================================*/
//!
class ConvexHullShape
   : public CollisionShape
{

public:

   /*----- methods -----*/

   MOTION_DLL_API ConvexHullShape();
   MOTION_DLL_API ConvexHullShape( const Vec3f*, uint numVertices );

   // Building.
   MOTION_DLL_API void addVertex( const Vec3f& );
   MOTION_DLL_API void reserveVertices( uint qty );
   MOTION_DLL_API void clearVertices();

   // Accessors.
   MOTION_DLL_API const Vec3f& vertex( uint id ) const;
   MOTION_DLL_API uint numVertices() const;

protected:

   /*----- methods -----*/

   virtual ~ConvexHullShape();

   virtual btCollisionShape*  bulletCollisionShape();

   /*----- data members -----*/

   btConvexHullShape*  _btConvexHullShape;
};

/*==============================================================================
   CLASS SphereHullShape
==============================================================================*/
//!
class SphereHullShape
   : public CollisionShape
{

public:

   /*----- methods -----*/

   MOTION_DLL_API SphereHullShape();
   MOTION_DLL_API SphereHullShape( const float* sphereData, uint nSpheres );

   // Building.
   inline void addSphere( const Vec3f&, float radius );
   inline void addSphere( const Vec4f& );
   inline void reserveSpheres( uint qty );
   inline void clearSpheres();

   // Accessors.
   inline const Vec4f& sphere( uint id ) const;
   inline uint numSpheres() const;

protected:

   /*----- methods -----*/

   virtual ~SphereHullShape();

   virtual btCollisionShape*  bulletCollisionShape();
   MOTION_DLL_API void  convertSpheresToBullet();

   /*----- data members -----*/

   btMultiSphereShape*  _btMultiSphereShape;
   Vector<Vec4f>        _spheres;
};

//------------------------------------------------------------------------------
//!
inline void
SphereHullShape::addSphere( const Vec3f& pt, float radius )
{
   _spheres.pushBack( Vec4f( pt, radius ) );
   CHECK( _btMultiSphereShape == NULL );
}

//------------------------------------------------------------------------------
//!
inline void
SphereHullShape::addSphere( const Vec4f& sphere )
{
   _spheres.pushBack( sphere );
   CHECK( _btMultiSphereShape == NULL );
}

//------------------------------------------------------------------------------
//!
inline void
SphereHullShape::reserveSpheres( uint qty )
{
   _spheres.reserve( qty );
}

//------------------------------------------------------------------------------
//!
inline void
SphereHullShape::clearSpheres()
{
   _spheres.clear();
   CHECK( _btMultiSphereShape == NULL );
}

//------------------------------------------------------------------------------
//!
inline const Vec4f&
SphereHullShape::sphere( uint id ) const
{
   return _spheres[id];
}

//------------------------------------------------------------------------------
//!
inline uint
SphereHullShape::numSpheres() const
{
   return (uint)_spheres.size();
}

/*==============================================================================
  CLASS TriangleMeshShape
==============================================================================*/
class TriangleMeshShape:
   public CollisionShape
{
public:

   /*----- methods -----*/

   MOTION_DLL_API TriangleMeshShape(
      uint numTriangles, const uint*  indices,  size_t iStride,
      uint numVertices,  const float* vertices, size_t vStride
   );

   MOTION_DLL_API void  setIndices( uint numTriangles, const uint* indices, size_t stride );
   MOTION_DLL_API void  setVertices( uint numVertices, const float* vertices, size_t stride );

protected:

   /*----- data members -----*/

   btBvhTriangleMeshShape*  _btBvhTriangleMeshShape;

   /*----- methods -----*/

   virtual ~TriangleMeshShape();

   virtual btCollisionShape*  bulletCollisionShape();

private:
}; //class TriangleMeshShape

NAMESPACE_END

#endif
