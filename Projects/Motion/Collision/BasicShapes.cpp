/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Motion/Collision/BasicShapes.h>

#include <Base/Dbg/Defs.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS SphereShape
==============================================================================*/

//------------------------------------------------------------------------------
//!
SphereShape::SphereShape( float r ) :
   CollisionShape( SPHERE ),
   _radius( r )
{
   _margin = r;
   _bbox.set( r );
}

//------------------------------------------------------------------------------
//!
SphereShape::~SphereShape()
{
}

//------------------------------------------------------------------------------
//!
Vec3f
SphereShape::getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const
{
   return ref.position();
}

/*==============================================================================
   CLASS BoxShape
==============================================================================*/

BoxShape::BoxShape( float x, float y, float z ) :
   CollisionShape( BOX ),
   _size( x, y, z )
{
   updateMargin();
   _bbox.set( -x, x, -y, y, -z, z );
}

//------------------------------------------------------------------------------
//!
BoxShape::BoxShape( const Vec3f& s ) :
   CollisionShape( BOX ),
   _size( s )
{
   updateMargin();
   _bbox.set( -s.x, s.x, -s.y, s.y, -s.z, s.z );
}

//------------------------------------------------------------------------------
//!
BoxShape::~BoxShape()
{
}

//------------------------------------------------------------------------------
//!
void
BoxShape::updateMargin()
{
   float min = _size.min();
   _margin   = min > 0.2f ? 0.02f : min * 0.1f;
}

//------------------------------------------------------------------------------
//!
Vec3f
BoxShape::getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const
{
   // Transform 'dir' in object-space.
   Quatf q = ref.orientation().getInversed();
   Vec3f localDir = q * dir;
   
   // Figure out the corner based on signs alone.
   Vec3f pt = _size - _margin;
   if( localDir(0) < 0 ) pt(0) = -pt(0);
   if( localDir(1) < 0 ) pt(1) = -pt(1);
   if( localDir(2) < 0 ) pt(2) = -pt(2);
   
   // Transform that corner back into world-space.
   return ref.orientation() * pt + ref.position();
}

/*==============================================================================
   CLASS CylinderShape
==============================================================================*/

//------------------------------------------------------------------------------
//!
CylinderShape::CylinderShape( float r, float h ) :
   CollisionShape( CYLINDER ),
   _radius( r ),
   _height( h )
{
   _margin = 0.02f;
   _bbox.set( -r, r, -_height, _height, -r, r );
}

//------------------------------------------------------------------------------
//!
CylinderShape::~CylinderShape()
{
}

//------------------------------------------------------------------------------
//!
Vec3f
CylinderShape::getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const
{
   // Transform 'dir' in object-space.
   Quatf q = ref.orientation().getInversed();
   Vec3f localDir = q * dir;
   
   Vec3f pt;
   
   float sqrLength = localDir.x*localDir.x + localDir.z*localDir.z;
   if( sqrLength > 0.0f )
   {
      float s = _radius / CGM::sqrt( sqrLength );
      pt.x = localDir.x * s;
      pt.y = localDir.y < 0.0f ? -_height : _height;
      pt.z = localDir.z * s;
   }
   else
   {
      pt.x = 0.0f;
      pt.y = localDir.y < 0.0f ? -_height : _height;
      pt.z = 0.0f;
   }   
   
   // Transform back into world-space.
   return ref.orientation() * pt + ref.position();
}

/*==============================================================================
   CLASS ConeShape
==============================================================================*/

//------------------------------------------------------------------------------
//!
ConeShape::ConeShape( float r, float h ) :
   CollisionShape( CONE ),
   _radius( r ),
   _height( h )
{
   updateSinApex();
   _margin = 0.02f;
   _bbox.set( -r, r, -_height*0.5f, _height*0.5f, -r, r );
}

//------------------------------------------------------------------------------
//!
ConeShape::~ConeShape()
{
}

