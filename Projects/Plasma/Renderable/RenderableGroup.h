/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_RENDERABLE_GROUP_H
#define PLASMA_RENDERABLE_GROUP_H

#include <Plasma/StdDefs.h>
#include <Plasma/Renderable/Renderable.h>


NAMESPACE_BEGIN

/*==============================================================================
  CLASS RenderableGroup
==============================================================================*/

class RenderableGroup:
   public Renderable
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API virtual void render( Gfx::Pass&, const Viewport& ) const;

   // Group.
   PLASMA_DLL_API void addFront( Renderable* r );
   PLASMA_DLL_API void addBack( Renderable* r );
   PLASMA_DLL_API void add( Renderable* r, int pos );
   PLASMA_DLL_API void remove( Renderable* r );
   PLASMA_DLL_API void clear();

   inline const Vector< RCP<Renderable> >&  renderables() const { return _group; }

protected:

private:

   /*----- data members -----*/

   Vector< RCP<Renderable> > _group;
};


NAMESPACE_END

#endif
