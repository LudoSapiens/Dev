/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Physics/CollisionShapeGenerator.h>

#include <Plasma/Geometry/MetaGeometry.h>
#include <Plasma/Geometry/SurfaceGeometry.h>
#include <Plasma/Geometry/MeshGeometry.h>

#if MOTION_BULLET
#include <MotionBullet/Collision/BasicShapes.h>
#else
#include <Motion/Collision/BasicShapes.h>
#endif

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
CollisionShapeGenerator::CollisionShapeGenerator()
{
}

//------------------------------------------------------------------------------
//!
CollisionShapeGenerator::~CollisionShapeGenerator()
{
}

//------------------------------------------------------------------------------
//!
CollisionShape*
CollisionShapeGenerator::generate( int type, const Geometry& geom )
{
   switch( type )
   {
      case CollisionShape::GROUP     : return NULL;
      case CollisionShape::SPHERE    : return generateSphere( geom );
      case CollisionShape::BOX       : return generateBoxInGroup( geom );
      case CollisionShape::CYLINDER  : return NULL;
      case CollisionShape::CONE      : return NULL;
      case CollisionShape::CONVEXHULL: return generateConvexHull( geom );
      case CollisionShape::SPHEREHULL: return NULL;
      case CollisionShape::TRIMESH   : return generateTrimesh( geom );
      default                        : return NULL;
   }
}

//------------------------------------------------------------------------------
//!
CollisionShape*
CollisionShapeGenerator::generateSphere( const Geometry& geom )
{
   float best = 0.0f;

   // Compute sphere for MetaGeometry.
   MetaGeometry* mgeom = geom.metaGeometry();
   if( mgeom )
   {
      Vector<float> vtx;
      Vector<uint>  idx;
      mgeom->triangulate( vtx, idx );

      for( uint i = 0; i < vtx.size(); i += 5 )
      {
         Vec3f p = Vec3f( vtx[i], vtx[i+1], vtx[i+2] );
         float tmp = sqrLength( p );
         if( best < tmp )  best = tmp;
      }
   }
   // Compute sphere for Surface.
   SurfaceGeometry* sgeom = geom.surface();
   if( sgeom )
   {
      for( uint i = 0; i < sgeom->numVertices(); ++i )
      {
         float tmp = sqrLength( sgeom->vertex(i) );
         if( best < tmp )  best = tmp;
      }
   }
   // Compute sphere for mesh.
   MeshGeometry* mesh = geom.mesh();
   if( mesh )
   {
      for( uint i = 0; i < mesh->numVertices(); ++i )
      {
         float tmp = sqrLength( mesh->position(i) );
         if( best < tmp )  best = tmp;
      }
   }

   float r = CGM::sqrt( best );
   return new SphereShape( r );
}

//------------------------------------------------------------------------------
//!
CollisionShape*
CollisionShapeGenerator::generateBoxAtOrigin( const Geometry& geom )
{
   Vec3f best( 0.0f );

   // Compute box for MetaGeometry.
   MetaGeometry* mgeom = geom.metaGeometry();
   if( mgeom )
   {
      Vector<float> vtx;
      Vector<uint>  idx;
      mgeom->triangulate( vtx, idx );

      for( uint i = 0; i < vtx.size(); i += 5 )
      {
         Vec3f p = Vec3f( vtx[i], vtx[i+1], vtx[i+2] );
         best = CGM::max( best, CGM::abs(p) );
      }
   }
   // Compute box for Surface.
   SurfaceGeometry* sgeom = geom.surface();
   if( sgeom )
   {
      for( uint i = 0; i < sgeom->numVertices(); ++i )
      {
         best = CGM::max( best, CGM::abs( sgeom->vertex(i) ) );
      }
   }
   // Compute box for mesh.
   MeshGeometry* mesh = geom.mesh();
   if( mesh )
   {
      for( uint i = 0; i < mesh->numVertices(); ++i )
      {
         best = CGM::max( best, CGM::abs( mesh->position(i) ) );
      }
   }
   return new BoxShape( best );
}

