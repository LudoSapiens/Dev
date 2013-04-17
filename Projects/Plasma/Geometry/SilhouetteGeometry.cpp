/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Geometry/SilhouetteGeometry.h>

#include <CGMath/Grid.h>

#include <Base/ADT/Map.h>
#include <Base/ADT/Pair.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS SilhouetteGeometry
==============================================================================*/

//------------------------------------------------------------------------------
//! 
SilhouetteGeometry::SilhouetteGeometry():
   MeshGeometry( MeshGeometry::LINES )
{
   _type = Geometry::SILHOUETTE;
}

//------------------------------------------------------------------------------
//! 
SilhouetteGeometry::~SilhouetteGeometry()
{
}

//------------------------------------------------------------------------------
//! 
void
SilhouetteGeometry::mesh( MeshGeometry* m, bool compact )
{
   if( !m || (m->primitiveType() != MeshGeometry::TRIANGLES) )
   {
      _facing.clear();
      _edges.clear();
      _normals.clear();
      // ... clear geometry...
      return;
   }

   computeVertices( m );
   computeEdges( compact );
   updateProperties();

   update( Vec3f( 0.0f, 1.0f, 15.0f ) );
}

//------------------------------------------------------------------------------
//! 
void
SilhouetteGeometry::update( const Vec3f& pos )
{
   Vector<uint32_t> ids;

   // Test triangles facing.
   for( uint i = 0; i < _facing.size(); ++i )
   {
      uint32_t posi = _triangles[i*3];
      _facing[i] = dot( _normals[i], position( posi ) - pos ) < 0.0f;
   }

   // Find silhouette edges.
   for( uint i = 0; i < _edges.size(); ++i )
   {
      if( _facing[_edges[i]._triIdx[0]] != _facing[_edges[i]._triIdx[1]] )
      {
         ids.pushBack( _edges[i]._vIdx[0] );
         ids.pushBack( _edges[i]._vIdx[1] );
      }
   }

   allocateIndices( uint(ids.size()) );
   copyIndices( ids.data() );
   clearPatches();
   addPatch( 0, uint(ids.size()) );
   _rgeom = NULL; // FIXME: we should update only the indices.
}

//------------------------------------------------------------------------------
//! 
void
SilhouetteGeometry::computeVertices( MeshGeometry* m )
{
   // Find unique indices spatially.
   const float error   = 1.0/1024.0f;
   uint numVertices    = m->numVertices();
   uint numNewVertices = 0;
   Vector<uint> ids( numVertices );
   Grid<uint> grid( numVertices / 2, 1.0f/128.0f );

   for( uint i = 0; i < numVertices; ++i )
   {
      ids[i]    = i;
      Vec3f pos = m->position(i);
      // Does the vertex exist?
      Vec3i c0 = grid.cellCoord( pos-error );
      Vec3i c1 = grid.cellCoord( pos+error );
      for( int x = c0.x; x <= c1.x ; ++x )
      {
         for( int y = c0.y; y <= c1.y; ++y )
         {
            for( int z = c0.z; z <= c1.z; ++z )
            {
               Grid<uint>::Link* l = grid.cell( Vec3i(x,y,z) );
               for( ; l; l = l->_next )
               {
                  // Have we found the vertex?
                  if( sqrLength( m->position(l->_obj)-pos) < (error*error) )
                  {
                     ids[i] = l->_obj;
                  }
               }
            }
         }
      }
      // New vertex: add it to the grid.
      if( ids[i] == i )
      {
         ++numNewVertices;
         grid.add( grid.cellID( pos ), i );
      }
   }

   // Create vertices.
   const int attribs[] = {
      MeshGeometry::POSITION,
      //MeshGeometry::NORMAL,
      0
   };
   setAttributes( attribs );
   allocateVertices( numNewVertices );

   uint cid = 0;
   for( uint i = 0; i < numVertices; ++i )
   {
      if( ids[i] == i )
      {
         float* data = _vData+(cid*_vStride+_pOffset);
         Vec3f pos   = m->position(i);
         data[0]     = pos.x;
         data[1]     = pos.y;
         data[2]     = pos.z;
         ids[i] = cid++;
      }
      else
      {
         ids[i] = ids[ids[i]];
      }
   }

   // Create indices.
   uint numIndices = m->numIndices();
   _triangles.resize( numIndices );
   for( uint i = 0; i < numIndices; ++i )
   {
      _triangles[i] = ids[m->indices()[i]];
   }
}

//------------------------------------------------------------------------------
//! 
void
SilhouetteGeometry::computeEdges( bool compact )
{
   uint numTriangles = uint(_triangles.size())/3;

   // Allocate memory for back/front facing information and face normals.
   _facing.resize( numTriangles );
   _normals.resize( numTriangles );

   // Create edges.
   typedef Pair<uint32_t,uint32_t> Key;
   typedef Map< Key, uint32_t >    EdgesMap;

   EdgesMap edgesId;
   Vector<Edge> edges;
   for( uint i = 0; i < numTriangles; ++i )
   {
      uint32_t v0 = _triangles[i*3+0];
      uint32_t v1 = _triangles[i*3+1];
      uint32_t v2 = _triangles[i*3+2];
      // Edge 0.
      EdgesMap::Iterator it = edgesId.find( Key( v1, v0 ) );
      if( it != edgesId.end() )
      {
         edges[it->second]._triIdx[1] = i;
      }
      else
      {
         edgesId[ Key( v0, v1 ) ] = int(edges.size());
         edges.pushBack( Edge( v0, v1, i, 0 ) );
      }
      // Edge 1.
      it = edgesId.find( Key( v2, v1 ) );
      if( it != edgesId.end() )
      {
         edges[it->second]._triIdx[1] = i;
      }
      else
      {
         edgesId[ Key( v1, v2 ) ] = int(edges.size());
         edges.pushBack( Edge( v1, v2, i, 0 ) );
      }
      // Edge 2.
      it = edgesId.find( Key( v0, v2 ) );
      if( it != edgesId.end() )
      {
         edges[it->second]._triIdx[1] = i;
      }
      else
      {
         edgesId[ Key( v2, v0 ) ] = int(edges.size());
         edges.pushBack( Edge( v2, v0, i, 0 ) );
      }

      // Computing normal.
      Vec3f p0    = position( v0 );
      Vec3f p1    = position( v1 );
      Vec3f p2    = position( v2 );
      _normals[i] = normalize( cross( p1-p0, p2-p0 ) );
   }

   // Compacts edges ?
   uint numEdges = uint(edges.size());
   if( compact )
   {
      for( uint i = 0; i < edges.size(); ++i )
      {
         if( _normals[edges[i]._triIdx[0]] == _normals[edges[i]._triIdx[1]] )
         {
            edges[i]._vIdx[0] = edges[i]._vIdx[1];
            --numEdges;
         }
      }
   }

   // Copy edges.
   _edges.reserve( numEdges );
   for( uint i = 0; i < edges.size(); ++i )
   {
      if( edges[i]._vIdx[0] != edges[i]._vIdx[1] ) _edges.pushBack( edges[i] ); 
   }
}

NAMESPACE_END
