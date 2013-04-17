/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Procedural/ProceduralMesh.h>
#include <Plasma/Procedural/ProceduralMaterial.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Physics/CollisionShapeGenerator.h>

#if MOTION_BULLET
#include <MotionBullet/Collision/BasicShapes.h>
#include <MotionBullet/Collision/CollisionGroup.h>
#else
#include <Motion/Collision/BasicShapes.h>
#include <Motion/Collision/CollisionGroup.h>
#endif

#include <Fusion/Resource/ResManager.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/Core/Core.h>

#include <CGMath/Range.h>

#include <Base/ADT/StringMap.h>
#include <Base/IO/FileSystem.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

/*==============================================================================
   Procedural API/functions.
==============================================================================*/

//------------------------------------------------------------------------------
//!
void meshVM( VMState* vm, MeshGeometry* mesh )
{
   // Only accept a table as argument.
   if( !VM::isTable( vm, 1 ) )
   {
      StdErr << "Missing arguments to mesh()." << nl;
      return;
   }

   // Indices.
   Vector< uint32_t >  indices;

   if( VM::get( vm, 1, "indices" ) )
   {
      uint s = VM::getTableSize( vm, -1 );
      indices.resize( s );
      for( uint i = 0; i < s; ++i )
      {
         VM::geti( vm, -1, i+1 );
         indices[i] = VM::toUInt( vm, -1 ) - 1; // Consistent with Lua's position table.
         VM::pop( vm );
      }
      VM::pop( vm );
   }

   uint numPrims = uint(indices.size())/mesh->primitiveSize();
   if( indices.size() != numPrims*mesh->primitiveSize() )
   {
      StdErr << "Warning - Mesh ends with an incomplete primitive." << nl;
   }
   if( indices.empty() )
   {
      StdErr << "Mesh has no indices." << nl;
      return;
   }

   // Indices ranges.
   if( VM::get( vm, 1, "ranges" ) )
   {
      uint startIdx = 0;
      for( int i = 1; VM::geti( vm, -1, i ); ++i )
      {
         uint idxSize = VM::toUInt( vm, -1 );
         mesh->addPatch( startIdx, idxSize, i-1 );
         startIdx += idxSize;
         VM::pop(vm);
      }
   }
   else
   {
      mesh->addPatch( 0, uint(indices.size()) );
   }

   // Materials IDs.
   if( VM::get( vm, 1, "materials" ) )
   {
      for( int i = 1; VM::geti( vm, -1, i ); ++i )
      {
         uint matID = VM::toUInt( vm, -1 );
         mesh->materialID( i-1, matID-1 );
         VM::pop(vm);
      }
   }

   // Vertices.
   if( VM::get( vm, 1, "vertices" ) )
   {
      // AOS format, e.g.:
      //   trimesh{ indices=i, vertices={ format={ POSITION, NORMAL }, ... } }
      // 1. Read vertex buffer.
      Vector< float >  vertices;
      uint s = VM::getTableSize( vm, -1 );
      vertices.resize( s );
      for( uint i = 0; i < s; ++i )
      {
         VM::geti( vm, -1, i+1 );
         vertices[i] = VM::toFloat( vm, -1 );
         VM::pop( vm );
      }
      // 2. Read vertex specification.
      Vector< int >  attr( 1, MeshGeometry::INVALID );
      if( VM::get( vm, -1, "format" ) )
      {
         uint s = VM::getTableSize( vm, -1 );
         if( s == 0 )
         {
            StdErr << "ERROR - Empty format specification in mesh vertices table." << nl;
            return;
         }
         attr.resize( s+1, MeshGeometry::INVALID );
         for( uint i = 0; i < s; ++i )
         {
            VM::geti( vm, -1, i+1 );
            attr[i] = VM::toUInt( vm, -1 );
            VM::pop( vm );
         }
         VM::pop( vm ); // Popping 'format' table.
      }
      else
      {
         StdErr << "ERROR - Missing format specification in mesh vertices table." << nl;
         return;
      }
      VM::pop( vm ); // Popping 'vertices' table.
      // 3. Set the data.
      mesh->allocateIndices( uint(indices.size()) );
      mesh->copyIndices( indices.data() );
      mesh->setAttributes( attr.data() );
      mesh->allocateVertices( uint(vertices.size() / mesh->vertexStride()) );
      mesh->copyAttributes( vertices.data(), uint(mesh->vertexStride()), mesh->vertexStride(), 0 );
   }
   else
   {
      // SOA format.
      Vector< float >  mappings;
      Vector< float >  normals;
      Vector< float >  positions;
      if( VM::get( vm, 1, "mappings" ) )
      {
         uint s = VM::getTableSize( vm, -1 );
         mappings.resize( s );
         for( uint i = 0; i < s; ++i )
         {
            VM::geti( vm, -1, i+1 );
            mappings[i] = VM::toFloat( vm, -1 );
            VM::pop( vm );
         }
         VM::pop( vm );
      }

      if( VM::get( vm, 1, "normals" ) )
      {
         uint s = VM::getTableSize( vm, -1 );
         normals.resize( s );
         for( uint i = 0; i < s; ++i )
         {
            VM::geti( vm, -1, i+1 );
            normals[i] = VM::toFloat( vm, -1 );
            VM::pop( vm );
         }
         VM::pop( vm );
      }

      if( VM::get( vm, 1, "positions" ) )
      {
         uint s = VM::getTableSize( vm, -1 );
         positions.resize( s );
         for( uint i = 0; i < s; ++i )
         {
            VM::geti( vm, -1, i+1 );
            positions[i] = VM::toFloat( vm, -1 );
            VM::pop( vm );
         }
         VM::pop( vm );
      }

      if( positions.empty() )
      {
         StdErr << "Mesh has no positions." << nl;
         return;
      }

      mesh->allocateIndices( uint(indices.size()) );
      mesh->copyIndices( indices.data() );

      uint8_t bitCode = 0;
      bitCode |= normals.empty() ? 0 : 1;
      bitCode <<= 1;
      bitCode |= mappings.empty() ? 0 : 1;
      switch( bitCode )
      {
         case 0x00:
         {
            // Positions only.
            const int attr[] = {
               MeshGeometry::POSITION,
               MeshGeometry::INVALID
            };
            mesh->setAttributes( attr );
            mesh->allocateVertices( uint(positions.size() / mesh->vertexStride()) );
            mesh->copyAttributes( positions.data(), 3, 3, 0 );
         }  break;
         case 0x01:
         {
            // Positions and mappings.
            Rangei range = int(positions.size());
            range |= int(mappings.size());
            if( range.size() != 0 )
            {
               StdErr << "Warning - Different number of positions (" << positions.empty() << ") and mappings (" << mappings.empty() << "); using " << range.min() << "." << nl;
            }
            const int attr[] = {
               MeshGeometry::POSITION,
               MeshGeometry::MAPPING,
               MeshGeometry::INVALID
            };
            mesh->setAttributes( attr );
            mesh->allocateVertices( uint(range.min()) / uint(mesh->vertexStride()) );
            mesh->copyAttributes( positions.data(), 3, 3, 0 );
            mesh->copyAttributes( mappings.data() , 2, 2, 3 );
         }  break;
         case 0x02:
         {
            // Positions and normals.
            Rangei range = int(positions.size());
            range |= int(normals.size());
            if( range.size() != 0 )
            {
               StdErr << "Warning - Different number of positions (" << positions.empty() << ") and normals (" << normals.empty() << "); using " << range.min() << "." << nl;
            }
            const int attr[] = {
               MeshGeometry::POSITION,
               MeshGeometry::NORMAL,
               MeshGeometry::INVALID
            };
            mesh->setAttributes( attr );
            mesh->allocateVertices( uint(range.min()) / uint(mesh->vertexStride()) );
            mesh->copyAttributes( positions.data(), 3, 3, 0 );
            mesh->copyAttributes( normals.data()  , 3, 3, 3 );
         }  break;
         case 0x03:
         {
            // Positions, mappings and normals.
            Rangei range = int(positions.size());
            range |= int(normals.size());
            range |= int(mappings.size());
            if( range.size() != 0 )
            {
               StdErr << "Warning - Different number of positions (" << positions.empty() << "), normals (" << normals.empty() << "), and mappings (" << mappings.size() << "); using " << range.min() << "." << nl;
            }
            const int attr[] = {
               MeshGeometry::POSITION,
               MeshGeometry::NORMAL,
               MeshGeometry::MAPPING,
               MeshGeometry::INVALID
            };
            mesh->setAttributes( attr );
            mesh->allocateVertices( uint(range.min()) / uint(mesh->vertexStride()) );
            mesh->copyAttributes( positions.data(), 3, 3, 0 );
            mesh->copyAttributes( normals.data()  , 3, 3, 3 );
            mesh->copyAttributes( mappings.data() , 2, 3, 6 );
         }  break;
         default:
            break;
      }
   }
   mesh->updateProperties();
}

