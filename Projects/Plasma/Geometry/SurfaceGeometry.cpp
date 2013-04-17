/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Geometry/SurfaceGeometry.h>
#include <Plasma/Animation/Skeleton.h>
#include <Plasma/Animation/Pinocchio.h>

#include <Fusion/Core/Core.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/IO/FileDevice.h>

#include <cmath>

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_surf, "SurfaceGeometry" );

/*
//------------------------------------------------------------------------------
//!
int
saveVM( VMState* vm )
{
   Surface* surface = (Surface*)VM::thisPtr( vm );
   const char* name = VM::toCString( vm, 1 );

   RCP<FileDevice> of = new FileDevice( name, IODevice::MODE_WRITE );
   TextStream out( of.ptr() );

   for( uint i = 0; i < surface->numVertices(); ++i )
   {
      const Vec3f& v = surface->vertex(i);
      out << "v " << v(0) << " " << v(1) << " " << v(2) << "\n";
   }

   Surface::PatchContainer::ConstIterator patchIt  = surface->patches().begin();
   Surface::PatchContainer::ConstIterator patchEnd = surface->patches().end();
   for( uint i = 0; patchIt != patchEnd; ++patchIt, ++i )
   {
      for( uint fid = 0; fid < (*patchIt)->numFaces(); ++fid )
      {
         const Surface::Face& f = (*patchIt)->faces()[fid];
         out << "f " << f._vID[0]+1 << " " << f._vID[1]+1 << " " << f._vID[2]+1 << "\n";
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int
computeWeightsVM( VMState* vm )
{
   Surface*      surface  = (Surface*)VM::thisPtr( vm );
   RCP<Skeleton> skeleton = SkeletonVM::to( vm, 1 );
   surface->computeWeights( skeleton.ptr() );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
translateVM( VMState* vm )
{
   Surface* surface = (Surface*)VM::thisPtr( vm );
   surface->translate( VM::toVec3f( vm, 1 ) );
   return 0;
}
*/

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS SurfaceGeometry::Patch
==============================================================================*/

//------------------------------------------------------------------------------
//!
SurfaceGeometry::Patch::Patch()
   : _materialID(1)
{
}

//------------------------------------------------------------------------------
//!
SurfaceGeometry::Patch::~Patch()
{
}

/*==============================================================================
   CLASS SurfaceGeometry
==============================================================================*/

//------------------------------------------------------------------------------
//!
SurfaceGeometry::SurfaceGeometry():
   Geometry( SURFACE )
{
}

//------------------------------------------------------------------------------
//!
SurfaceGeometry::~SurfaceGeometry()
{
}

//------------------------------------------------------------------------------
//!
uint
SurfaceGeometry::numFaces() const
{
   uint nbFace = 0;

   for( uint p = 0; p < _patches.size(); ++p )
   {
      nbFace += uint(_patches[p]->_faces.size());
   }

   return nbFace;
}


//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::computeConnectivity()
{
   _verticesStars.resize( _vertices.size() );
   std::fill( _verticesStars.begin(), _verticesStars.end(), (uint)NULL_ID );

   // Compute connectivity information for all patches.
   for( uint p = 0; p < _patches.size(); ++p )
   {
      // Allocate memory for connectivity information.
      _patches[p]->_edgesNeighbors.resize( _patches[p]->_faces.size() * 3 );
      _patches[p]->_edgesStars.resize( _patches[p]->_faces.size() * 3 );

      // Add each of 3 edges for all triangles faces.
      FaceContainer& faces = _patches[p]->_faces;
      uint edge = p << 24;
      for( uint f = 0; f < faces.size(); ++f )
      {
         uint v0 = faces[f]._vID[0];
         uint v1 = faces[f]._vID[1];
         uint v2 = faces[f]._vID[2];
         addEdge( v0, v1, edge++ );
         addEdge( v1, v2, edge++ );
         addEdge( v2, v0, edge++ );
      }
   }

   // Clear star information.
   for( uint p = 0; p < _patches.size(); ++p )
   {
      _patches[p]->_edgesStars = EdgeContainer();
   }
}

//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::computeDerivedData()
{
   if( _verticesStars.empty() )
   {
      computeConnectivity();
   }

   computeBBox();
   computeNormals();
   computeTangents();
   computeDefaultMapping();
}

//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::computeWeights( Skeleton* skeleton )
{
   printf( "Computing weights...\n" );
   Pinocchio::recomputeVertexWeights( this, skeleton );
}

//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::translate( const Vec3f& v )
{
   for( uint i = 0; i < _vertices.size(); ++i )
   {
      _vertices[i] += v;
   }
   _bbox.translate( v );
}

//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::computeBBox()
{
   // Do we have something to compute?
   if( _vertices.empty() ) return;

   // Compute bounding box.
   _bbox.set( _vertices[0] );
   for( uint v = 1; v < _vertices.size(); ++v )
   {
      _bbox |= _vertices[v];
   }
}