//------------------------------------------------------------------------------
//!
CollisionShape*
CollisionShapeGenerator::generateBoxInGroup( const Geometry& geom )
{
   AABBoxf bbox = AABBoxf::empty();

   // Compute box for MetaGeometry.
   MetaGeometry* mgeom = geom.metaGeometry();
   if( mgeom )
   {
      Vector<float> vtx;
      Vector<uint>  idx;
      mgeom->triangulate( vtx, idx );
      for( uint i = 0; i < vtx.size(); i += 5 )
      {
         Vec3f p = Vec3f( vtx[i], vtx[i+1], vtx[i+2] );
         bbox |= p;
      }
   }
   // Compute box for Surface.
   SurfaceGeometry* sgeom = geom.surface();
   if( sgeom ) bbox = sgeom->boundingBox();

   // Compute box for mesh.
   MeshGeometry* mesh = geom.mesh();
   if( mesh ) bbox = mesh->boundingBox();

   // Compute collision shape.
   Vec3f center = bbox.center();
   Vec3f size_2 = bbox.size() * 0.5f;
   if( CGM::equal(center, Vec3f::zero()) )
   {
      return new BoxShape( size_2 );
   }
   else
   {
      CollisionGroup* grp = new CollisionGroup();
      grp->addShape( Reff(center), new BoxShape(size_2) );
      return grp;
   }
}

//------------------------------------------------------------------------------
//!
CollisionShape*
CollisionShapeGenerator::generateConvexHull( const Geometry& geom )
{
   ConvexHullShape* hull = new ConvexHullShape();

   // Compute convex hull for MetaGeometry.
   MetaGeometry* mgeom = geom.metaGeometry();
   if( mgeom )
   {
      Vector<float> vertices;
      Vector<uint> indices;
      mgeom->triangulate( vertices, indices );

      hull->reserveVertices( uint(vertices.size())/5 );

      // Compute centroid.
      Vec3f c(0.0f);
      for( uint i = 0; i < vertices.size(); i+=5 )
      {
         Vec3f v( vertices[i], vertices[i+1], vertices[i+2] );
         c += v;
         hull->addVertex( v );
      }
      c *= 5.0f;
      c /= float(vertices.size());

      StdErr << "# of vertices: " << vertices.size()/5.0f << "\n";

      /*
      // For each face of the healpix, find a sample.
      for( uint f = 0; f < 12; ++f )
      {
         Vec3f dir = HEALPix::pix2vec( 0, f, 0, 0 );
      }
      */
   }
   // Compute convex hull for Surface.
   SurfaceGeometry* sgeom = geom.surface();
   if( sgeom )
   {
      hull->reserveVertices( sgeom->numVertices() );

      // Compute centroid.
      Vec3f c(0.0f);
      for( uint i = 0; i < sgeom->numVertices(); ++i )
      {
         c += sgeom->vertex(i);
         hull->addVertex( sgeom->vertex(i) );
      }
      c /= float(sgeom->numVertices());

      StdErr << "# of vertices: " << sgeom->numVertices() << "\n";
   }
   // Compute convex hull for mesh.
   MeshGeometry* mesh = geom.mesh();
   if( mesh )
   {
      hull->reserveVertices( mesh->numVertices() );

      // Compute centroid.
      Vec3f c(0.0f);
      for( uint i = 0; i < mesh->numVertices(); ++i )
      {
         c += mesh->position(i);
         hull->addVertex( mesh->position(i) );
      }
      c /= float(mesh->numVertices());

      StdErr << "# of vertices: " << mesh->numVertices() << "\n";
   }
   return hull;
}

//------------------------------------------------------------------------------
//!
CollisionShape*
CollisionShapeGenerator::generateTrimesh( const Geometry& geom )
{
   MeshGeometry* mesh = geom.mesh();
   if( !mesh )
   {
      MetaGeometry* meta = geom.metaGeometry();
      if( meta )
      {
         RCP<MeshGeometry> mesh = new MeshGeometry();
         meta->createMesh( *mesh );
         return new TriangleMeshShape( mesh->numPrimitives(),
                                       mesh->indices(),
                                       3*sizeof(mesh->indices()[0]),
                                       mesh->numVertices(),
                                       mesh->vertices(),
                                       mesh->vertexStride()*sizeof(float) );
      }
   }
   if( mesh )
   {
      if( mesh->primitiveType() == MeshGeometry::TRIANGLES )
      {
         return new TriangleMeshShape( mesh->numPrimitives(),
                                       mesh->indices(),
                                       3*sizeof(mesh->indices()[0]),
                                       mesh->numVertices(),
                                       mesh->vertices(),
                                       mesh->vertexStride()*sizeof(float) );
      }
   }
   return nullptr;
}
