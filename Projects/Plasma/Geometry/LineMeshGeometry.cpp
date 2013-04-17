/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Geometry/LineMeshGeometry.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
LineMeshGeometry::LineMeshGeometry():
   MeshGeometry( TRIANGLES )
{
   _type = LINEMESH;
   int attrList[] = {
      POSITION,  // Position.
      MAPPING,   // Radius and t.
      GENERIC_3, // Previous.
      GENERIC_3, // Next.
      GENERIC_4, // Color.
      0,
   };
   setAttributes( attrList );
}

//------------------------------------------------------------------------------
//!
LineMeshGeometry::~LineMeshGeometry()
{
}

//------------------------------------------------------------------------------
//!
void
LineMeshGeometry::updateAdjacency()
{
   // Reset previous and next.
   uint numV = numVertices();
   auto v    = vertex(0);
   for( uint i = 0; i < numV; ++i, ++v )
   {
      v.previous( v.position() );
      v.next( v.position() );
   }
   // Set previous and next for each vertex of each segment.
   uint numS = numSegments();
   auto s    = segment(0);
   for( uint i = 0; i < numS; ++i, ++s )
   {
      auto v0 = vertex( s.v0() );
      auto v1 = vertex( s.v1() );
      v0.next( v1.position() );
      v1.previous( v0.position() );
   }
}

//------------------------------------------------------------------------------
//!
void
LineMeshGeometry::print( TextStream& os ) const
{
   os << "LineMesh:" << nl;
   os << numVertices() << " vertices, " << numSegments() << " segments" << nl;
   uint n;
   n = numVertices();
   for( uint i = 0; i < n; ++i )
   {
      const Vertex v = vertex( i );
      os << "v[" << i << "]:"
         << " pos=" << v.position()
         << " t="   << v.t()
         << " r="   << v.radius()
         << " c="   << v.color()
         << " pr="  << v.previous()
         << " nx="  << v.next()
         << nl;
   }
   n = numSegments();
   for( uint i = 0; i < n; ++i )
   {
      const Segment s = segment( i );
      os << "s[" << i << "]:"
         << " " << s.v0()
         << " " << s.v1()
         << nl;
   }
   os << "---" << nl;
   MeshGeometry::print( os );
}