//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::computeBIH()
{
   // Do we have something to compute?
   if( _vertices.empty() ) return;

   uint numPrims = numFaces();
   Vector<AABBoxf> bboxes( numPrims, AABBoxf::empty() );
   Vector<AABBoxf*> bboxesPtr( numPrims );
   Vector<Vec3f> center( numPrims );
   Vector<uint> ids( numPrims );
   uint i = 0;
   for( uint p = 0; p < numPatches(); ++p )
   {
      FaceContainer& faces = _patches[p]->_faces;
      for( uint f = 0; f < faces.size(); ++f, ++i )
      {
         const Vec3f& v0 = _vertices[faces[f]._vID[0]];
         const Vec3f& v1 = _vertices[faces[f]._vID[1]];
         const Vec3f& v2 = _vertices[faces[f]._vID[2]];

         bboxes[i] |= v0;
         bboxes[i] |= v1;
         bboxes[i] |= v2;
         bboxes[i].grow( 1e-4f );
         

         bboxesPtr[i] = &(bboxes[i]);
         center[i]    = ( v0 + v1 + v2 ) * (1.0f/3.0f);
         ids[i]       = (p<<24) | f;
      }
   }

   _bih.create( bboxesPtr, center, &ids, 2 );
}

//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::clearBIH()
{
   _bih.clear();
}

