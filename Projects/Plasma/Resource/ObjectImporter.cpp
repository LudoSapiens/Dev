/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Resource/ObjectImporter.h>

#include <Plasma/Procedural/Surface.h>
#include <Plasma/World/Light.h>
#include <Plasma/World/Material.h>
#include <Plasma/World/StaticEntity.h>

#include <Base/ADT/Map.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/IO/BinaryStream.h>
#include <Base/IO/FileDevice.h>

#include <iostream>

USING_NAMESPACE


/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_oi, "ObjectImporter" );

#define MAIN_CHUNK             0x4D4D
#define EDIT3D_CHUNK           0x3D3D
#define OBJECT_CHUNK           0x4000
#define TRIMESH_CHUNK          0x4100
#define VERTICES_CHUNK         0x4110
#define FACES_CHUNK            0x4120
#define TEXCOORDS_CHUNK        0x4140
#define SMOOTHING_CHUNK        0x4150
#define REF_CHUNK              0x4160
#define MAT_CHUNK              0xAFFF
#define MAT_NAME_CHUNK         0xA000
#define MAT_DIFFUSE_CHUNK      0xA020
#define MAT_SPECULAR_CHUNK     0xA030
#define MAT_SHININESS_CHUNK    0xA040
#define MATGROUP_CHUNK         0x4130
#define LIGHT_CHUNK            0x4600
#define COLOR_CHUNK            0x0010
#define LIGHT_MULTIPLIER_CHUNK 0x465B
#define FLOAT_PERCENTAGE       0x0031

//------------------------------------------------------------------------------
//!
template< typename T > void read( BinaryStream& is, T& data )
{
   is.read( (char*)&data, sizeof(T) );
}

void read( BinaryStream& is, String& str )
{
   char c;
   is >> c;
   while( c != 0 )
   {
      str += c;
      is >> c;
   }
}

//------------------------------------------------------------------------------
//!
inline void swap8( unsigned char& a, unsigned char& b )
{
   unsigned char tmp = a;
   a = b;
   b = tmp;
}

//------------------------------------------------------------------------------
//!
inline void byteSwap32( unsigned char* ptr )
{
   swap8( ptr[0], ptr[3] );
   swap8( ptr[1], ptr[2] );
}

//------------------------------------------------------------------------------
//!
void swap( Vec3f& v )
{
   byteSwap32( (unsigned char*)&v(0) );
   byteSwap32( (unsigned char*)&v(1) );
   byteSwap32( (unsigned char*)&v(2) );
}

//------------------------------------------------------------------------------
//!
void swap( Vec3i& v )
{
   byteSwap32( (unsigned char*)&v(0) );
   byteSwap32( (unsigned char*)&v(1) );
   byteSwap32( (unsigned char*)&v(2) );
}


/*==============================================================================
  CLASS TriMesh
==============================================================================*/

//! Simple triangle mesh for parsing.

class TriMesh
   : public RCObject
{

public:

   /*----- classes -----*/

   struct Face
   {
      uint _indices[3];
      uint _normals[3];
      uint _wedges[3];
      unsigned short _flag;
   };

   class TMSurface
      : public RCObject
   {
   public:

      String       _matName;
      Vector<uint> _faces;
   };


   /*----- types and enumerations ----*/

   enum {
      NULL_NORMAL = 0xffff,
      NULL_WEDGE  = 0xffff,
      NULL_EDGE   = 0xffffffff
   };

   /*----- methods -----*/

   TriMesh() :
      _referential( Reff::identity() )
   {}

   void computeSurface();
   void computeAdjacencies();
   void computeNormals();
   RCP<StaticEntity> generateObject( const Vector< RCP<Material> >& mats );
   void add( const RCP<TriMesh>& mesh );

   /*----- data members -----*/

   Reff                   _referential;
   Vector< Vec3f >        _vertices;
   Vector< Vec3f >        _normals;
   Vector< Vec2f >        _texCoords;
   Vector< int >          _smoothingGroups;
   Vector< Face >         _faces;
   Vector< RCP<TMSurface> > _surfaces;
   Vector< uint > _verticesAdj;
   Vector< uint > _edgesRings;

protected:

   /*----- methods -----*/

   virtual ~TriMesh() {}

   void updateEdges(
      unsigned short vID,
      unsigned short vDest,
      uint edge
   );

};

