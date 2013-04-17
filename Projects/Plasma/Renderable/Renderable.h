/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_RENDERABLE_H
#define PLASMA_RENDERABLE_H

#include <Plasma/StdDefs.h>

#include <Gfx/Pass/Pass.h>

#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

class Viewport;

/*==============================================================================
  CLASS Renderable
==============================================================================*/

//! Class for adding custom rendering in a world. Used mainly for debugging and
//! manipulator.

class Renderable
   : public RCObject
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API virtual void render( Gfx::Pass&, const Viewport& ) const = 0;

protected:

   /*----- methods -----*/

   PLASMA_DLL_API Renderable();
   PLASMA_DLL_API virtual ~Renderable();
};

NAMESPACE_END

#endif //PLASMA_RENDERABLE_H
