/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PRIMITIVE_RENDERABLE_H
#define PLASMA_PRIMITIVE_RENDERABLE_H

#include <Plasma/StdDefs.h>

#include <Plasma/World/Material.h>
#include <Plasma/Renderable/Renderable.h>

#include <Fusion/VM/VMObject.h>

#include <Gfx/Geom/Geometry.h>

#include <CGMath/Ref.h>

#include <Base/ADT/Map.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS PrimitiveRenderable
==============================================================================*/
//! A class grouping buffers (vertex and index), along with shader program
//! in order to draw specific primitives.
//! Basically consists in a Lua wrapper for the Gfx::Geometry class.
class PrimitiveRenderable:
   public Renderable
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API PrimitiveRenderable();
   PLASMA_DLL_API virtual ~PrimitiveRenderable();

   PLASMA_DLL_API virtual void render( const RCP<Gfx::Pass>& ) const;

   // Accessors.
         RCP<Gfx::Geometry>  geom()       { return _geom; }
   const RCP<Gfx::Geometry>  geom() const { return _geom; }

   // VM.
   PLASMA_DLL_API void  init( VMState* );
   PLASMA_DLL_API bool  performGet( VMState* );
   PLASMA_DLL_API bool  performSet( VMState* );

protected:

   bool  setBuffer( VMState* vm, const String& name );

   /*----- data members -----*/
   typedef Map<String, Gfx::VertexBuffer*>  CurBufMap;

   RCP<Gfx::Geometry>  _geom;    //!< The geometry instance.
   RCP<Material>       _mat;     //!< The associated material.
   Reff                _xform;   //!< World transform matrix.
   Vec2i               _range;   //!< Range used for index buffer.
   bool                _hasRange;//!< Indicates whether or not the range was specified.
   RCP<Gfx::AlphaState>   _aState; //!< Alpha state
   RCP<Gfx::ColorState>   _cState; //!< Color state
   RCP<Gfx::DepthState>   _dState; //!< Depth state
   RCP<Gfx::StencilState> _sState; //!< Stencil state
   CurBufMap           _curBufMap; //!< A map for the current buffers.

}; //class PrimitiveRenderable


/*==============================================================================
  VM Section
  ==============================================================================*/
//VMOBJECT_TRAITS( PrimitiveRenderable, primitive )
//typedef VMObject< PrimitiveRenderable > PrimitiveVM;


NAMESPACE_END

#endif //PLASMA_PRIMITIVE_RENDERABLE_H
