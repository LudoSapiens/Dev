/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <MotionBullet/Collision/BasicShapes.h>

#include <Base/Dbg/Defs.h>

#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btCylinderShape.h>
#include <BulletCollision/CollisionShapes/btConeShape.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btMultiSphereShape.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h>
#include <LinearMath/btVector3.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS SphereShape
==============================================================================*/

//------------------------------------------------------------------------------
//!
SphereShape::SphereShape( float r ) :
   CollisionShape( SPHERE )
{
   _btSphereShape = new btSphereShape( r );
}

//------------------------------------------------------------------------------
//!
SphereShape::~SphereShape()
{
   delete _btSphereShape;
}

//------------------------------------------------------------------------------
//!
float
SphereShape::radius() const
{
   return _btSphereShape->getRadius();
}

//------------------------------------------------------------------------------
//!
void
SphereShape::radius( float r )
{
   _btSphereShape->setUnscaledRadius( r );
}

//------------------------------------------------------------------------------
//!
btCollisionShape*
SphereShape::bulletCollisionShape()
{
   return _btSphereShape;
}


/*==============================================================================
   CLASS BoxShape
==============================================================================*/

BoxShape::BoxShape( float x, float y, float z ) :
   CollisionShape( BOX ),
   _size( x, y, z )
{
   _btBoxShape = new btBoxShape( btVector3(x, y, z) );
}

//------------------------------------------------------------------------------
//!
BoxShape::BoxShape( const Vec3f& s ) :
   CollisionShape( BOX ),
   _size( s )
{
   _btBoxShape = new btBoxShape( btVector3(s.x, s.y, s.z) );
}

//------------------------------------------------------------------------------
//!
BoxShape::~BoxShape()
{
   delete _btBoxShape;
}

//------------------------------------------------------------------------------
//!
btCollisionShape*
BoxShape::bulletCollisionShape()
{
   return _btBoxShape;
}

/*==============================================================================
   CLASS CylinderShape
==============================================================================*/

//------------------------------------------------------------------------------
//!
CylinderShape::CylinderShape( float r, float h ) :
   CollisionShape( CYLINDER ),
   _height( h )
{
   _btCylinderShape = new btCylinderShape( btVector3(r, h, r) );
}

//------------------------------------------------------------------------------
//!
CylinderShape::~CylinderShape()
{
   delete _btCylinderShape;
}

//------------------------------------------------------------------------------
//!
float
CylinderShape::radius() const
{
   return _btCylinderShape->getRadius();
}

//------------------------------------------------------------------------------
//!
btCollisionShape*
CylinderShape::bulletCollisionShape()
{
   return _btCylinderShape;
}

/*==============================================================================
   CLASS ConeShape
==============================================================================*/

//------------------------------------------------------------------------------
//!
ConeShape::ConeShape( float r, float h ) :
   CollisionShape( CONE )
{
   _btConeShape = new btConeShape( r, h );
}

//------------------------------------------------------------------------------
//!
ConeShape::~ConeShape()
{
   delete _btConeShape;
}

//------------------------------------------------------------------------------
//!
float
ConeShape::radius() const
{
   return _btConeShape->getRadius();
}

//------------------------------------------------------------------------------
//!
float
ConeShape::height() const
{
   return _btConeShape->getHeight();
}

//------------------------------------------------------------------------------
//!
btCollisionShape*
ConeShape::bulletCollisionShape()
{
   return _btConeShape;
}

/*==============================================================================
   CLASS ConvexHullShape
==============================================================================*/

ConvexHullShape::ConvexHullShape() :
   CollisionShape( CONVEXHULL )
{
   _btConvexHullShape = new btConvexHullShape();
   _btConvexHullShape->setMargin(0.01f);
}

//------------------------------------------------------------------------------
//!
ConvexHullShape::ConvexHullShape( const Vec3f* vertices, uint numVertices ) :
   CollisionShape( CONVEXHULL )
{
   _btConvexHullShape = new btConvexHullShape(
      (const btScalar*)vertices, numVertices, sizeof(Vec3f)
   );
   _btConvexHullShape->setMargin(0.01f);
}

//------------------------------------------------------------------------------
//!
ConvexHullShape::~ConvexHullShape()
{
   delete _btConvexHullShape;
}

