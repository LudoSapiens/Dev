/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_SELECTION_H
#define PLASMA_SELECTION_H

#include <Plasma/StdDefs.h>
#include <Plasma/World/Camera.h>
#include <Plasma/World/Entity.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

class World;

/*==============================================================================
  CLASS Selection
==============================================================================*/

class Selection
   : public RCObject
{

public:

   /*----- methods -----*/

   Selection();
   Selection( Entity* );

   // Getters.
   inline World* world() const;
   inline Entity* entity() const;
   inline Camera* camera() const;

   inline void entity( const RCP<Entity>& );
   inline void camera( const RCP<Camera>& );

protected:
   
   /*----- methods -----*/

   virtual ~Selection();
   
private:

   friend class World;

   /*----- data members -----*/

   World*                  _world;
   RCP<Entity>             _entity;
   RCP<Camera>             _camera;
};

//------------------------------------------------------------------------------
//!
inline World*
Selection::world() const
{
   return _entity.isValid() ? _entity->world() : 0;
}

//------------------------------------------------------------------------------
//!
inline Entity* 
Selection::entity() const
{
   return _entity.ptr();
}

//------------------------------------------------------------------------------
//!
inline Camera* 
Selection::camera() const
{
   return _camera.ptr();
}

//------------------------------------------------------------------------------
//!
inline void 
Selection::entity( const RCP<Entity>& e )
{
   _entity = e;
}

//------------------------------------------------------------------------------
//!
inline void 
Selection::camera( const RCP<Camera>& c )
{
   _camera = c;
}

NAMESPACE_END

#endif

