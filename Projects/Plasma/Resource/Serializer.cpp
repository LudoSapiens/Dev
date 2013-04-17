/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Resource/Serializer.h>

#include <Plasma/Geometry/Geometry.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Geometry/MetaGeometry.h>

#if MOTION_BULLET
#include <MotionBullet/Collision/BasicShapes.h>
#include <MotionBullet/Collision/CollisionGroup.h>
#else
#include <Motion/Collision/BasicShapes.h>
#include <Motion/Collision/CollisionGroup.h>
#endif

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//enum WorldCode
//{
//   CODE_WORLD_BRAIN,
//   CODE_WORLD_CAMERA,
//   CODE_WORLD_ENTITY,
//   CODE_WORLD_ENTITY_GROUP,
//   CODE_WORLD_LIGHT,
//   CODE_WORLD_MATERIAL,
//   CODE_WORLD_RECEPTOR,
//   CODE_WORLD_RIGID_ENTITY,
//   CODE_WORLD_SKELETAL_ENTITY,
//   CODE_WORLD_SOUND,
//   CODE_WORLD_VIEWPORT,
//};


UNNAMESPACE_END


NAMESPACE_BEGIN

//=================
// OUTPUT ROUTINES
//=================

template< typename T > inline BinaryStream&
operator<<( BinaryStream& os, const Vec2<T>& v )
{
   return os << v.x << v.y;
}

template< typename T > inline BinaryStream&
operator<<( BinaryStream& os, const Vec3<T>& v )
{
   return os << v.x << v.y << v.z;
}

template< typename T > inline BinaryStream&
operator<<( BinaryStream& os, const Vec4<T>& v )
{
   return os << v.x << v.y << v.z << v.w;
}

template< typename T > inline BinaryStream&
operator<<( BinaryStream& os, const Quat<T>& v )
{
   return os << v.x() << v.y() << v.z() << v.w();
}

template< typename T > inline BinaryStream&
operator<<( BinaryStream& os, const Ref<T>& v )
{
   return os << v.orientation() << v.position() << v.scale();
}


//================
// INPUT ROUTINES
//================

template< typename T > inline BinaryStream&
operator>>( BinaryStream& is, Vec2<T>& v )
{
   return is >> v.x >> v.y;
}

template< typename T > inline BinaryStream&
operator>>( BinaryStream& is, Vec3<T>& v )
{
   return is >> v.x >> v.y >> v.z;
}

template< typename T > inline BinaryStream&
operator>>( BinaryStream& is, Vec4<T>& v )
{
   return is >> v.x >> v.y >> v.z >> v.w;
}

template< typename T > inline BinaryStream&
operator>>( BinaryStream& is, Quat<T>& v )
{
   return is >> v.x() >> v.y() >> v.z() >> v.w();
}

template< typename T > inline BinaryStream&
operator>>( BinaryStream& is, Ref<T>& v )
{
   return is >> v.orientation() >> v.position() >> v.scale();
}

