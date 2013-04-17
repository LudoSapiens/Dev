/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_GEOMETRY_H
#define GFX_GEOMETRY_H

#include <Gfx/Geom/Buffer.h>
#include <Gfx/StdDefs.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>
#include <Base/ADT/Vector.h>


NAMESPACE_BEGIN

namespace Gfx
{


/*==============================================================================
   TYPES
==============================================================================*/
typedef enum
{
   PRIM_INVALID,
   PRIM_POINTS,
   PRIM_LINES,
   PRIM_LINE_LOOP,
   PRIM_LINE_STRIP,
   PRIM_TRIANGLES,
   PRIM_TRIANGLE_STRIP,
   PRIM_TRIANGLE_FAN,
   PRIM_QUADS,
   PRIM_QUAD_STRIP,
   PRIM_POLYGON
} PrimitiveType;

//------------------------------------------------------------------------------
//!
GFX_DLL_API const char*  toStr( PrimitiveType pt );


/*==============================================================================
   CLASS Geometry
==============================================================================*/

class Geometry:
   public RCObject
{

public:

   /*----- types and enumerations ----*/

   typedef Vector< RCP<VertexBuffer> >  BufferContainer;


   /*----- methods -----*/

   PrimitiveType  primitiveType() const                   { return _primType; }
   void           primitiveType( const PrimitiveType pt ) { _primType = pt; }

   const RCP<IndexBuffer>&  indexBuffer() const           { return _indices; }
   void  indexBuffer( const RCP<IndexBuffer>& buffer )    { _indices = buffer; _revision = 0; }

   const BufferContainer&  buffers() const                { return _buffers; }
   RCP<VertexBuffer> buffer( uint i ) const               { return _buffers[i]; }

   uint  numBuffers() const                               { return (uint)_buffers.size(); }
   void  numBuffers( const uint n )                       { _buffers.resize(n); }

   void  addBuffer( const RCP<VertexBuffer>& buffer )     { _buffers.pushBack(buffer); _revision = 0; }
   GFX_DLL_API bool  removeBuffer( const RCP<VertexBuffer>& buffer );
   void  removeAllBuffers()                               { _buffers.clear(); _indices = NULL; }

   GFX_DLL_API size_t  numPrimitives() const;
   GFX_DLL_API size_t  numPrimitives( size_t nbIndices ) const;

   inline uint  revision() const { return _revision; }

protected:

   /*----- methods -----*/

   Geometry( const PrimitiveType pt = PRIM_INVALID );
   GFX_DLL_API virtual ~Geometry();

   /*----- data members -----*/

   mutable uint            _revision;
   PrimitiveType           _primType;  //!< Type of primitive to draw with the specified buffers
   RCP<IndexBuffer>        _indices;   //!< The indices (separate from the other buffers)
   BufferContainer         _buffers;   //!< A series of buffers representing various parameters, such as color, normal, etc. (one has to be vertex)

private:

   //Only Managers can create this object
   GFX_MAKE_MANAGERS_FRIENDS();

};


}  //namespace Gfx

NAMESPACE_END


#endif //GFX_GEOMETRY_H
