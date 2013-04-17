/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Renderable/RenderableGroup.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN


UNNAMESPACE_END


/*==============================================================================
  CLASS RenderableGroup
==============================================================================*/

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
void
RenderableGroup::render( Gfx::Pass& p, const Viewport& v ) const
{
   for( auto r = _group.begin(); r != _group.end(); ++r )
   {
      (*r)->render( p, v );
   }
}

//------------------------------------------------------------------------------
//!
void
RenderableGroup::addFront( Renderable* r )
{
   _group.insert( _group.begin(), r );
}

//------------------------------------------------------------------------------
//!
void
RenderableGroup::addBack( Renderable* r )
{
   _group.pushBack( r );
}

//------------------------------------------------------------------------------
//!
void
RenderableGroup::add( Renderable* r, int pos )
{
   _group.insert( _group.begin()+pos, r );
}

//------------------------------------------------------------------------------
//!
void
RenderableGroup::remove( Renderable* r )
{
   _group.remove( r );
}

//------------------------------------------------------------------------------
//!
void
RenderableGroup::clear()
{
   _group.clear();
}

NAMESPACE_END
