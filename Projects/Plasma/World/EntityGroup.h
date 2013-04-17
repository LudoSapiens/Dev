/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef ENTITYGROUP_H
#define ENTITYGROUP_H

#include <Plasma/StdDefs.h>

#include <Plasma/World/Entity.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS EntityGroup
==============================================================================*/

class EntityGroup :
   public RCObject
{
public:	

   /*----- methods -----*/

   PLASMA_DLL_API EntityGroup();
   PLASMA_DLL_API virtual ~EntityGroup();

   // Entities.
   PLASMA_DLL_API void addEntity( Entity* );
   PLASMA_DLL_API void removeEntity( Entity* );
   PLASMA_DLL_API void removeAllEntities();
   inline uint numEntities() const            { return uint(_entities.size()); }
   inline Entity* entity( uint i ) const      { return _entities[i].ptr(); }

   // Groups.
   PLASMA_DLL_API void addGroup( EntityGroup* );
   PLASMA_DLL_API void removeGroup( EntityGroup* );
   PLASMA_DLL_API void removeAllGroup();
   inline uint numGroups() const              { return uint(_groups.size()); }
   inline EntityGroup* group( uint i ) const  { return _groups[i].ptr(); }

protected:
   
   /*----- methods -----*/
   
   friend class World;

private: 

   /*----- data members -----*/

   EntityGroup*               _parent;
   Vector< RCP<Entity> >      _entities;
   Vector< RCP<EntityGroup> > _groups;

};

NAMESPACE_END

#endif
