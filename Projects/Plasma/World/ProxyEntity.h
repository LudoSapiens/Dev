/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PROXY_ENTITY_H
#define PLASMA_PROXY_ENTITY_H

#include <Plasma/StdDefs.h>
#include <Plasma/World/Entity.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ProxyEntity
==============================================================================*/
class ProxyEntity:
   public Entity
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API ProxyEntity();

   PLASMA_DLL_API void entity( Entity* e );

   Entity* entity() const { return _entity.ptr(); }

protected:

   /*----- methods -----*/

   virtual ~ProxyEntity();

   /*----- data members -----*/

   RCP<Entity> _entity;
};

NAMESPACE_END

#endif