//------------------------------------------------------------------------------
//!
int linemeshVM( VMState* vm )
{
   ProceduralMesh* userData = (ProceduralMesh*)VM::userData( vm );
   MeshGeometry* mesh       = userData->geometry();
   mesh->primitiveType( MeshGeometry::LINES );

   meshVM( vm, mesh );
   return 0;
}

//------------------------------------------------------------------------------
//!
int pointmeshVM( VMState* vm )
{
   ProceduralMesh* userData = (ProceduralMesh*)VM::userData( vm );
   MeshGeometry* mesh       = userData->geometry();
   mesh->primitiveType( MeshGeometry::POINTS );

   meshVM( vm, mesh );
   return 0;
}

//------------------------------------------------------------------------------
//!
int trimeshVM( VMState* vm )
{
   ProceduralMesh* userData = (ProceduralMesh*)VM::userData( vm );
   MeshGeometry* mesh       = userData->geometry();
   mesh->primitiveType( MeshGeometry::TRIANGLES );

   meshVM( vm, mesh );
   return 0;
}

//------------------------------------------------------------------------------
//!
StringMap _collision_strToType(
   "group"     , CollisionShape::GROUP     ,
   "sphere"    , CollisionShape::SPHERE    ,
   "box"       , CollisionShape::BOX       ,
   "cylinder"  , CollisionShape::CYLINDER  ,
   "cone"      , CollisionShape::CONE      ,
   "hull"      , CollisionShape::CONVEXHULL,
   "spheres"   , CollisionShape::SPHEREHULL,
   "trimesh"   , CollisionShape::TRIMESH   ,
   ""
);