namespace Serializer
{

//------------------------------------------------------------------------------
//!
bool  dumpGeometry( const Geometry& geom, BinaryStream& os )
{
   // Retreived mesh geometry.
   MeshGeometry* mesh = NULL;
   switch( geom.type() )
   {
      case Geometry::MESH:
         mesh = geom.mesh();
         break;
      case Geometry::METAGEOMETRY:
         mesh = new MeshGeometry();
         geom.metaGeometry()->createMesh( *mesh );
         break;
      default:
         return false;
   }

   if( mesh )
   {
      os << (uint8_t)mesh->type();
      return dumpMeshGeometry( *mesh, os );
   }
   else
   {
      return false;
   }
}

//------------------------------------------------------------------------------
//!
bool  dumpMeshGeometry( const MeshGeometry& geom, BinaryStream& os )
{
   bool ok = true;

   // 1. Dump primitive type.
   os << (uint8_t)geom.primitiveType();

   // 2. Dump index buffer.
   // 2.1. Dump number of indices.
   uint32_t nIndices = geom.numIndices();
   os << nIndices;
   // 2.2. Dump the indices with minimal number of bits for each (8, 16, or 32).
   const uint32_t* indices = geom.indices();
   if( nIndices < (1<<8) )
   {
      // Dumping using uint8_t for every index.
      for( uint i = 0; i < nIndices; ++i )
      {
         os << (uint8_t)indices[i];
      }
   }
   else
   if( nIndices < (1<<16) )
   {
      // Dumping using uint16_t for every index.
      for( uint i = 0; i < nIndices; ++i )
      {
         os << (uint16_t)indices[i];
      }
   }
   else
   {
      // Dumping using uint32_t for every index.
      for( uint i = 0; i < nIndices; ++i )
      {
         os << (uint32_t)indices[i];
      }
   }

   // 3. Dump vertex buffer (only one for now).
   // 3.1. Attributes list.
   // 3.1.1. Number of attributes.
   os << (uint8_t)geom.numAttributes();
   // 3.1.2. The attribute codes.
   for( uint i = 0; i < geom.numAttributes(); ++i )
   {
      os << (uint8_t)geom.attributeType( i );
   }
   // 3.2. Vertex data.
   // 3.2.2. Number of vertices (different from vSize).
   os << (uint32_t)geom.numVertices();
   // 3.2.3. Vertex floats.
   const size_t vSize = geom.verticesSize();
   const float* vData = geom.vertices();
   for( uint i = 0; i < vSize; ++i )
   {
      os << (float)vData[i];
   }

   // 4. Dump the patches.
   ok &= dumpPatches( geom, os );

   // 5. Dump the collision shape (if any).
   ok &= dumpCollisionShape( geom, os );

   return ok && os.ok();
}

//------------------------------------------------------------------------------
//! Dumps the patches of a geometry into a binary stream.
bool  dumpPatches( const Geometry& geom, BinaryStream& os )
{
   // 1. Dump the number of patches.
   uint n = geom.numPatches();
   os << (uint32_t)n;

   // 2. Dump the patches.
   for( uint i = 0; i < n; ++i )
   {
      const Geometry::PatchInfo& patchInfo = geom.patchInfo( i );
      os << (uint32_t)patchInfo.rangeStart();
      os << (uint32_t)patchInfo.rangeSize();
      os << (uint32_t)patchInfo.materialID();
   }

   return os.ok();
}

//------------------------------------------------------------------------------
//! Dumps the collision shape of a geometry into a binary stream.
bool  dumpCollisionShape( const Geometry& geom, BinaryStream& os )
{
   const CollisionShape* shape = geom.collisionShape();
   if( shape != NULL )
   {
      os << (uint8_t)1; // Flag to indicate there is a collision shape.
      return dumpCollisionShape( *shape, os );
   }
   else
   {
      os << (uint8_t)0; // Flag to indicate there is no collision shape.
      return os.ok();
   }
}

//------------------------------------------------------------------------------
//!
bool  dumpCollisionShape( const CollisionShape& shape, BinaryStream& os )
{
   os << (uint8_t)shape.type();
   switch( shape.type() )
   {
      case CollisionShape::GROUP:
      {
         const CollisionGroup* group = (const CollisionGroup*)&shape;
         uint32_t n = group->numShapes();
         os << n;
         for( uint i = 0; i < n; ++i )
         {
            const Reff ref              = group->referential( i );
            const CollisionShape* shape = group->shape( i );
            os << ref;
            CHECK( shape != NULL );
            dumpCollisionShape( *shape, os );
         }
      }  break;
      case CollisionShape::SPHERE:
      {
         const SphereShape* sphere = (const SphereShape*)&shape;
         os << sphere->radius();
      }  break;
      case CollisionShape::BOX:
      {
         const BoxShape* box = (const BoxShape*)&shape;
         os << box->size();
      }  break;
      case CollisionShape::CYLINDER:
      {
         const CylinderShape* cyl = (const CylinderShape*)&shape;
         os << cyl->radius() << cyl->height();
      }  break;
      case CollisionShape::CONE:
      {
         const ConeShape* cone = (const ConeShape*)&shape;
         os << cone->radius() << cone->height();
      }  break;
      case CollisionShape::CONVEXHULL:
      {
         const ConvexHullShape* hull = (const ConvexHullShape*)&shape;
         uint32_t n = hull->numVertices();
         os << n;
         for( uint i = 0; i < n; ++i )
         {
            os << hull->vertex( i );
         }
      }  break;
      case CollisionShape::SPHEREHULL:
      {
         const SphereHullShape* hull = (const SphereHullShape*)&shape;
         uint32_t n = hull->numSpheres();
         os << n;
         for( uint i = 0; i < n; ++i )
         {
            os << hull->sphere( i );
         }
      }  break;
      //case CollisionShape::TRIMESH:
      //{
      //   
      //}  break;
      default:
      {
         StdErr << "Serializer::dumpCollisionShape() - Unsupported shape type: " << shape.type() << nl;
         return false;
      }
   }
   return os.ok();
}

//------------------------------------------------------------------------------
//!
bool  loadGeometry( BinaryStream& is, Resource<Geometry>* res )
{
   uint8_t type;
   is >> type;
   if( is.ok() )
   {
      switch( type )
      {
         //case Geometry::SURFACE     : return loadSurfaceGeometry( is );
         //case Geometry::METAGEOMETRY: return loadMetaGeometry( is );
         case Geometry::MESH        : return loadMeshGeometry( is, res );
         //case Geometry::SILHOUETTE  : return laodSilhouetteGeometry( is );
         default: break;
      }
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool  loadMeshGeometry( BinaryStream& is, Resource<Geometry>* res )
{
   // Create geometry mesh.
   RCP<MeshGeometry> geom( new MeshGeometry() );

   // 1. Load primitive type.
   uint8_t type;
   is >> type;
   geom->primitiveType( type );

   // 2. Load index buffer.
   // 2.1. Load number of indices.
   uint32_t nIndices;
   is >> nIndices;
   // 2.2. Load the indices (careful with the number of bits for each).
   geom->allocateIndices( nIndices );
   uint32_t* indices = const_cast<uint32_t*>( geom->indices() );
   if( nIndices < (1<<8) )
   {
      uint8_t idx;
      for( uint i = 0; i < nIndices; ++i )
      {
         is >> idx;
         indices[i] = idx;
      }
   }
   else
   if( nIndices < (1<<16) )
   {
      uint16_t idx;
      for( uint i = 0; i < nIndices; ++i )
      {
         is >> idx;
         indices[i] = idx;
      }
   }
   else
   {
      // Storing in place (since it's 32b).
      for( uint i = 0; i < nIndices; ++i )
      {
         is >> indices[i];
      }
   }

   // 3. Load vertex buffer (only one for now).
   // 3.1. Attributes list.
   // 3.1.1. Number of attributes.
   uint8_t nAttributes;
   is >> nAttributes;
   // 3.1.2. The attribute codes.
   int* attrList = new int[nAttributes+1];
   uint8_t attrType;
   for( uint i = 0; i < nAttributes; ++i )
   {
      is >> attrType;
      attrList[i] = attrType;
   }
   attrList[nAttributes] = 0;
   geom->setAttributes( attrList );
   delete [] attrList;
   // 3.2. Vertex data.
   // 3.2.2. Number of vertices.
   uint32_t nVerts;
   is >> nVerts;
   // 3.2.3. Vertex floats.
   geom->allocateVertices( nVerts );
   float* vData = const_cast<float*>( geom->vertices() );
   size_t vSize = geom->verticesSize();
   for( uint i = 0; i < vSize; ++i )
   {
      is >> vData[i];
   }

   // 4. Load the patches.
   bool ok = loadPatches( is, *geom );

   // 5. Update the bounding box and other properties.
   geom->updateProperties();

   // 6. Load the collision shape (if any).
   ok &= loadCollisionShape( is, *geom );

   res->data( geom.ptr() );

   return ok && is.ok();
}

//------------------------------------------------------------------------------
//! Loads the patches from a binary stream into a MeshGeometry.
bool  loadPatches( BinaryStream& is, MeshGeometry& geom )
{
   // 1. Load the number of patches.
   uint32_t n;
   is >> n;

   // 2. Load the patches.
   uint32_t start, size, mat;
   for( uint32_t i = 0; i < n; ++i )
   {
      is >> start >> size >> mat;
      geom.addPatch( start, size, mat );
   }

   return is.ok();
}

//------------------------------------------------------------------------------
//! Loads the collision shape from a binary stream into a Geometry.
bool  loadCollisionShape( BinaryStream& is, Geometry& geom )
{
   uint8_t flag;
   is >> flag;
   if( flag == 0 )
   {
      // No CollisionShape present.
      geom.collisionShape( NULL ); // Kill any existing one.
      return is.ok();
   }
   else
   if( flag == 1 )
   {
      RCP<CollisionShape> shape = loadCollisionShape( is );
      geom.collisionShape( shape.ptr() );
      return shape.isValid() && is.ok();
   }
   else
   {
      StdErr << "ERROR - Serializer::loadCollisionShape() - Invalid flag: " << flag << "." << nl;
      return false;
   }
}

//------------------------------------------------------------------------------
//!
RCP<CollisionShape>  loadCollisionShape( BinaryStream& is )
{
   uint8_t type;
   is >> type;
   switch( type )
   {
      case CollisionShape::GROUP:
      {
         RCP<CollisionGroup> group = new CollisionGroup();
         uint32_t n;
         is >> n;
         for( uint i = 0; i < n; ++i )
         {
            Reff ref;
            is >> ref;
            RCP<CollisionShape> shape = loadCollisionShape( is );
            if( shape.isValid() )
            {
               group->addShape( ref, shape.ptr() );
            }
            else
            {
               StdErr << "ERROR - Serializer::loadShape() - Could not read collision shape " << i << " of " << n << " in a collision group." << nl;
               return NULL;
            }
         }
         return group;
      }  break;
      case CollisionShape::SPHERE:
      {
         float radius;
         is >> radius;
         return new SphereShape( radius );
      }  break;
      case CollisionShape::BOX:
      {
         Vec3f size;
         is >> size;
         return new BoxShape( size );
      }  break;
      case CollisionShape::CYLINDER:
      {
         float radius, height;
         is >> radius >> height;
         return new CylinderShape( radius, height );
      }  break;
      case CollisionShape::CONE:
      {
         float radius, height;
         is >> radius >> height;
         return new ConeShape( radius, height );
      }  break;
      case CollisionShape::CONVEXHULL:
      {
         RCP<ConvexHullShape> hull = new ConvexHullShape();
         uint32_t n;
         is >> n;
         hull->reserveVertices( n );
         Vec3f v;
         for( uint i = 0; i < n; ++i )
         {
            is >> v;
            hull->addVertex( v );
         }
         return hull;
      }  break;
      case CollisionShape::SPHEREHULL:
      {
         RCP<SphereHullShape> hull = new SphereHullShape();
         uint32_t n;
         is >> n;
         hull->reserveSpheres( n );
         Vec4f v;
         for( uint i = 0; i < n; ++i )
         {
            is >> v;
            hull->addSphere( v );
         }
         return hull;
      }  break;
      //case CollisionShape::TRIMESH:
      //{
      //   
      //}  break;
      default:
      {
         StdErr << "Serializer::loadShape() - Unsupported shape type: " << type << nl;
      }
   }
   return NULL;
}

} //namespace Serializer

NAMESPACE_END