//------------------------------------------------------------------------------
//!
void TriMesh::computeSurface()
{
   if( !_surfaces.empty() )
   {
      return;
   }

   RCP<TMSurface> surface( new TMSurface() );
   _surfaces.pushBack( surface );
   for( unsigned short s = 0; s < _faces.size(); ++s )
   {
      surface->_faces.pushBack( s );
   }

}

//------------------------------------------------------------------------------
//!
void TriMesh::computeAdjacencies()
{
   if( !_edgesRings.empty() )
   {
      return;
   }
   _edgesRings.resize( _faces.size() * 3 );
   _verticesAdj.resize( _vertices.size() );

   // Empty adjacencies.
   std::fill(
      _verticesAdj.begin(),
      _verticesAdj.end(),
      (uint)NULL_EDGE
   );

   // For each face, add adjacencies information.
   Vector< Face >::Iterator iter = _faces.begin();
   Vector< Face >::Iterator end  = _faces.end();

   uint edge = 0;
   for( ; iter != end; ++iter )
   {
      unsigned short v0 = iter->_indices[ 0 ];
      unsigned short v1 = iter->_indices[ 1 ];
      unsigned short v2 = iter->_indices[ 2 ];

      updateEdges( v0, v1, edge++ );
      updateEdges( v1, v2, edge++ );
      updateEdges( v2, v0, edge++ );
   }
}

//------------------------------------------------------------------------------
//!
void TriMesh::updateEdges(
   unsigned short vID,
   unsigned short vDest,
   uint edge
)
{
   uint startEdge = _verticesAdj[vID];
   if( startEdge == NULL_EDGE )
   {
      _verticesAdj[vID] = edge;
      _edgesRings[edge] = edge;
      return;
   }

   // Add edge to ring of vertex.
   _edgesRings[edge] = _edgesRings[startEdge];
   _edgesRings[startEdge] = edge;
}