//------------------------------------------------------------------------------
//!
CollisionGroup* collisionGroupFromTable( VMState* vm, ProceduralMesh& userData )
{
   // [ ..., table ]

   int n = VM::getTableSize( vm, -1 );
   CollisionGroup* group = new CollisionGroup( n );

   Reff ref = Reff::identity();
   VM::get( vm, -1, "referential", ref );

   userData.addShape( group, ref );

   // Iterate over all entries of the table.
   for( int i = 1; VM::geti( vm, -1, i ); ++i )
   {
      // [ ..., table, table[i] ]
      CollisionShape* shape;

      if( VM::isTable( vm, -1 ) )
      {
         // [ ..., table, otherTable ]
         shape = collisionGroupFromTable( vm, userData );
      }
      else
      {
         // [ ..., table, shape ]
         shape = (CollisionShape*)VM::toPtr( vm, -1 );
      }

      group->addShape( userData.shapeReferential(shape), shape );

      VM::pop( vm, 1 );
      // [ ..., table ]
   }

   return group;
}

//------------------------------------------------------------------------------
//!
int collisionVM( VMState* vm )
{
   ProceduralMesh* userData = (ProceduralMesh*)VM::userData( vm );

   if( VM::getTop( vm ) != 1 )
   {
      StdErr << "Procedural mesh collision() call takes a single argument." << nl;
   }

   switch( VM::type( vm, 1 ) )
   {
      case VM::STRING:
         // Auto-collision mode.
         userData->autoCollision( _collision_strToType[VM::toCString(vm, 1)] );
         break;
      case VM::PTR:
         // Assign from light userdata.
         userData->lastCollision( (CollisionShape*)VM::toPtr( vm, 1 ) );
         break;
      case VM::TABLE:
         // Create a group from a series of shapes.
         userData->lastCollision( collisionGroupFromTable( vm, *userData ) );
         break;
      default:
         break;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int collisionBoxVM( VMState* vm )
{
   ProceduralMesh* userData = (ProceduralMesh*)VM::userData( vm );

   Vec3f size(1.0f);
   VM::get( vm, -1, "size", size );

   Reff ref = Reff::identity();
   VM::get( vm, -1, "referential", ref );

   CollisionShape* shape = new BoxShape(size);
   userData->addShape( shape, ref );

   VM::push( vm, (void*)shape );

   return 1;
}

//------------------------------------------------------------------------------
//!
int collisionConeVM( VMState* vm )
{
   ProceduralMesh* userData = (ProceduralMesh*)VM::userData( vm );

   float radius = 1.0f;
   float height = 1.0f;
   VM::get( vm, -1, "radius", radius );
   VM::get( vm, -1, "height", height );

   Reff ref = Reff::identity();
   VM::get( vm, -1, "referential", ref );

   CollisionShape* shape = new ConeShape( radius, height );
   userData->addShape( shape, ref );

   VM::push( vm, (void*)shape );

   return 1;
}

//------------------------------------------------------------------------------
//!
int collisionCylinderVM( VMState* vm )
{
   ProceduralMesh* userData = (ProceduralMesh*)VM::userData( vm );

   float radius = 1.0f;
   float height = 1.0f;
   VM::get( vm, -1, "radius", radius );
   VM::get( vm, -1, "height", height );

   Reff ref = Reff::identity();
   VM::get( vm, -1, "referential", ref );

   CollisionShape* shape = new CylinderShape( radius, height );
   userData->addShape( shape, ref );

   VM::push( vm, (void*)shape );

   return 1;
}

//------------------------------------------------------------------------------
//!
int collisionHullVM( VMState* vm )
{
   ProceduralMesh* userData = (ProceduralMesh*)VM::userData( vm );

   int size = VM::getTableSize( vm, -1 );

   ConvexHullShape* shape( new ConvexHullShape() );
   shape->reserveVertices( size );

   for( int i = 1; VM::geti( vm, -1, i ); ++i )
   {
      shape->addVertex( VM::toVec3f( vm, -1 ) );
      VM::pop( vm, 1 );
   }

   Reff ref = Reff::identity();
   VM::get( vm, -1, "referential", ref );

   userData->addShape( shape, ref );

   VM::push( vm, (void*)shape );

   return 1;
}

//------------------------------------------------------------------------------
//!
int collisionSphereVM( VMState* vm )
{
   ProceduralMesh* userData = (ProceduralMesh*)VM::userData( vm );

   float radius = 1.0f;
   VM::get( vm, -1, "radius", radius );

   Reff ref = Reff::identity();
   VM::get( vm, -1, "referential", ref );

   CollisionShape* shape = new SphereShape( radius );
   userData->addShape( shape, ref );

   VM::push( vm, (void*)shape );

   return 1;
}

//------------------------------------------------------------------------------
//!
int collisionSpheresVM( VMState* vm )
{
   ProceduralMesh* userData = (ProceduralMesh*)VM::userData( vm );

   int size = VM::getTableSize( vm, -1 );

   SphereHullShape* shape( new SphereHullShape() );
   shape->reserveSpheres( size );

   for( int i = 1; VM::geti( vm, -1, i ); ++i )
   {
      Vec4f sphere = VM::toVec4f( vm, -1 );
      shape->addSphere( sphere );
      VM::pop( vm, 1 );
   }

   Reff ref = Reff::identity();
   VM::get( vm, -1, "referential", ref );

   userData->addShape( shape, ref );

   VM::push( vm, (void*)shape );

   return 1;
}

//------------------------------------------------------------------------------
//!
const VM::Reg funcs[] = {
   // Instantiation.
   { "linemesh",          linemeshVM          },
   { "pointmesh",         pointmeshVM         },
   { "trimesh",           trimeshVM           },
   // Collision shapes.
   { "collision",         collisionVM         },
   { "collisionBox",      collisionBoxVM      },
   { "collisionCone",     collisionConeVM     },
   { "collisionCylinder", collisionCylinderVM },
   { "collisionHull",     collisionHullVM     },
   { "collisionSphere",   collisionSphereVM   },
   { "collisionSpheres",  collisionSpheresVM  },
   { 0,0 }
};

const VM::EnumReg _attrEnums[] = {
   { "INVALID",    MeshGeometry::INVALID   },
   { "POSITION",   MeshGeometry::POSITION  },
   { "NORMAL",     MeshGeometry::NORMAL    },
   { "MAPPING",    MeshGeometry::MAPPING   },
   { "GENERIC_1",  MeshGeometry::GENERIC_1 },
   { "GENERIC_2",  MeshGeometry::GENERIC_2 },
   { "GENERIC_3",  MeshGeometry::GENERIC_3 },
   { "GENERIC_4",  MeshGeometry::GENERIC_4 },
   { 0, 0 }
};

//------------------------------------------------------------------------------
//!
void initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "_G", funcs );
   VM::registerEnum( vm, "Attribute", _attrEnums );
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS ProceduralMesh
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
ProceduralMesh::initialize()
{
   VMRegistry::add( initVM, VM_CAT_MESH );
}

//------------------------------------------------------------------------------
//!
ProceduralMesh::ProceduralMesh( Resource<Geometry>* res, const String& path ):
   _res( res ),
   _path( path ),
   _lastCollision( NULL ),
   _autoCollision( StringMap::INVALID )
{
}

//------------------------------------------------------------------------------
//!
ProceduralMesh::ProceduralMesh( Resource<Geometry>* res, const String& path, const Table& params ):
   _res( res ),
   _params( &params ),
   _path( path ),
   _lastCollision( NULL ),
   _autoCollision( StringMap::INVALID )
{
}

//------------------------------------------------------------------------------
//!
void
ProceduralMesh::execute()
{
   // Prepare to build geometry.
   VMState* vm = VM::open( VM_CAT_MESH | VM_CAT_MATH, true );
   VM::userData( vm, this );

   _geom = new MeshGeometry();

   // Execute the script, potentially with parameters.
   if( _params.isValid() )
   {
      VM::push( vm, *_params );
      VM::doFile( vm, _path, 1, 0 );
   }
   else
   {
      VM::doFile( vm, _path, 0 );
   }

   // Collision geometry.
   if( _autoCollision != StringMap::INVALID )
   {
      CollisionShapeGenerator gen;
      _geom->collisionShape( gen.generate(_autoCollision, *_geom) );
   }
   else
   if( _lastCollision != NULL )
   {
      const Reff& ref = shapeReferential( _lastCollision );
      if( ref == Reff::identity() )
      {
         _geom->collisionShape( _lastCollision );
      }
      else
      {
         // Need to wrap in a collision group.
         CollisionGroup* group = new CollisionGroup( 1 );
         group->addShape( ref, _lastCollision );
         _geom->collisionShape( group );
      }
   }

   VM::close( vm );

   _res->data( _geom.ptr() );
}

NAMESPACE_END
