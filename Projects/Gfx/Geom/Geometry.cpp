/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Geom/Geometry.h>

NAMESPACE_BEGIN

namespace Gfx
{

//------------------------------------------------------------------------------
//!
const char*
toStr( PrimitiveType pt )
{
   switch( pt )
   {
   case PRIM_INVALID       : return "<invalid>";
   case PRIM_POINTS        : return "points";
   case PRIM_LINES         : return "lines";
   case PRIM_LINE_LOOP     : return "line loop";
   case PRIM_LINE_STRIP    : return "line strip";
   case PRIM_TRIANGLES     : return "triangles";
   case PRIM_TRIANGLE_STRIP: return "triangle strip";
   case PRIM_TRIANGLE_FAN  : return "triangle fan";
   case PRIM_QUADS         : return "quads";
   case PRIM_QUAD_STRIP    : return "quad strip";
   case PRIM_POLYGON       : return "polygon";
   default                 : return "<unknown>";
   }
}

/*==============================================================================
   CLASS Geometry
==============================================================================*/

//------------------------------------------------------------------------------
//!
Geometry::Geometry
( const PrimitiveType pt ) :
   _revision(0),
   _primType(pt)
{
}

//------------------------------------------------------------------------------
//!
Geometry::~Geometry
( void )
{
}

//------------------------------------------------------------------------------
//!
bool
Geometry::removeBuffer
( const RCP<VertexBuffer>& buffer )
{
   if( _buffers.remove( buffer ) )
   {
      _revision = 0;
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
size_t
Geometry::numPrimitives
( void )
const
{
   size_t numIndices;
   if( indexBuffer().isValid()  )       numIndices = indexBuffer()->numIndices();
   else if( numBuffers() > 0 )          numIndices = buffer(0)->numVertices();
   else                                 numIndices = 0;

   return numPrimitives( numIndices );
}

//------------------------------------------------------------------------------
//!
size_t
Geometry::numPrimitives( size_t numIndices )
const
{
   switch(primitiveType())
   {
   case PRIM_INVALID       : return 0;
   case PRIM_POINTS        : return numIndices;
   case PRIM_LINES         : return numIndices/2;
   case PRIM_LINE_LOOP     : return numIndices;
   case PRIM_LINE_STRIP    : return numIndices-1;
   case PRIM_TRIANGLES     : return numIndices/3;
   case PRIM_TRIANGLE_STRIP: return numIndices-2;
   case PRIM_TRIANGLE_FAN  : return numIndices-2;
   case PRIM_QUADS         : return numIndices/4;
   case PRIM_QUAD_STRIP    : return (numIndices-2)/2;
   case PRIM_POLYGON       : return 1;
   default                 : return 0;
   }
}

}  //namespace Gfx

NAMESPACE_END
