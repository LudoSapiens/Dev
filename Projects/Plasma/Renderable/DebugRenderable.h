/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DEBUGRENDERABLE_H
#define PLASMA_DEBUGRENDERABLE_H

#include <Plasma/StdDefs.h>
#include <Plasma/Renderable/Renderable.h>

#include <Base/Util/RCP.h>
#include <Base/ADT/Vector.h>

#include <CGMath/Vec3.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS DebugRenderable
==============================================================================*/

//!

class DebugRenderable
   : public Renderable
{

public: 

   /*----- methods -----*/

   PLASMA_DLL_API DebugRenderable();
   PLASMA_DLL_API virtual void render( const RCP<Gfx::Pass>& ) const; 
   
protected: 

   /*----- methods -----*/

   virtual ~DebugRenderable();

   void addDebugGeometry();

private: 

   /*----- data members -----*/

   RCP<Gfx::Geometry>     _geom;
   RCP<Gfx::VertexBuffer> _vbuffer;
   RCP<Gfx::IndexBuffer>  _ibuffer;

   Vector<ushort>         _idx;
   Vector<Vec3f>          _vertices;

   RCP<Gfx::Program>      _prog;
};

NAMESPACE_END

#endif

