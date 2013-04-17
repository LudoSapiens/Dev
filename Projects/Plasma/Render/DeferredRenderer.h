/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DEFERREDRENDERER_H
#define PLASMA_DEFERREDRENDERER_H

#include <Plasma/StdDefs.h>
#include <Plasma/Render/Renderer.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS DeferredRenderer
==============================================================================*/

//!

class DeferredRenderer
   : public Renderer
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API DeferredRenderer();

   PLASMA_DLL_API virtual void render( const RCP<Gfx::RenderNode>&, World*, Viewport* );


protected: 

   /*----- methods -----*/

   ~DeferredRenderer();
   virtual void performResize();
   virtual void performBeginFrame();
   virtual void performEndFrame();

   /*----- members -----*/

};


NAMESPACE_END

#endif
