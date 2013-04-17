/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/EntityGroup.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

UNNAMESPACE_END


/*==============================================================================
   CLASS EntityGroup
==============================================================================*/

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
EntityGroup::EntityGroup():
   _parent(0)
{
}

//------------------------------------------------------------------------------
//!
EntityGroup::~EntityGroup()
{
}
//------------------------------------------------------------------------------
//! 
void 
EntityGroup::addEntity( Entity* e )
{
   _entities.pushBack( e );
}

//------------------------------------------------------------------------------
//! 
void 
EntityGroup::removeEntity( Entity* e )
{
   _entities.remove( e );
}

//------------------------------------------------------------------------------
//! 
void 
EntityGroup::removeAllEntities()
{
   _entities.clear();
}

//------------------------------------------------------------------------------
//! 
void 
EntityGroup::addGroup( EntityGroup* g )
{
   if( g->_parent ) g->_parent->removeGroup(g);

   g->_parent = this;
   _groups.pushBack( g );
}

//------------------------------------------------------------------------------
//! 
void 
EntityGroup::removeGroup( EntityGroup* g )
{
   _groups.remove( g );
}

//------------------------------------------------------------------------------
//! 
void 
EntityGroup::removeAllGroup()
{
   _groups.clear();
}

NAMESPACE_END