//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::computeNormals()
{
   // Compute normals without taking into account mapping creases.
   // 1. Prepare data.
   Vector< Vector<uint> > nIDs( _patches.size() );
   for( uint p = 0; p < nIDs.size(); ++p )
   {
      // FIXME: use swap?
      nIDs[p] = Vector<uint>( _patches[p]->_faces.size()*3, NULL_ID );
   }

   // 2. Compute normals IDs.
   uint dID = 0;
   for( uint p = 0; p < _patches.size(); ++p )
   {
      for( uint e = 0; e < nIDs[p].size(); ++e )
      {
         if( nIDs[p][e] == NULL_ID )
         {
            nIDs[p][e] = dID;

            // Go around vertex and set ID to similar corner.
            uint curEdge = _patches[p]->_edgesNeighbors[e];
            while( curEdge != NULL_ID )
            {
               if( crease( curEdge ) > 0 )
               {
                  break;
               }
               curEdge = nextEdge( curEdge );
               if( nIDs[curEdge>>24][curEdge&0xffffff] != NULL_ID )
               {
                  break;
               }
               nIDs[curEdge>>24][curEdge&0xffffff] = dID;
               curEdge = neighborEdge( curEdge );
            }
            // Go around the other way.
            curEdge = _patches[p]->_edgesNeighbors[prevEdge( e )];
            while( curEdge != NULL_ID )
            {
               if( crease( curEdge ) > 0 )
               {
                  break;
               }
               if( nIDs[curEdge>>24][curEdge&0xffffff] != NULL_ID )
               {
                  break;
               }
               nIDs[curEdge>>24][curEdge&0xffffff] = dID;
               curEdge = neighborEdge( prevEdge( curEdge ) );
            }
            ++dID;
         }
      }
   }

   // 3. Compute normals.
   VecContainer normals( dID, Vec3f::zero() );
   for( uint p = 0; p < _patches.size(); ++p )
   {
      FaceContainer& faces = _patches[p]->_faces;
      for( uint f = 0; f < faces.size(); ++f )
      {
         // Compute face normal.
         Vec3f e0 = _vertices[faces[f]._vID[1]] - _vertices[faces[f]._vID[0]];
         Vec3f e1 = _vertices[faces[f]._vID[2]] - _vertices[faces[f]._vID[0]];
         Vec3f normal = e0.cross( e1 );

         normals[nIDs[p][f*3+0]] += normal;
         normals[nIDs[p][f*3+1]] += normal;
         normals[nIDs[p][f*3+2]] += normal;
      }
   }

   // 4. Normalized all normals.
   for( uint n = 0; n < normals.size(); ++n )
   {
      normals[n].normalize();
   }

   // Compute derived data IDs and set normals.
   // 1. Reset faces normals (derived data) IDs.
   for( uint p = 0; p < _patches.size(); ++p )
   {
      FaceContainer& faces = _patches[p]->_faces;
      for( uint f = 0; f < faces.size(); ++f )
      {
         faces[f]._dID[0] = NULL_ID;
         faces[f]._dID[1] = NULL_ID;
         faces[f]._dID[2] = NULL_ID;
      }
   }

   // 2. Compute IDs and assigned normals.
   _normals.clear();
   dID = 0;
   for( uint p = 0; p < _patches.size(); ++p )
   {
      FaceContainer& faces = _patches[p]->_faces;
      for( uint f = 0; f < faces.size(); ++f )
      {
         for( uint e = 0; e < 3; ++e )
         {
            if( faces[f]._dID[e] == NULL_ID )
            {
               _normals.pushBack( normals[nIDs[p][f*3+e]] );

               faces[f]._dID[e] = dID;
               uint mID = faces[f]._mID[e];

               // Go around vertex and set ID to similar corner.
               uint curEdge = _patches[p]->_edgesNeighbors[(f*3)+e];
               while( curEdge != NULL_ID )
               {
                  Face& face = faceFromEdge( curEdge );
                  uint edgeNb = (curEdge&0xffffff) % 3;
                  if( face._creases[edgeNb] > 0 )
                  {
                     break;
                  }
                  edgeNb = (edgeNb + 1) % 3;
                  if( face._dID[edgeNb] != NULL_ID || face._mID[edgeNb] != mID )
                  {
                     break;
                  }
                  face._dID[edgeNb] = dID;
                  curEdge = neighborEdge( nextEdge( curEdge ) );
               }

               curEdge = _patches[p]->_edgesNeighbors[(f*3)+((e+2)%3)];
               while( curEdge != NULL_ID )
               {
                  Face& face = faceFromEdge( curEdge );
                  uint edgeNb = (curEdge&0xffffff) % 3;
                  if( face._creases[edgeNb] > 0 )
                  {
                     break;
                  }
                  if( face._dID[edgeNb] != NULL_ID || face._mID[edgeNb] != mID )
                  {
                     break;
                  }
                  face._dID[edgeNb] = dID;
                  curEdge = neighborEdge( prevEdge( curEdge ) );
               }
               ++dID;
            }
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::computeTangents()
{
   if( _mapping.empty() ) return;

   _tangents.resize( _normals.size() );
   std::fill( _tangents.begin(), _tangents.end(), Vec4f::zero() );

   Vector<Vec3f> bitangents( _normals.size(), Vec3f::zero() );

   // For each patch compute the tangents.
   for( uint p = 0; p < _patches.size(); ++p )
   {
      // For each face compute tangents.
      FaceContainer& faces = _patches[p]->_faces;
      for( uint f = 0; f < faces.size(); ++f )
      {
         const Vec3f& v0 = _vertices[faces[f]._vID[0]];
         const Vec3f& v1 = _vertices[faces[f]._vID[1]];
         const Vec3f& v2 = _vertices[faces[f]._vID[2]];

         const Vec2f& w0 = _mapping[faces[f]._mID[0]];
         const Vec2f& w1 = _mapping[faces[f]._mID[1]];
         const Vec2f& w2 = _mapping[faces[f]._mID[2]];

         float x1 = v1(0) - v0(0);
         float x2 = v2(0) - v0(0);
         float y1 = v1(1) - v0(1);
         float y2 = v2(1) - v0(1);
         float z1 = v1(2) - v0(2);
         float z2 = v2(2) - v0(2);

         float s1 = w1(0) - w0(0);
         float s2 = w2(0) - w0(0);
         float t1 = w1(1) - w0(1);
         float t2 = w2(1) - w0(1);

         float r = 1.0f / (s1*t2 - s2*t1);
         Vec4f tangent(
            (t2*x1 - t1*x2)*r,
            (t2*y1 - t1*y2)*r,
            (t2*z1 - t1*z2)*r,
            0.0f
         );
         Vec3f bitangent(
            (s1*x2 - s2*x1)*r,
            (s1*y2 - s2*y1)*r,
            (s1*z2 - s2*z1)*r
         );

         // Add axis to each vertex components.
         _tangents[faces[f]._dID[0]] += tangent;
         _tangents[faces[f]._dID[1]] += tangent;
         _tangents[faces[f]._dID[2]] += tangent;

         bitangents[faces[f]._dID[0]] += bitangent;
         bitangents[faces[f]._dID[1]] += bitangent;
         bitangents[faces[f]._dID[2]] += bitangent;
      }
   }

   for( uint d = 0; d < _tangents.size(); ++d )
   {
      Vec3f& n = _normals[d];
      Vec3f& t = reinterpret_cast<Vec3f&>( _tangents[d] );

      // Gram-Schmidt orthogonalization.
      t  = ( t - n * n.dot(t) ).normalize();

      // Handness.
      _tangents[d](3) = bitangents[d].dot( n.cross(t) ) < 0.0f ? -1.0f : 1.0f;
   }
}

//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::computeDefaultMapping()
{
   if( !_mapping.empty() ) return;

   _mapping.pushBack( Vec2f::zero() );

   for( uint p = 0; p < _patches.size(); ++p )
   {
      FaceContainer& faces = _patches[p]->_faces;
      for( uint f = 0; f < faces.size(); ++f )
      {
         faces[f]._mID[0] = 0;
         faces[f]._mID[1] = 0;
         faces[f]._mID[2] = 0;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::computeCreases()
{
   for( uint p = 0; p < _patches.size(); ++p )
   {
      FaceContainer& faces = _patches[p]->_faces;
      for( uint f = 0; f < faces.size(); ++f )
      {
         for( uint e = 0; e < 3; ++e )
         {
            uint eID0 = (p<<24)+(f*3)+e;
            uint eID1 = neighborEdge( eID0 );

            if( eID0 < eID1 && eID1 != NULL_ID )
            {
               Face& face1 = faceFromEdge( eID1 );
               uint eNb = (eID1&0xffffff) % 3;
               uchar crease =
                  CGM::max( faces[f]._creases[e], face1._creases[eNb] );

               faces[f]._creases[e] = crease;
               face1._creases[eNb]  = crease;
            }
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::computeRenderableGeometry()
{
   DBG_BLOCK( os_surf, "SurfaceGeometry::ComputeGeometry" );

   // Create vertex data.
   bool skin       = numWeights() > 0;
   uint vsize      = skin ? 84: 48;
   uint nbVertices = numNormals();
   uint size       = nbVertices * vsize;

   if( nbVertices == 0 )
   {
      _rgeom = 0;
      return;
   }

   _rgeom = Core::gfx()->createGeometry( Gfx::PRIM_TRIANGLES );

   Vector<bool> vcreated( nbVertices, false );
   uchar* buffer = new uchar[size];

   for( uint p = 0; p < _patches.size(); ++p )
   {
      FaceContainer& faces = _patches[p]->_faces;
      for( uint f = 0; f < faces.size(); ++f )
      {
         for( uint e = 0; e < 3; ++e )
         {
            uint id = faces[f]._dID[e];
            if( !vcreated[id] )
            {
               float* fbuffer = (float*)&buffer[id*vsize];

               // Position.
               const Vec3f& v = vertex( faces[f]._vID[e] );
               *fbuffer++ = v(0);
               *fbuffer++ = v(1);
               *fbuffer++ = v(2);

               // Normal.
               const Vec3f& n = normal( id );
               *fbuffer++ = n(0);
               *fbuffer++ = n(1);
               *fbuffer++ = n(2);

               // Tangent.
               const Vec4f& t = tangent( id );
               *fbuffer++ = t(0);
               *fbuffer++ = t(1);
               *fbuffer++ = t(2);
               *fbuffer++ = t(3);

               // Mapping.
               const Vec2f& m = mapping( faces[f]._mID[e] );
               *fbuffer++ = m(0);
               *fbuffer++ = m(1);

               // Weights.
               if( skin )
               {
                  const Weights& w = weight( faces[f]._vID[e] );
                  *fbuffer++ = w._weights(0);
                  *fbuffer++ = w._weights(1);
                  *fbuffer++ = w._weights(2);
                  *fbuffer++ = w._weights(3);
                  *fbuffer++ = w._bones(0);
                  *fbuffer++ = w._bones(1);
                  *fbuffer++ = w._bones(2);
                  *fbuffer++ = w._bones(3);
                  *fbuffer++ = w._qty;
               }
            }
         }
      }
   }
   DBG_MSG( os_surf, "Create vertex buffer" );
   RCP<Gfx::VertexBuffer> vbuffer( Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, size, buffer ) );
   vbuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION,  Gfx::ATTRIB_FMT_32F_32F_32F, 0 );
   vbuffer->addAttribute( Gfx::ATTRIB_TYPE_NORMAL,    Gfx::ATTRIB_FMT_32F_32F_32F, 12 );
   vbuffer->addAttribute( Gfx::ATTRIB_TYPE_TANGENT,   Gfx::ATTRIB_FMT_32F_32F_32F_32F, 24 );
   vbuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, 40 );

   if( skin )
   {
      vbuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD1, Gfx::ATTRIB_FMT_32F_32F_32F_32F, 48 );
      vbuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD2, Gfx::ATTRIB_FMT_32F_32F_32F_32F, 64 );
      vbuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD3, Gfx::ATTRIB_FMT_32F, 80 );
   }
   DBG_MSG( os_surf, "Add vertex buffer to geometry." );
   _rgeom->addBuffer( vbuffer );

   delete[] buffer;

   // Create index data.
   uint nbIndices = numFaces() * 3;
   if( nbVertices < 0xffff )
   {
      ushort* ibuffer = new ushort[nbIndices];
      ushort* sbuffer = ibuffer;
      uint offset     = 0;
      for( uint p = 0; p < _patches.size(); ++p )
      {
         Geometry::addPatch( offset, _patches[p]->numFaces() * 3, 0 );

         FaceContainer& faces = _patches[p]->_faces;
         for( uint f = 0; f < faces.size(); ++f )
         {
            *sbuffer++ = faces[f]._dID[0];
            *sbuffer++ = faces[f]._dID[1];
            *sbuffer++ = faces[f]._dID[2];
         }
         offset += uint(faces.size()) * 3;
      }
      DBG_MSG( os_surf, "Create index buffer 16 bytes." );
      _rgeom->indexBuffer(
         Core::gfx()->createBuffer( Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_NONE, nbIndices*sizeof(ushort), ibuffer )
      );
      delete[] ibuffer;
   }
   else
   {
      uint* ibuffer = new uint[nbIndices];
      uint* ubuffer = ibuffer;
      uint offset   = 0;
      for( uint p = 0; p < _patches.size(); ++p )
      {
         Geometry::addPatch( offset, _patches[p]->numFaces() * 3, 0 );

         FaceContainer& faces = _patches[p]->_faces;
         for( uint f = 0; f < faces.size(); ++f )
         {
            *ubuffer++ = faces[f]._dID[0];
            *ubuffer++ = faces[f]._dID[1];
            *ubuffer++ = faces[f]._dID[2];
         }
         offset += uint(faces.size()) * 3;
      }
      DBG_MSG( os_surf, "Create index buffer 32 bytes." );
      _rgeom->indexBuffer(
         Core::gfx()->createBuffer( Gfx::INDEX_FMT_32, Gfx::BUFFER_FLAGS_NONE, nbIndices*sizeof(uint), ibuffer )
      );
      delete[] ibuffer;
   }
}

//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::addEdge( uint v0, uint v1, uint edge )
{
   // No neighbor for  now.
   _patches[edge>>24]->_edgesNeighbors[edge&0xffffff] = NULL_ID;

   // First edge connect to vertex v0?
   uint firstEdge = starFirst( v0 );
   if( firstEdge == NULL_ID )
   {
      _verticesStars[v0] = edge;
      _patches[edge>>24]->_edgesStars[edge&0xffffff] = edge;
      return;
   }

   // Find neighbor edge.
   uint curEdge = firstEdge;
   do
   {
      uint prvEdge = prevEdge( curEdge );
      if( v1 == orgVertexID( prvEdge ) )
      {
         // Connects neighbors edges.
         // In case of multiples triangles sharing the same two vertices, the
         // triangles are paired together in the order of their creation.
         if( _patches[prvEdge>>24]->_edgesNeighbors[prvEdge&0xffffff] == NULL_ID )
         {
            neighbors( edge, prvEdge );
         }
      }
      curEdge = starNext( curEdge );
   } while( curEdge != firstEdge );

   // Add edge to star.
   _patches[edge>>24]->_edgesStars[edge&0xffffff] = starNext( firstEdge );
   _patches[firstEdge>>24]->_edgesStars[firstEdge&0xffffff] = edge;
}

//------------------------------------------------------------------------------
//! This work only for triangulated quads.
//!
void
SurfaceGeometry::splitFace( uint p0, uint f0 )
{
   // Find neighbor.
   uint iEdge0 = (p0<<24)+(f0*3);
   uint iEdge1 = neighborEdge( iEdge0 );
   uint p1     = iEdge1>>24;

   // For now, only face with neighbor is supported.
   uint f1 = (iEdge1&0xffffff)/3;
   FaceContainer& faces0 = _patches[p0]->_faces;
   FaceContainer& faces1 = _patches[p1]->_faces;

   // Compute new vertex.
   uint nv = faces0[f0]._creases[0] == 0 ?
      addVertex(
         (_vertices[faces0[f0]._vID[0]] +
          _vertices[faces0[f0]._vID[2]] +
          _vertices[faces1[f1]._vID[0]] +
          _vertices[faces1[f1]._vID[2]] ) * 0.25f
      ) :
      addVertex(
         (_vertices[faces0[f0]._vID[0]] +
          _vertices[faces0[f0]._vID[1]] ) * 0.5f
      );
   _verticesLevel.pushBack( faces0[f0]._level + 1 );

   // Compute new mapping.
   uint nm0 = NULL_ID;
   uint nm1 = NULL_ID;
   if( !_mapping.empty() )
   {
      // Do we have a discontinuity in the mapping?
      if( faces0[f0]._mID[0] != faces1[f1]._mID[1] ||
          faces0[f0]._mID[1] != faces1[f1]._mID[0] )
      {
         nm0 = addMapping(
            (_mapping[faces0[f0]._mID[0]] + _mapping[faces0[f0]._mID[1]]) * 0.5f
         );
         nm1 = addMapping(
            (_mapping[faces1[f1]._mID[0]] + _mapping[faces1[f1]._mID[1]]) * 0.5f
         );
      }
      else
      {
         nm0 = faces0[f0]._creases[0] == 0 ?
            addMapping(
               (_mapping[faces0[f0]._mID[0]] +
                _mapping[faces0[f0]._mID[2]] +
                _mapping[faces1[f1]._mID[0]] +
                _mapping[faces1[f1]._mID[2]]) * 0.25f
            ) :
            addMapping(
               (_mapping[faces0[f0]._mID[0]] +
                _mapping[faces0[f0]._mID[1]]) * 0.5f
            );
         nm1 = nm0;
      }
   }

   // Compute new crease value.
   uchar ncrease = faces0[f0]._creases[0] == 0 ? 0 : faces0[f0]._creases[0] - 1;

   // Create new triangles.
   uint ntri0 = _patches[p0]->addFace(
      faces0[f0]._vID[2], faces0[f0]._vID[0], nv,
      faces0[f0]._mID[2], faces0[f0]._mID[0], nm0,
      faces0[f0]._creases[2], ncrease, 0
   );
   uint ntri1 = _patches[p1]->addFace(
      faces1[f1]._vID[1], faces1[f1]._vID[2], nv,
      faces1[f1]._mID[1], faces1[f1]._mID[2], nm1,
      faces1[f1]._creases[1], 0, ncrease
   );


   faces0[ntri0]._level    = faces0[f0]._level + 1;
   faces0[ntri0]._maxLevel = faces0[f0]._maxLevel;
   faces1[ntri1]._level    = faces1[f1]._level + 1;
   faces1[ntri1]._maxLevel = faces1[f1]._maxLevel;

   // Update connectivity.
   uint niEdge0 = (p0<<24)+(ntri0*3);
   uint niEdge1 = (iEdge1&~0xffffff)+(ntri1*3);

   // 1. Neighbors.
   _patches[p0]->_edgesNeighbors.pushBack( NULL_ID );
   _patches[p0]->_edgesNeighbors.pushBack( NULL_ID );
   _patches[p0]->_edgesNeighbors.pushBack( NULL_ID );
   _patches[p1]->_edgesNeighbors.pushBack( NULL_ID );
   _patches[p1]->_edgesNeighbors.pushBack( NULL_ID );
   _patches[p1]->_edgesNeighbors.pushBack( NULL_ID );

   neighbors( iEdge0, neighborEdge( iEdge0+1 ) );
   neighbors( iEdge1, neighborEdge( iEdge1+2 ) );
   neighbors( niEdge0, neighborEdge( iEdge0+2 ) );
   neighbors( niEdge1, neighborEdge( iEdge1+1 ) );
   neighbors( iEdge0+2, iEdge1+1 );
   neighbors( niEdge0+1, niEdge1+2 );
   neighbors( niEdge0+2, iEdge0+1 );
   neighbors( niEdge1+1, iEdge1+2 );

   // 2. Vertices stars.
   _verticesStars.pushBack( iEdge0 + 2 );
   _verticesStars[faces0[f0]._vID[0]] = niEdge1;
   _verticesStars[faces0[f0]._vID[2]] = niEdge0;
   _verticesStars[faces1[f1]._vID[0]] = iEdge0;
   _verticesStars[faces1[f1]._vID[2]] = iEdge1;

   // Update old triangles.
   faces0[f0]._vID[0]     = faces0[f0]._vID[1];
   faces0[f0]._vID[1]     = faces0[f0]._vID[2];
   faces0[f0]._vID[2]     = nv;
   faces0[f0]._mID[0]     = faces0[f0]._mID[1];
   faces0[f0]._mID[1]     = faces0[f0]._mID[2];
   faces0[f0]._mID[2]     = nm0;
   faces0[f0]._creases[2] = ncrease;
   faces0[f0]._creases[0] = faces0[f0]._creases[1];
   faces0[f0]._creases[1] = 0;
   ++faces0[f0]._level;

   faces1[f1]._vID[1]     = faces1[f1]._vID[0];
   faces1[f1]._vID[0]     = faces1[f1]._vID[2];
   faces1[f1]._vID[2]     = nv;
   faces1[f1]._mID[1]     = faces1[f1]._mID[0];
   faces1[f1]._mID[0]     = faces1[f1]._mID[2];
   faces1[f1]._mID[2]     = nm1;
   faces1[f1]._creases[1] = ncrease;
   faces1[f1]._creases[0] = faces1[f1]._creases[2];
   faces1[f1]._creases[2] = 0;
   ++faces1[f1]._level;
}

//------------------------------------------------------------------------------
//!
float
SurfaceGeometry::computeError( uint p0, uint f0 )
{
   // Find neighbor.
   uint iEdge0 = (p0<<24)+(f0*3);
   uint iEdge1 = neighborEdge( iEdge0 );
   uint p1     = iEdge1>>24;

   // For now, only face with neighbor is supported.
   uint f1 = (iEdge1&0xffffff)/3;
   Face& face0 = _patches[p0]->_faces[f0];
   Face& face1 = _patches[p1]->_faces[f1];

   uint level = face0._level;

   // Is this face part of a basic block?
   if(  face1._level != level ) return 0.0f;

   // Always subdivide odd level.
   if( level & 1 ) return 1000000.0f;

   // ...
   if( _verticesLevel[face0._vID[0]] != level ||
       _verticesLevel[face0._vID[1]] != level
   )
   {
      return 0.0f;
   }

   float error = 0.0f;

   // Compute new corner position.
   Vec3f co0;
   Vec3f co1;
   Vec3f co2;
   Vec3f co3;
   computeCorner( face0._vID[0], level+1, co0 );
   computeCorner( face0._vID[2], level+1, co1 );
   computeCorner( face1._vID[0], level+1, co2 );
   computeCorner( face1._vID[2], level+1, co3 );

   const Vec3f& v0 = _vertices[face0._vID[0]];
   const Vec3f& v1 = _vertices[face0._vID[2]];
   const Vec3f& v2 = _vertices[face1._vID[0]];
   const Vec3f& v3 = _vertices[face1._vID[2]];

   // Compute center position.
   Vec3f c = face0._creases[0] == 0 ?
      (v0 + v1 + v2 + v3 ) * 0.25f : (v0 + v2 ) * 0.5f;

   // Test first edge.
   if( face0._creases[1] == 0 )
   {
      Vec3f c2;
      if( computeFaceCenter( neighborEdge( iEdge0+1 ), level+1, c2 ) )
      {
         Vec3f diff   = ( co1 + co2 )*0.5f - ( v1 + v2 + c + c2 )*0.25f;
         float cerror = diff.sqrLength() / ( co1 - co2 ).sqrLength();
         CGM::clampMin( error, cerror );
      }
   }
   else
   {
      Vec3f diff   = ( co1 + co2 )*0.5f - ( v1 + v2 )*0.5f;
      float cerror = diff.sqrLength() / ( co1 - co2 ).sqrLength();
      CGM::clampMin( error, cerror );
   }

   // Test second edge.
   if( face0._creases[2] == 0 )
   {
      Vec3f c2;
      if( computeFaceCenter( neighborEdge( iEdge0+2 ), level+1, c2 ) )
      {
         Vec3f diff   = ( co0 + co1 )*0.5f - ( v0 + v1 + c + c2 )*0.25f;
         float cerror = diff.sqrLength() / ( co0 - co1 ).sqrLength();
         CGM::clampMin( error, cerror );
      }
   }
   else
   {
      Vec3f diff   = ( co0 + co1 )*0.5f - ( v0 + v1 )*0.5f;
      float cerror = diff.sqrLength() / ( co0 - co1 ).sqrLength();
      CGM::clampMin( error, cerror );
   }

   // Test third edge.
   if( face1._creases[1] == 0 )
   {
      Vec3f c2;
      if( computeFaceCenter( neighborEdge( iEdge1+1 ), level+1, c2 ) )
      {
         Vec3f diff   = ( co0 + co3 )*0.5f - ( v0 + v3 + c + c2 )*0.25f;
         float cerror = diff.sqrLength() / ( co0 - co3 ).sqrLength();
         CGM::clampMin( error, cerror );
      }
   }
   else
   {
      Vec3f diff   = ( co0 + co3 )*0.5f - ( v0 + v3 )*0.5f;
      float cerror = diff.sqrLength() / ( co0 - co3 ).sqrLength();
      CGM::clampMin( error, cerror );
   }

   // Test fourth edge.
   if( face1._creases[2] == 0 )
   {
      Vec3f c2;
      if( computeFaceCenter( neighborEdge( iEdge1+2 ), level+1, c2 ) )
      {
         Vec3f diff   = ( co2 + co3 )*0.5f - ( v2 + v3 + c + c2 )*0.25f;
         float cerror = diff.sqrLength() / ( co2 - co3 ).sqrLength();
         CGM::clampMin( error, cerror );
      }
   }
   else
   {
      Vec3f diff   = ( co2 + co3 )*0.5f - ( v2 + v3 )*0.5f;
      float cerror = diff.sqrLength() / ( co2 - co3 ).sqrLength();
      CGM::clampMin( error, cerror );
   }

   return CGM::sqrt( error );
}

//------------------------------------------------------------------------------
//!
bool
SurfaceGeometry::computeFaceCenter( uint e0, uint level, Vec3f& c )
{
   uint p0 = e0>>24;
   uint f0 = (e0&0xffffff)/3;
   Face& face0 = _patches[p0]->_faces[f0];

   if( face0._level == level )
   {
      c = _vertices[face0._vID[2]];
      return true;
   }
   else
   {
      uint iEdge0 = (p0<<24)+(f0*3);
      uint iEdge1 = neighborEdge( iEdge0 );
      uint p1     = iEdge1>>24;
      uint f1     = (iEdge1&0xffffff)/3;
      Face& face1 = _patches[p1]->_faces[f1];

      // Is this face part of a basic block?
      if( face0._level != face1._level )
      {
         return false;
      }

      c = (_vertices[face0._vID[0]] +
           _vertices[face0._vID[2]] +
           _vertices[face1._vID[0]] +
           _vertices[face1._vID[2]] ) * 0.25f;
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
SurfaceGeometry::computeCorner( uint v, uint level, Vec3f& c )
{
   Vec3f spos( 0, 0, 0 );
   Vec3f hpos( 0, 0, 0 );
   uint snb = 0;
   uint hnb = 0;
   uint firstEdge = starFirst( v );
   uint curEdge   = firstEdge;
   do
   {
      if( faceFromEdge( curEdge )._level != level )
      {
         if( ( (curEdge&0xffffff) % 3 ) != 0 )
         {
            Vec3f pos;
            if( computeFaceCenter( curEdge, level, pos ) )
            {
               spos += pos;
               spos += _vertices[dstVertexID( curEdge )];
               snb  += 2;
            }
            else
            {
               c = _vertices[v];
               return false;
            }
         }
      }
      else
      {
         ++snb;
         spos += _vertices[dstVertexID( curEdge )];
      }

      if( crease( curEdge ) > 0 )
      {
         ++hnb;
         hpos += _vertices[dstVertexID( curEdge )];
      }

      curEdge = neighborEdge( prevEdge( curEdge ) );
   } while( curEdge != firstEdge );

   // Smooth.
   if( hnb < 2 )
   {
      float n = (float)snb*0.5f;
      c = spos*(1.0f/(n*n)) + _vertices[v]*((n-2.0f)/n);
   }
   // Crease.
   else if( hnb == 2 )
   {
      c = hpos*0.125f + _vertices[v]*0.75f;
   }
   // Corner.
   else
   {
      c = _vertices[v];
   }

   return true;
}

//------------------------------------------------------------------------------
//!
void
SurfaceGeometry::subdivide( float error, uint maxLevel )
{
   computeConnectivity();
   computeCreases();

   if( _verticesLevel.empty() )
   {
      _verticesLevel = LevelContainer( _vertices.size(), 0 );
   }

   VecContainer vertices;
   for( uint l = 0; l < maxLevel*2; ++l )
   {
      // Face split.
      for( uint p = 0; p < _patches.size(); ++p )
      {
         FaceContainer& faces = _patches[p]->_faces;
         uint nbFace = uint(faces.size());
         for( uint f = 0; f < nbFace; ++f )
         {
            // Do we need to subdivide?
            if( faces[f]._level == l && computeError( p, f ) >= error )
            {
               splitFace( p, f );
            }
         }
      }
      // Smoothing: corner rule.
      if( (l&1) == 0 )
      {
         vertices = _vertices;
         for( uint v = 0; v < _vertices.size(); ++v )
         {
            if( _verticesLevel[v] == l )
            {
               Vec3f spos( 0, 0, 0 );
               Vec3f hpos( 0, 0, 0 );
               uint snb = 0;
               uint hnb = 0;
               uint fnb = 0;
               uint firstEdge = starFirst( v );
               uint curEdge   = firstEdge;
               do
               {
                  ++snb;
                  spos += _vertices[dstVertexID( curEdge )];

                  if( faceFromEdge( curEdge )._level == l+1 )
                  {
                     ++fnb;
                  }

                  if( crease( curEdge ) > 0 )
                  {
                     ++hnb;
                     hpos += _vertices[dstVertexID( curEdge )];
                  }

                  curEdge = neighborEdge( prevEdge( curEdge ) );
               } while( curEdge != firstEdge );
               if( fnb == snb )
               {
                  ++_verticesLevel[v];
                  // Smooth.
                  if( hnb < 2 )
                  {
                     float n = (float)snb*0.5f;
                     vertices[v] = spos*(1.0f/(n*n)) + _vertices[v]*((n-2.0f)/n);
                  }
                  // Crease.
                  else if( hnb == 2 )
                  {
                     vertices[v] = hpos*0.125f + _vertices[v]*0.75f;
                  }
               }
            }
         }
      }
      else
      {
         for( uint v = 0; v < _vertices.size(); ++v )
         {
            if( _verticesLevel[v] == l )
            {
               ++_verticesLevel[v];
               _vertices[v] = vertices[v];
            }
         }
      }
   }
}

/*
//------------------------------------------------------------------------------
//!
void
Surface::init( VMState* vm )
{
   // Reading vertices.
   if( VM::get( vm, 1, "vertex" ) )
   {
      reserveVertices( VM::getTableSize( vm, -1 ) );
      for( int i = 1; VM::geti( vm, -1, i ); ++i )
      {
         addVertex( VM::toVec3f( vm, -1 ) );
         VM::pop( vm, 1 );
      }
      VM::pop( vm, 1 );
   }

   // Reading skin weights.
   if( VM::get( vm, 1, "skin" ) )
   {
      reserveWeights( VM::getTableSize( vm, -1 ) );
      for( int i = 1; VM::geti( vm, -1, i ); ++i )
      {
         Vec4f w;
         Vec4i b;
         VM::get( vm, -1, "w", w );
         VM::get( vm, -1, "b", b );
         int nbWeight;
         for( nbWeight = 0; (nbWeight < 4) && (w(nbWeight) != 0); ++nbWeight );
         addWeights( nbWeight, b, w );
         VM::pop( vm, 1 );
      }
      VM::pop( vm, 1 );
   }

   // Reading mapping.
   if( VM::get( vm, 1, "mapping" ) )
   {
      reserveMapping( VM::getTableSize( vm, -1 ) );
      for( int i = 1; VM::geti( vm, -1, i ); ++i )
      {
         addMapping( VM::toVec2f( vm, -1 ) );
         VM::pop( vm, 1 );
      }
      VM::pop( vm, 1 );
   }

   // Reading patches.
   if( VM::get( vm, 1, "patch" ) )
   {
      for( int p = 1; VM::geti( vm, -1, p ); ++p )
      {
         RCP<Patch> patch = addPatch();
         VM::get( vm, -1, "mat", patch->_materialID );
         VM::get( vm, -1, "face" );
         patch->reserveFaces( VM::getTableSize( vm, -1 ) );
         for( int i = 1; VM::geti( vm, -1, i ); ++i )
         {
            Vec3i v;
            Vec3i m( 0, 0, 0 );
            Vec3i c( 255, 255, 255 );
            VM::get( vm, -1, "v", v );
            VM::get( vm, -1, "m", m );
            VM::get( vm, -1, "c", c );
            patch->addFace(
               v(0), v(1), v(2),
               m(0), m(1), m(2),
               c(0), c(1), c(2)
            );
            VM::pop( vm, 1 );
         }
         VM::pop( vm, 2 ); // Face + Patch.
      }
      VM::pop( vm, 1 );
   }

   computeDerivedData();
}

//------------------------------------------------------------------------------
//!
bool
Surface::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_COMPUTE_WEIGHTS:
         VM::push( vm, this, computeWeightsVM );
         return true;
      case ATTRIB_NUM_FACES:
         VM::push( vm, numFaces() );
         return true;
      case ATTRIB_NUM_MAPPINGS:
         VM::push( vm, numMappings() );
         return true;
      case ATTRIB_NUM_NORMALS:
         VM::push( vm, numNormals() );
         return true;
      case ATTRIB_NUM_PATCHES:
         VM::push( vm, numPatches() );
         return true;
      case ATTRIB_NUM_TANGENTS:
         VM::push( vm, numTangents() );
         return true;
      case ATTRIB_NUM_VERTICES:
         VM::push( vm, numVertices() );
         return true;
      case ATTRIB_NUM_WEIGHTS:
         VM::push( vm, numWeights() );
         return true;
      case ATTRIB_SAVE:
         VM::push( vm, this, saveVM );
         return true;
      case ATTRIB_TRANSLATE:
         VM::push( vm, this, translateVM );
         return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Surface::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      // Read only.
      case ATTRIB_COMPUTE_WEIGHTS:
      case ATTRIB_NUM_FACES:
      case ATTRIB_NUM_MAPPINGS:
      case ATTRIB_NUM_NORMALS:
      case ATTRIB_NUM_PATCHES:
      case ATTRIB_NUM_TANGENTS:
      case ATTRIB_NUM_VERTICES:
      case ATTRIB_NUM_WEIGHTS:
      case ATTRIB_SAVE:
      case ATTRIB_TRANSLATE:
         return true;
   }

   return false;
}
*/
NAMESPACE_END
