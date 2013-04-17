/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_BASICSRENDERABLE_H
#define PLASMA_BASICSRENDERABLE_H

#include <Plasma/StdDefs.h>
#include <Plasma/Renderable/Renderable.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS GridRenderable
==============================================================================*/

//!

class GridRenderable
   : public Renderable
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API GridRenderable();

   PLASMA_DLL_API virtual void render( const RCP<Gfx::Pass>& ) const;
      
protected:

   /*----- methods -----*/

   virtual ~GridRenderable();
   
private:

   /*----- data members -----*/

   int   _qty;
   float _space;
};

NAMESPACE_END

#endif