//------------------------------------------------------------------------------
//!
void
ConvexHullShape::addVertex( const Vec3f& v )
{
   _btConvexHullShape->addPoint( btVector3( v.x, v.y, v.z ) );
}

//------------------------------------------------------------------------------
//!
void
ConvexHullShape::reserveVertices( uint /*qty*/ )
{
}

//------------------------------------------------------------------------------
//!
void
ConvexHullShape::clearVertices()
{
   delete _btConvexHullShape;
   _btConvexHullShape = new btConvexHullShape();
   _btConvexHullShape->setMargin(0.01f);
}

//------------------------------------------------------------------------------
//!
const Vec3f&
ConvexHullShape::vertex( uint id ) const
{
   return *(Vec3f*)&_btConvexHullShape->getUnscaledPoints()[id];
}

//------------------------------------------------------------------------------
//!
uint
ConvexHullShape::numVertices() const
{
   return _btConvexHullShape->getNumPoints();
}

//------------------------------------------------------------------------------
//!
btCollisionShape*
ConvexHullShape::bulletCollisionShape()
{
   return _btConvexHullShape;
}

/*==============================================================================
   CLASS SphereHullShape
==============================================================================*/

SphereHullShape::SphereHullShape() :
   CollisionShape( SPHEREHULL ),
   _btMultiSphereShape( NULL )
{
}

//------------------------------------------------------------------------------
//! SphereData is packed [X0,Y0,Z0,r0, X1,Y1,Z1,r1, ...]
SphereHullShape::SphereHullShape( const float* sphereData, const uint nSpheres ) :
   CollisionShape( SPHEREHULL ),
   _btMultiSphereShape( NULL )
{
   _spheres.resize( nSpheres );
   memcpy( _spheres.data(), sphereData, nSpheres*sizeof(_spheres[0]) );
}

//------------------------------------------------------------------------------
//!
SphereHullShape::~SphereHullShape()
{
   delete _btMultiSphereShape;
}

//------------------------------------------------------------------------------
//!
btCollisionShape*
SphereHullShape::bulletCollisionShape()
{
   if( _btMultiSphereShape == NULL )  convertSpheresToBullet();
   return _btMultiSphereShape;
}

//------------------------------------------------------------------------------
//!
void
SphereHullShape::convertSpheresToBullet()
{
   uint n = uint(_spheres.size());
   Vector<btVector3> p( n );
   Vector<btScalar>  r( n );
   for( uint i = 0; i < n; ++i )
   {
      const Vec4f& s = _spheres[i];
      p[i] = btVector3( s.x, s.y, s.z );
      r[i] = s.w;
   }
   //delete _btMultiSphereShape;
   CHECK( _btMultiSphereShape == NULL );
   _btMultiSphereShape = new btMultiSphereShape( p.data(), r.data(), n );
}


/*==============================================================================
   CLASS TriangleMeshShape
==============================================================================*/

//------------------------------------------------------------------------------
//!
TriangleMeshShape::TriangleMeshShape(
   uint numTriangles, const uint*  indices,  size_t iStride,
   uint numVertices,  const float* vertices, size_t vStride
):
   CollisionShape( CollisionShape::TRIMESH )
{
   btTriangleIndexVertexArray* interface = new btTriangleIndexVertexArray(
      numTriangles, (int*)indices, int(iStride),
      numVertices, (btScalar*)vertices, int(vStride)
   );
   _btBvhTriangleMeshShape = new btBvhTriangleMeshShape(interface, false);
}

//------------------------------------------------------------------------------
//!
TriangleMeshShape::~TriangleMeshShape()
{
}

//------------------------------------------------------------------------------
//!
void
TriangleMeshShape::setIndices( uint /*numTriangles*/, const uint* /*indices*/, size_t /*stride*/ )
{
}

//------------------------------------------------------------------------------
//!
void
TriangleMeshShape::setVertices( uint /*numVertices*/, const float* /*vertices*/, size_t /*stride*/ )
{
}

//------------------------------------------------------------------------------
//!
btCollisionShape*
TriangleMeshShape::bulletCollisionShape()
{
   return _btBvhTriangleMeshShape;
}


NAMESPACE_END