//------------------------------------------------------------------------------
//!
//! For each face and for each corner
//!   if not normal compute normal
//!     Create a new normal
//!     for each face at vertex having smoothgroup & != 0
//!       compute face normal
//!       add to vertex normal
//!       set normal to new normal
void TriMesh::computeNormals()
{
   if( !_normals.empty() )
   {
      return;
   }

   int qty = _faces.size() - _smoothingGroups.size();
   if(  qty > 0  )
   {
      for ( ; qty > 0; --qty)
      {
         _smoothingGroups.pushBack( 1 );
      }
   }

   computeAdjacencies();

   // Clear normals.
   Vector< Face >::Iterator it   = _faces.begin();
   Vector< Face >::Iterator end  = _faces.end();
   for( ; it != end; ++it )
   {
      it->_normals[0] = NULL_NORMAL;
      it->_normals[1] = NULL_NORMAL;
      it->_normals[2] = NULL_NORMAL;
   }

   // Compute normals.
   it = _faces.begin();
   for( int f = 0 ; it != end; ++it, ++f )
   {
      for ( int c = 0; c < 3; ++c )
      {
         if( it->_normals[c] == NULL_NORMAL  )
         {
            unsigned short nID     = _normals.size();
            Vec3f normal   = Vec3f::zero();
            int group      = _smoothingGroups[f];

            // The face is not smooth!
            if( group == 0 )
            {
               Vec3f& v0 = _vertices[ _faces[f]._indices[0] ];
               Vec3f& v1 = _vertices[ _faces[f]._indices[1] ];
               Vec3f& v2 = _vertices[ _faces[f]._indices[2] ];

               Vec3f fnormal = (v1-v0).cross( v2-v0 );
               normal += fnormal;
               _faces[f]._normals[c] = nID;
            }
            else
            {
               uint startEdge = f*3 + c;
               uint curEdge   = startEdge;

               do
               {
                  uint cf = curEdge / 3;
                  if( ( _smoothingGroups[cf] & group ) != 0 )
                  {
                     group |= _smoothingGroups[cf];

                     // Compute face normal.
                     Vec3f& v0 = _vertices[ _faces[cf]._indices[0] ];
                     Vec3f& v1 = _vertices[ _faces[cf]._indices[1] ];
                     Vec3f& v2 = _vertices[ _faces[cf]._indices[2] ];

                     Vec3f fnormal = (v1-v0).cross( v2-v0 );
                     normal += fnormal.normalize();

                     _faces[cf]._normals[curEdge % 3] = nID;
                  }
                  curEdge = _edgesRings[curEdge];
               } while ( curEdge != startEdge );
            }

            normal.normalize();
            _normals.pushBack( normal );
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
RCP<StaticEntity> TriMesh::generateObject( const Vector< RCP<Material> >& mats )
{
   RCP<StaticEntity> obj( new StaticEntity() );
   RCP<Surface> surf( new Surface() );
   obj->geometry( surf );

   if( _vertices.empty() )
   {
      return obj;
   }

   computeAdjacencies();
   computeNormals();

   // Clear wedges.
   Vector< Face >::Iterator fIt  = _faces.begin();
   Vector< Face >::Iterator fEnd = _faces.end();
   for( ; fIt != fEnd; ++fIt )
   {
      fIt->_wedges[0] = NULL_WEDGE;
      fIt->_wedges[1] = NULL_WEDGE;
      fIt->_wedges[2] = NULL_WEDGE;
   }

   // Create wedges.
   fIt = _faces.begin();
   for( int f = 0 ; fIt != fEnd; ++fIt, ++f )
   {
      for ( int c = 0; c < 3; ++c )
      {
         if( fIt->_wedges[c] == NULL_WEDGE  )
         {
            unsigned short wID = surf->numVertices();
            unsigned short nID = fIt->_normals[c];
            unsigned short vID = fIt->_indices[c];

            // Create wedge data.
            surf->addVertex( _vertices[vID] );
            //FIXME: surf->addNormal( _normals[nID] );
            surf->addMapping(
               vID < _texCoords.size() ?
               _texCoords[vID] : Vec2f::zero()
            );

            // Set faces to the current wedge.
            uint startEdge = f*3 + c;
            uint curEdge   = startEdge;
            do
            {
               uint cf = curEdge / 3;
               if( _faces[cf]._normals[curEdge % 3] == nID )
               {
                  _faces[cf]._wedges[curEdge % 3] = wID;
               }
               curEdge = _edgesRings[curEdge];
            } while ( curEdge != startEdge );
         }
      }
   }

   // Create Faces.
   computeSurface();

   Map< String, uint >  matToID;
   for ( uint s = 0; s < _surfaces.size(); ++s )
   {
      RCP<Surface::Patch> patch = surf->addPatch();

#if 0
      // Find material ID in object.
      uint matID = 0xffff;
      for ( uint m = 0; m < obj->getMaterials().size(); ++m )
      {
         if( obj->getMaterials()[m]->name() == _surfaces[s]->_matName )
         {
            matID = m;
            break;
         }
      }
      if( matID == 0xffff )
      {
         // find material in material list.
         for ( uint m = 0; m < mats.size(); ++m )
         {
            if( mats[m]->name() == _surfaces[s]->_matName )
            {
               obj->addMaterial( mats[m] );
               matID = obj->getMaterials().size()-1;
               break;
            }
         }
      }
      // Default material.
      if( matID == 0xfff )
      {
         matID = 0;
      }
#else
      uint matID;
      if( matToID.has(_surfaces[s]->_matName) )
      {
         matID = matToID[_surfaces[s]->_matName];
      }
      else
      {
         matID = matToID.size() + 1;  //skip 0
         matToID[_surfaces[s]->_matName] = matID;
      }
#endif
      patch->materialID( matID );

      // Create faces.
      patch->reserveFaces( _surfaces[s]->_faces.size() );
      for ( uint f = 0; f < _surfaces[s]->_faces.size(); ++f )
      {
         const Face& face = _faces[ _surfaces[s]->_faces[f] ];
         patch->addFace(
            face._wedges[0],
            face._wedges[1],
            face._wedges[2],
               face._wedges[0],
               face._wedges[1],
               face._wedges[2]
         );
      }
   }

   // Compute the derived data.
   surf->computeConnectivity();
   surf->computeDerivedData();

   return obj;
}

//------------------------------------------------------------------------------
//!
void TriMesh::add( const RCP<TriMesh>& mesh )
{
   // Make sure we have enough texture coordinates.
   _texCoords.resize( _vertices.size() );

   // Repositioning of indices for faces.
   int offset = _vertices.size();
   for ( uint f = 0; f < mesh->_faces.size(); ++f )
   {
      mesh->_faces[f]._indices[0] += offset;
      mesh->_faces[f]._indices[1] += offset;
      mesh->_faces[f]._indices[2] += offset;
   }

   // Surfaces.
   computeSurface();
   mesh->computeSurface();

   Vector< RCP<TMSurface > >::Iterator sIt  = mesh->_surfaces.begin();
   Vector< RCP<TMSurface > >::Iterator sEnd = mesh->_surfaces.end();
   offset = _faces.size();
   for ( ; sIt != sEnd; ++sIt )
   {
      for ( uint f = 0; f < (*sIt)->_faces.size(); ++f )
      {
         (*sIt)->_faces[f] += offset;
      }
   }

#if 0
   // Add ....
   _vertices.insert(
      _vertices.end(),
      mesh->_vertices.begin(),
      mesh->_vertices.end()
   );
   _normals.insert(
      _normals.end(),
      mesh->_normals.begin(),
      mesh->_normals.end()
   );
   _texCoords.insert(
      _texCoords.end(),
      mesh->_texCoords.begin(),
      mesh->_texCoords.end()
   );
   _smoothingGroups.insert(
      _smoothingGroups.end(),
      mesh->_smoothingGroups.begin(),
      mesh->_smoothingGroups.end()
   );
   _faces.insert(
      _faces.end(),
      mesh->_faces.begin(),
      mesh->_faces.end()
   );
   _surfaces.insert(
      _surfaces.end(),
      mesh->_surfaces.begin(),
      mesh->_surfaces.end()
   );
#else
#define APPEND_VECTOR( vecName ) \
   vecName.reserve( vecName.size() + mesh->vecName.size() ); \
   for( uint i = 0; i < mesh->vecName.size(); ++ i ) \
   { \
      vecName.pushBack( mesh->vecName[i] ); \
   }

   APPEND_VECTOR( _vertices );
   APPEND_VECTOR( _normals );
   APPEND_VECTOR( _texCoords );
   APPEND_VECTOR( _smoothingGroups );
   APPEND_VECTOR( _faces );
   APPEND_VECTOR( _surfaces );

#undef APPEND_VECTOR

#endif
}

//------------------------------------------------------------------------------
//!
int importObjectVM( VMState* vm )
{
   Path path = VM::toPath( vm, 1 );

   float scale;
   if( VM::getTop(vm) >= 2 )
   {
      scale = (float)VM::toNumber( vm, 2 );
   }
   else
   {
      scale = 1.0f;
   }

   ObjectImporter importer;
   RCP<StaticEntity> entity = importer.importObject(path, scale);
   StaticEntityVM::push( vm, entity );

   return 1;
}

//------------------------------------------------------------------------------
//!
const VM::Reg vm_funcs[] = {
   // Blocks creation.
   { "importObject", importObjectVM },
   { 0, 0 }
};

UNNAMESPACE_END


/*==============================================================================
 CLASS ObjectImporter
==============================================================================*/

void
ObjectImporter::initialize( VMState* vm, const char* nameSpace )
{
   VM::registerFunctions( vm, nameSpace, vm_funcs );
}

//------------------------------------------------------------------------------
//!
ObjectImporter::ObjectImporter()
{

}

//------------------------------------------------------------------------------
//!
ObjectImporter::~ObjectImporter()
{

}

//------------------------------------------------------------------------------
//!
RCP<StaticEntity> ObjectImporter::importObject( const Path& name, float scale )
{
   DBG_BLOCK( os_oi, "ObjectImporter::importObject( path=" << name.string() << ", scale=" << scale << ")" );
   String ext = name.string().getExt();
   if( ext == "3ds" )
   {
      Vector< RCP<StaticEntity> > objects;
      Vector< RCP<Light> > lights;
      import3ds( name, scale, true, objects, lights );

      if( objects.empty() )
      {
         return RCP<StaticEntity>();
      }

      // Compute the kd-tree of each surface
      //const Surface::VecContainer& v = objects[0]->surface()->vertices();
      //Surface::SurfaceContainer& s   = objects[0]->surface()->surfaces();
      //for (uint i = 0; i < s.size(); ++i) {
      //   s[i]->kdTree() = new Surface::KDTreeRT( name.cstr(), v, s[i]->faces(), 3, 1, 4, -1 );
      //}

      return objects[0];
   }
   else if( ext == "ply" ) {
      RCP<StaticEntity> object(new StaticEntity);

      std::cout << std::endl
                << "Loading mesh... " << std::flush;
      import_ply(name, scale, object);
      std::cout << "done." << std::endl;

      // Compute the kd-tree of each surface
      //const Surface::VecContainer& v = object->surface()->vertices();
      //Surface::SurfaceContainer& s   = object->surface()->surfaces();
      //for (uint i = 0; i < s.size(); ++i) {
      //   s[i]->kdTree() = new Surface::KDTreeRT( name.cstr(), v, s[i]->faces(), 3, 1, 4, -1 );
      //}

      return object;
   }
   else if( ext == "fab" ) {
      RCP<StaticEntity> object(new StaticEntity);

      import_fab(name, scale, object);

      // Compute the kd-tree of each surface
      //const Surface::VecContainer& v = object->surface()->vertices();
      //Surface::SurfaceContainer& s   = object->surface()->surfaces();
      //for (uint i = 0; i < s.size(); ++i) {
      //   s[i]->kdTree() = new Surface::KDTreeRT( name.cstr(), v, s[i]->faces(), 3, 1, 4, -1 );
      //}

      return object;
   }

   RCP<StaticEntity> object;
   return object;
}

#if 0
//------------------------------------------------------------------------------
//!
RCP<Scene> ObjectImporter::importScene( const Path& name, float scale )
{
   Vector< RCP<StaticEntity> > objects;
   Vector< RCP<Light> > lights;
   import3ds( name, scale, false, objects, lights );

   RCP<Scene> scene( new Scene() );
   for ( uint obj = 0; obj < objects.size(); ++obj )
   {
      scene->addObject( objects[obj] );
   }
   for ( uint l = 0; l < lights.size(); ++l )
   {
      scene->addLight( lights[l] );
   }
   return scene;
}
#endif

//------------------------------------------------------------------------------
//!
void ObjectImporter::import3ds(
   const Path& name,
   float scale,
   bool compress,
   Vector< RCP<StaticEntity> >& objects,
   Vector< RCP<Light> >& lights
)
{
   DBG_BLOCK( os_oi, "ObjectImporter::import3ds()" );
   Vector< String >  matNames;

   RCP<FileDevice> fd = new FileDevice( name, IODevice::MODE_READ );
   if( !fd->isOpen() )
   {
      printf( "FILE NOT FOUND: %s\n", name.cstr() );
      return;
   }

   BinaryStream  stream( fd.ptr(), BinaryStream::ENDIAN_LITTLE );


   // Loading mesh.
   Vector< RCP<Material> > materials;
   RCP<Material> material;
   Vector< RCP<TriMesh> > meshes;
   RCP<TriMesh> mesh;
   RCP<Light> light;

   for ( ;; )
   {
      unsigned short chunkID;
      uint   chunkLength;
      read( stream, chunkID );
      read( stream, chunkLength );

      //DBG_MSG( os_oi, "chunkID=" << chunkID << ", len=" << chunkLength );
      if( fd->eof() )
      {
         break;
      }

      switch( chunkID )
      {
         case MAIN_CHUNK:
         {
         } break;

         case EDIT3D_CHUNK:
         {
         } break;

         case OBJECT_CHUNK:
         {
            String name;
            read( stream, name );
         } break;

         case TRIMESH_CHUNK:
         {
            mesh = RCP<TriMesh>( new TriMesh() );
            meshes.pushBack( mesh );
         } break;

         case VERTICES_CHUNK:
         {
            unsigned short qty;
            Vec3f v;
            read( stream, qty );
            DBG_MSG( os_oi, qty << " vertices" );
            for ( ; qty > 0; --qty )
            {
               read( stream, v );
               mesh->_vertices.pushBack( scale * Vec3f( v(0), v(2), -v(1) ) );
            }
         } break;

         case FACES_CHUNK:
         {
            unsigned short qty;
            TriMesh::Face face;
            read( stream, qty );
            DBG_MSG( os_oi, qty << " faces" );

            unsigned short i0;
            unsigned short i1;
            unsigned short i2;
            for ( ; qty > 0; --qty )
            {
               read( stream, i0 );
               read( stream, i1 );
               read( stream, i2 );
               read( stream, face._flag );

               face._indices[0] = i0;
               face._indices[1] = i1;
               face._indices[2] = i2;
               mesh->_faces.pushBack( face );
            }
         } break;

         case TEXCOORDS_CHUNK:
         {
            unsigned short qty;
            Vec2f v;
            read( stream, qty );
            DBG_MSG( os_oi, qty << " texCoords" );
            for ( ; qty > 0; --qty )
            {
               read( stream, v );
               mesh->_texCoords.pushBack( v );
            }
         } break;

         case SMOOTHING_CHUNK:
         {
            unsigned short qty = ( chunkLength - 6 ) / 4;
            DBG_MSG( os_oi, qty << " smoothings" );
            uint group;
            for ( ; qty > 0; --qty )
            {
               read( stream, group );
               mesh->_smoothingGroups.pushBack( group );
            }
         } break;

         case MATGROUP_CHUNK:
         {
            RCP<TriMesh::TMSurface> surface( new TriMesh::TMSurface() );
            mesh->_surfaces.pushBack( surface );
            read( stream, surface->_matName );

            unsigned short qty;
            unsigned short matID;
            read( stream, qty );
            DBG_MSG( os_oi, qty << " materials (" << surface->_matName << ")" );

            for ( ; qty > 0; --qty )
            {
               read( stream, matID );
               surface->_faces.pushBack( matID );
            }
         } break;


         case REF_CHUNK:
         {
            Vec3f xaxis;
            Vec3f yaxis;
            Vec3f zaxis;
            Vec3f pos;
            read( stream, xaxis );
            read( stream, yaxis );
            read( stream, zaxis );
            read( stream, pos );
            //pos *= scale;
            //mesh->_referential.position( -pos );
            //mesh->_referential.orientation( xaxis, yaxis, zaxis );
            //mesh->_referential.position( -pos(0), -pos(2), pos(1) );
            //Quatf& q = mesh->_referential.orientation();
            //mesh->_referential.orientation( Quatf( q.getX(), q.getZ(), -q.getY(), q.getW() ) );
         } break;

         case MAT_CHUNK:
         {
            material = RCP<Material>( new Material() );
         } break;

         case MAT_NAME_CHUNK:
         {
            String name;
            read( stream, name );

            //material->setName( name );
            matNames.pushBack( name );
            materials.pushBack( material );
         } break;

         case MAT_DIFFUSE_CHUNK:
         {
            unsigned short colID;
            uint   colLength;
            read( stream, colID );
            read( stream, colLength );

            if( colID == COLOR_CHUNK )
            {
               Vec3f col;
               read( stream, col );
               //material->setDiffuse( col );
            }
            else
            {
               unsigned char r, g, b;
               read( stream, r );
               read( stream, g );
               read( stream, b );
               //material->setDiffuse( Vec3f( r, g, b ) / 255.0f );
            }
         } break;
         case MAT_SPECULAR_CHUNK:
         {
            unsigned short colID;
            uint   colLength;
            read( stream, colID );
            read( stream, colLength );

            if( colID == COLOR_CHUNK )
            {
               Vec3f col;
               read( stream, col );
               //material->setSpecular( col );
            }
            else
            {
               unsigned char r, g, b;
               read( stream, r );
               read( stream, g );
               read( stream, b );
               //material->setSpecular( Vec3f( r, g, b ) / 255.0f );
            }
         } break;
         case MAT_SHININESS_CHUNK:
         {
            unsigned short valID;
            uint   valLength;
            read( stream, valID );
            read( stream, valLength );

            if( valID == FLOAT_PERCENTAGE )
            {
               float val;
               read( stream, val );
               //material->setRoughness( Vec2f( val, val ) );
            }
            else
            {
               unsigned short val;
               read( stream, val );
               //material->setRoughness( Vec2f( (float)val, (float)val ) );
            }
         } break;

         case LIGHT_CHUNK:
         {
            light = RCP<Light>( new Light() );
            lights.pushBack( light );

            Vec3f pos;
            read( stream, pos );
            light->position( scale* Vec3f( pos(0), pos(2), -pos(1) ) );
         } break;
         case COLOR_CHUNK:
         {
            Vec3f col;
            read( stream, col );
            if( light.isValid() )
            {
               light->color( col );
            }
         } break;
         case LIGHT_MULTIPLIER_CHUNK:
         {
            float mul;
            read( stream, mul );
         } break;


         //default: stream.seekg( chunkLength - 6, std::ios::cur );
         default: fd->skip( chunkLength - 6 );
      }
   }

   if( compress )
   {
      if( meshes.empty() )
      {
         return;
      }
      Vector< RCP<TriMesh> >::Iterator it  = meshes.begin();
      Vector< RCP<TriMesh> >::Iterator end = meshes.end();
      RCP<TriMesh> mesh = *it;
      for ( ++it ; it != end; ++it )
      {
         mesh->add( *it );
      }
      objects.pushBack( mesh->generateObject( materials ) );
   }
   else
   {
      Vector< RCP<TriMesh> >::Iterator it  = meshes.begin();
      Vector< RCP<TriMesh> >::Iterator end = meshes.end();
      for ( ; it != end; ++it )
      {
         objects.pushBack( (*it)->generateObject( materials ) );
      }
   }
}

//------------------------------------------------------------------------------
//!
void ObjectImporter::import_ply(
   const Path&        name,
   float              scale,
   const RCP<StaticEntity>& object
)
{
#if 1
#if !defined(_MSC_VER)
#warning PLY IMPORTING DISABLED
#endif
#else
   DBG_BLOCK( os_oi, "ObjectImporter::import_ply()" );
   IFStream stream;

   stream.open( name.cstr() );

   if( !stream.isOpen() )
   {
      printf( "FILE NOT FOUND: %s\n", name.cstr() );
      return;
   }

   String line;

   // Make sure the first string of the file is "ply"
   stream.getLine( line );
   if( !line.startsWith( "ply" ) )
   {
      std::cout << "Invalid file! Expected a ply file." << std::endl;
   }

   // Read the header
   String tag, element_type;
   uint vertex_count = 0, face_count = 0;
   bool ascii = true;
   while( true )
   {
      stream.getLine( line );
      IStrStream line_stream( line );

      if( stream.eof() )
      {
         std::cerr << "Error parsing file... in header." << std::endl;
         return;
      }

      line_stream >> tag;

      if( tag.startsWith( "end_header" ) )
      {
         break;
      }
      else if( tag.startsWith( "element" ) ) {
         line_stream >> element_type;
         if( element_type.startsWith( "vertex" ) )
         {
            line_stream >> vertex_count;
         }
         else if( element_type.startsWith( "face" ) )
         {
            line_stream >> face_count;
         }
      }
      else if( tag.startsWith( "format" ) )
      {
         String format;
         line_stream >> format;
         if( format == "ascii" )
         {
            ascii = true;
         }
         else
         {
            ascii = false;
         }
      }
   }

   // The object geometry
   RCP<Surface> surf(new Surface);

   // ASCII reading.
   if( ascii )
   {
      // Read the vertices
      Vec3f vertex;
      surf->reserveVertices( vertex_count );
      for( uint i = 0; i < vertex_count; ++i )
      {
         stream.getLine( line );
         IStrStream line_stream( line );

         if( stream.eof() )
         {
            std::cerr << "Error parsing file... not enough vertices." << std::endl;
            return;
         }
         line_stream >> vertex(0) >> vertex(1) >> vertex(2);
         surf->addVertex( scale * vertex );
      }

      // The surface
      RCP<Surface::Patch> patch = surf->addPatch();

      // Read the faces
      uint v0, v1, v2;
      uint face_vertex_count;
      patch->reserveFaces( face_count );
      for( uint i = 0; i < face_count; ++i )
      {
         stream.getLine( line );
         IStrStream line_stream( line );

         if( stream.eof() )
         {
            std::cerr << "Error parsing file... not enough faces." << std::endl;
            return;
         }

         line_stream >> face_vertex_count;
         if( face_vertex_count != 3 )
         {
            std::cerr << "Only triangle mesh is supported." << std::endl;
            return;
         }

         line_stream >> v0 >> v1 >> v2;
         patch->addFace( v0, v1, v2 );
      }
   }
   // Binary reading.
   else
   {
      // Read the vertices.
      surf->reserveVertices( vertex_count );
      for( uint i = 0; i < vertex_count; ++i)
      {
         Vec3f vertex;
         read( stream, vertex );
         swap( vertex );
         surf->addVertex( scale * vertex );
      }

      // Read the faces.
      RCP<Surface::Patch> patch = surf->addPatch();
      patch->reserveFaces( face_count );

      char buffer[4*40];
      for( uint i = 0; i < face_count; ++i )
      {
         unsigned char vcount;
         read( stream, vcount );

         if( vcount == 3 )
         {
            Vec3i vid;
            read( stream, vid );
            swap( vid );
            patch->addFace( vid(0), vid(1), vid(2) );
         }
         else
         {
            stream.read( buffer, vcount*4 );
         }
      }
   }

   surf->computeConnectivity();
   surf->computeDerivedData();
   object->surface( surf );
#endif
}

//------------------------------------------------------------------------------
//!
void ObjectImporter::import_fab(const Path& name, float scale, const RCP<StaticEntity>& object)
{
   DBG_BLOCK( os_oi, "ObjectImporter::import_fab()" );

   RCP<FileDevice> fd = new FileDevice( name, IODevice::MODE_READ );
   if( !fd->isOpen() )
   {
      printf( "FILE NOT FOUND: %s\n", name.cstr() );
      return;
   }

   BinaryStream  stream( fd.ptr() );


   // Read the header
   String type;
   uint vertex_count, face_count;
   stream >> type >> vertex_count >> face_count;
   if (!type.startsWith("fab")) {
      std::cout << "Invalid file! Expected a fab file." << std::endl;
   }

   // The object geometry
   RCP<Surface> surf(new Surface);

   // Read the vertices
   Vec3f vertex;
   surf->reserveVertices(vertex_count);
   for (uint i = 0; i < vertex_count; ++i) {
      stream >> vertex(0) >> vertex(1) >> vertex(2);
      surf->addVertex(scale * vertex);
   }

   // The surface
   RCP<Surface::Patch> patch = surf->addPatch();

   // Read the faces
   uint v0, v1, v2;
   patch->reserveFaces(face_count);
   for (uint i = 0; i < face_count; ++i) {
      stream >> v0 >> v1 >> v2;
      patch->addFace(v0, v1, v2);
   }

   surf->computeConnectivity();
   surf->computeDerivedData();
   object->geometry(surf);
}