//------------------------------------------------------------------------------
//!
Vec3f
ConeShape::getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const
{
   // Transform 'dir' in object-space.
   Quatf q = ref.orientation().getInversed();
   Vec3f localDir = q * dir;
   
   Vec3f pt;
   
   if( localDir.y >= _sinApex*localDir.length() )
   {
      pt.x = 0.0f;
      pt.y = _height*0.5f;
      pt.z = 0.0f;
   }
   else
   {
      float sqrLength = localDir.x*localDir.x + localDir.z*localDir.z;
      if( sqrLength > 0.0f )
      {
         float s = _radius / CGM::sqrt( sqrLength );
         pt.x = localDir.x * s;
         pt.y = -_height*0.5f;
         pt.z = localDir.z * s;
      }
      else
      {
         pt.x = 0.0f;
         pt.y = -_height*0.5f;
         pt.z = 0.0f;
      }
   }
   
   // Transform back into world-space.
   return ref.orientation() * pt + ref.position();
}

/*==============================================================================
   CLASS ConvexHullShape
==============================================================================*/

ConvexHullShape::ConvexHullShape() :
   CollisionShape( CONVEXHULL )
{
   _margin = 0.02f;
}

//------------------------------------------------------------------------------
//!
ConvexHullShape::~ConvexHullShape()
{
}

//------------------------------------------------------------------------------
//!
Vec3f
ConvexHullShape::getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const
{
   CHECK( dir.sqrLength() > 0.0f );
   
   // Transform 'dir' in object-space.
   Quatf q = ref.orientation().getInversed();
   Vec3f localDir = q * dir;
   
   // Figure out the farthest vertex along localDir.
   float maxLength = -CGConstf::infinity();
   uint maxVertex = 0;
   for( uint i = 0; i < _vertices.size(); ++i )
   {
      float length = localDir.dot( _vertices[i] );
      if( length > maxLength )
      {
         maxLength = length;
         maxVertex = i;
      }
   }
   
   // Transform that vertex back into world-space.
   return ref.orientation() * _vertices[maxVertex] +
          ref.position();
}

/*==============================================================================
   CLASS SphereHullShape
==============================================================================*/

SphereHullShape::SphereHullShape() :
   CollisionShape( SPHEREHULL )
{
   _margin = CGConstf::max();
}

//------------------------------------------------------------------------------
//! SphereData is packed [X0,Y0,Z0,r0, X1,Y1,Z1,r1, ...]
SphereHullShape::SphereHullShape( const float* sphereData, const uint nSpheres ) :
   CollisionShape( SPHEREHULL ),
   _spheres( nSpheres )
{
   memcpy( _spheres.data(), sphereData, _spheres.dataSize() );
   //const float* cur = sphereData;
   //for( uint i = 0; i < nSpheres; ++i )
   //{
   //   _spheres[i] = Vec4f( cur[0], cur[1], cur[2], cur[3] );
   //   cur += 4;
   //}
}

//------------------------------------------------------------------------------
//!
SphereHullShape::~SphereHullShape()
{
}

//------------------------------------------------------------------------------
//!
Vec3f
SphereHullShape::getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const
{
   CHECK( dir.sqrLength() > 0.0f );
   
   // Transform 'dir' in object-space.
   Quatf q = ref.orientation().getInversed();
   Vec3f localDir = q * dir;
   localDir.normalize();
   
   Vec3f pt;
   
   // Figure out the farthest sphere along localDir.
   float maxLength = -CGConstf::infinity();
   for( uint i = 0; i < _spheres.size(); ++i )
   {
      Vec3f curPt = (const Vec3f&)_spheres[i] + localDir*(_spheres[i].w-_margin);
      float length = localDir.dot( curPt );
      if( length > maxLength )
      {
         maxLength = length;
         pt = curPt;
      }
   }
   
   // Transform that vertex back into world-space.
   return ref.orientation() * pt + ref.position();
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
   setIndices( numTriangles, indices, iStride );
   setVertices( numVertices, vertices, vStride );
}

//------------------------------------------------------------------------------
//!
TriangleMeshShape::~TriangleMeshShape()
{
}

//------------------------------------------------------------------------------
//!
void
TriangleMeshShape::setIndices( uint numTriangles, const uint* indices, size_t stride )
{
}

//------------------------------------------------------------------------------
//!
void
TriangleMeshShape::setVertices( uint numVertices, const float* vertices, size_t stride )
{
}

//------------------------------------------------------------------------------
//!
Vec3f
TriangleMeshShape::getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const
{
   return Vec3f(0.0f);
}

NAMESPACE_END
