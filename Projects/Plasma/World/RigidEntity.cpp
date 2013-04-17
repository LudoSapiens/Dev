/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/RigidEntity.h>
#include <Plasma/World/World.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

UNNAMESPACE_END

/*==============================================================================
  CLASS RigidEntity
==============================================================================*/


NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
RigidEntity::RigidEntity( RigidBody::Type bodyType ):
   Entity( RIGID )
{
   _body = new RigidBody( bodyType, (Entity*)this );
}

//------------------------------------------------------------------------------
//!
RigidEntity::RigidEntity( Entity::Type eType, RigidBody::Type bType ):
   Entity( eType )
{
   _body = new RigidBody( bType, (Entity*)this );
}

//------------------------------------------------------------------------------
//!
RigidEntity::~RigidEntity()
{
}

//------------------------------------------------------------------------------
//!
void
RigidEntity::referential( const Reff& ref )
{
   Entity::referential( ref );
   _body->referential( ref );
}

//------------------------------------------------------------------------------
//!
void
RigidEntity::position( const Vec3f& pos )
{
   Entity::position( pos );
   _body->referential( _referential );
}

//------------------------------------------------------------------------------
//!
void
RigidEntity::orientation( const Quatf& orient )
{
   Entity::orientation( orient );
   _body->referential( _referential );
}

//------------------------------------------------------------------------------
//!
void
RigidEntity::lookAt( const Vec3f& at, const Vec3f& up )
{
   Entity::lookAt( at, up );
   _body->referential( _referential );
}

//------------------------------------------------------------------------------
//!
void
RigidEntity::rotate( const Vec3f& pos, const Vec3f axis, float angle )
{
   Entity::rotate( pos, axis, angle );
   _body->referential( _referential );
}

//------------------------------------------------------------------------------
//!
void
RigidEntity::geometry( Geometry* geom )
{
   _geometry = geom;
   if( _body.isValid() )
   {
      if( _geometry.isValid() )
      {
         _body->shape( geom->collisionShape() );
      }
      else
      {
         _body->shape(0);
      }
   }
}

//------------------------------------------------------------------------------
//!
void
RigidEntity::connect( World* w )
{
   Entity::connect( w );
   world()->motionWorld()->addBody( _body.ptr() );
}

//------------------------------------------------------------------------------
//!
void
RigidEntity::disconnect()
{
   if( world() != NULL )
   {
      world()->motionWorld()->removeBody( _body.ptr() );
      Entity::disconnect();
   }
   else
   {
      printf("ERROR - Disconnected RigidEntity twice!\n");
   }
}


NAMESPACE_END
